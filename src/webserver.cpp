/**
 * Web server
 */

#include "webserver.h"

#include <ESPAsyncWebServer.h>

#include "SPIFFS.h"
#include "wifimanager.h"

/**
 * This enables the SPIFFS editor under '/edit'.
 * It should be disabled for production.
 */
#define SPIFFS_EDITOR

#ifdef SPIFFS_EDITOR
#include "SPIFFSEditor.h"
#endif

#define CONTENT_TYPE_PLAIN "text/plain"
#define CONTENT_TYPE_HTML "text/html"

#define STATIC_URL_PATH "/static"
#define STATIC_FILES_PATH "/web/static"

AsyncWebServer server(80);

/**
 * Handles request to show the wifi setup page.
 */
void handleWifiSetupPage(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/web/setup/setup.html", CONTENT_TYPE_HTML);
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

    if (p == "IP_PLACEHOLDER") {
        return WifiManager.IP();
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
    request->send(204, CONTENT_TYPE_PLAIN, "");
    delay(1000);
    ESP.restart();
}

/**
 * Handles request to reset the esp.
 */
void handleReset(AsyncWebServerRequest *request) {
    WifiManager.reset();

    request->send(204, CONTENT_TYPE_PLAIN, "");
    delay(1000);
    ESP.restart();
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
    int paramsSize = request->params();

    for (int i = 0; i < paramsSize; i++) {
        AsyncWebParameter *param = request->getParam(i);
        if (param->isPost()) {
            if (param->name() == "ssid") {
                WifiManager.saveSSID(param->value().c_str());
            }
            if (param->name() == "pass") {
                WifiManager.savePassword(param->value().c_str());
            }
        }
    }

    request->send(204, CONTENT_TYPE_PLAIN, "");
    delay(1000);
    ESP.restart();
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