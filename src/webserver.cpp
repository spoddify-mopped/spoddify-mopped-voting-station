/**
 * Web server
 */

#include "webserver.h"

#include <ESPAsyncWebServer.h>

#include "SPIFFS.h"
#include "station.h"
#include "wifimanager.h"

/**
 * This enables the SPIFFS editor under '/edit'.
 * It should be disabled for production.
 */
// #define SPIFFS_EDITOR

#ifdef SPIFFS_EDITOR
#include "SPIFFSEditor.h"
#endif

#define CONTENT_TYPE_PLAIN "text/plain"
#define CONTENT_TYPE_HTML "text/html"

#define STATIC_URL_PATH "/"
#define STATIC_FILES_PATH "/web/"

AsyncWebServer server(80);

uint32_t restartTime = 0;

void restart() { restartTime = millis() + 100; }

/**
 * Handles request to show the wifi setup page.
 */
void handleWifiSetupPage(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/web/setup.html", CONTENT_TYPE_HTML);
}

/**
 * Initalizes routes when in setup mode.
 */
void initalizeSetupRoutes() {
    /**
     *  GET
     */
    server.on("/", HTTP_GET, handleWifiSetupPage);
}

/**
 * Processes the template of the settings page.
 */
String settingsPageProcessor(const String &p) {
    if (p == "SSID_PLACEHOLDER") {
        return WifiManager.SSID();
    }

    if (p == "CONNECTION_STATUS") {
        return StationClient.isConnected() ? "Connected" : "Unconnected";
    }

    if (p == "STATION_HOST") {
        return StationClient.getStationHost();
    }

    return String();
}

/**
 * Handles request to show the wifi setup page.
 */
void handleSettingsPage(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/web/index.html", CONTENT_TYPE_HTML, false,
                  settingsPageProcessor);
}

/**
 * Handles request restart the esp.
 */
void handleRestart(AsyncWebServerRequest *request) {
    restart();

    request->send(204, CONTENT_TYPE_PLAIN, "");
}

/**
 * Handles request to reset the esp.
 */
void handleReset(AsyncWebServerRequest *request) {
    WifiManager.reset();
    restart();

    request->send(204, CONTENT_TYPE_PLAIN, "");
}

/**
 * Initalizes routes after setup has been finished.
 */
void initalizeRoutes() {
    /**
     *  GET
     */
    server.on("/", HTTP_GET, handleSettingsPage);

    /**
     *  POST
     */
    server.on("/restart", HTTP_POST, handleRestart);
    server.on("/reset", HTTP_POST, handleReset);
}

/**
 * Handles request to setup or update the wifi credentials.
 */
void handleWifiSetupForm(AsyncWebServerRequest *request) {
    String ssid = "";
    String pass = "";
    int resultStatus = 400;

    if (request->hasParam("ssid", true) && request->hasParam("pass", true)) {
        ssid = request->getParam("ssid", true)->value();
        pass = request->getParam("pass", true)->value();
    }

    if (!ssid.isEmpty()) {
        resultStatus = 200;
    }

    if (resultStatus == 400) {
        request->send(resultStatus, F(CONTENT_TYPE_PLAIN), F("Bad request"));
        return;
    }

    WifiManager.saveCredentials(ssid, pass);
    restart();

    request->send(204, CONTENT_TYPE_PLAIN, "");
}

/**
 * Handles not found requests.
 */
void handleNotFound(AsyncWebServerRequest *request) {
    request->send(404, F(CONTENT_TYPE_PLAIN), F("Not Found"));
}

/**
 * Initialize web server and routes
 */
void webserver_start(bool isReady) {
    server.onNotFound(handleNotFound);

    server.serveStatic(STATIC_URL_PATH, SPIFFS, STATIC_FILES_PATH);

    if (isReady) {
        initalizeRoutes();
    } else {
        initalizeSetupRoutes();
    }

    /**
     *  POST
     */
    server.on("/wifi", HTTP_POST, handleWifiSetupForm);

#ifdef SPIFFS_EDITOR
    server.addHandler(new SPIFFSEditor(SPIFFS));
#endif

    server.begin();
}