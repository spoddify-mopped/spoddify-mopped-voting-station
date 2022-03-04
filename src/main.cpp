#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <GxEPD2_BW.h>
#include <SD.h>
#include <WiFi.h>
#include <fonts/FiraSans_Medium_1_8pt7b.h>

#include "SPIFFS.h"
#include "filesystem.h"

#define LILYGO_T5_V213
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_213_B74
#define GxEPD2_BW_IS_GxEPD2_BW true

#define AP_NAME "SM_setup"
#define AP_PW NULL

// display

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(
    /*CS=5*/ 5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));  // GxEPD2_213_B74
#define MAX_HEIGHT(EPD)                                        \
    (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) \
         ? EPD::HEIGHT                                         \
         : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8));

AsyncWebServer server(80);

const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";

const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";

String ssid;
String pass;

unsigned long previousMillis = 0;
const long interval = 10000;

bool initWiFi() {
    if (ssid == "") {
        return false;
    }

    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.println("Connecting to WiFi...");

    unsigned long currentMillis = millis();
    previousMillis = currentMillis;

    while (WiFi.status() != WL_CONNECTED) {
        currentMillis = millis();
        if (currentMillis - previousMillis >= interval) {
            Serial.println("Failed to connect.");
            return false;
        }
    }

    Serial.println(WiFi.localIP());
    return true;
}

void serveReady() {
    server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response =
            request->beginResponseStream("application/json");
        DynamicJsonDocument json(1024);
        json["ssid"] = WiFi.SSID();
        json["ip"] = WiFi.localIP().toString();
        serializeJson(json, *response);
        request->send(response);
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html", false);
    });
    server.serveStatic("/", SPIFFS, "/");
    server.begin();
}

void drawReady() {
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(18, 18);
        display.print("Connection established.");
    } while (display.nextPage());
}

void serveSetup() {
    Serial.println("Starting Access Point...");
    WiFi.softAP(AP_NAME, AP_PW);

    IPAddress IP = WiFi.softAPIP();
    Serial.printf("Go to http://%s to setup the remote.\n'",
                  IP.toString().c_str());

    // TODO: Scan networks and show it in frontend
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/setup.html", "text/html");
    });

    server.serveStatic("/", SPIFFS, "/");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();

        for (int i = 0; i < params; i++) {
            AsyncWebParameter *param = request->getParam(i);
            if (param->isPost()) {
                if (param->name() == PARAM_INPUT_1) {
                    ssid = param->value().c_str();
                    Filesystem.writeFile(SPIFFS, ssidPath, ssid.c_str());
                }
                if (param->name() == PARAM_INPUT_2) {
                    pass = param->value().c_str();
                    Filesystem.writeFile(SPIFFS, passPath, pass.c_str());
                }
            }
        }

        request->send(204, "text/plain", "");
        delay(1000);
        ESP.restart();
    });
    server.begin();
}

void drawSetup() {
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(2, 20);
        display.print("Connect with Wi-Fi to: ");
        display.setCursor(2, 50);
        display.printf("'%s'", AP_NAME);
        display.setCursor(2, 80);
        display.print("and open http://192.168.4.1");
    } while (display.nextPage());
}

void setup() {
    Serial.begin(115200);

    display.init();

    display.setRotation(1);
    display.setFont(&FiraSans_Medium_1_8pt7b);
    display.setTextColor(GxEPD_BLACK);

    Filesystem.init();

    ssid = Filesystem.readFile(SPIFFS, ssidPath);
    pass = Filesystem.readFile(SPIFFS, passPath);

    bool wifiReady = initWiFi();

    if (wifiReady) {
        drawReady();
        serveReady();
    } else {
        drawSetup();
        serveSetup();
    }
}

void loop() {}