/**
 * WiFi manager
 */

#include "wifimanager.h"

#include <ArduinoJson.h>
#include <WiFi.h>

#include "filesystem.h"

#define AP_NAME_PREFIX "SM_setup_"
#define AP_PW NULL

#define WIFI_CONNECT_TIMEOUT 10000

#define CREDENTIALS_PATH "/settings.json"

/**
 * Returns the SSID of the connected WiFi network.
 */
String WifiManagerClass::SSID() { return WiFi.SSID(); }

/**
 * Returns the local ip of the esp.
 */
String WifiManagerClass::IP() { return WiFi.localIP().toString(); }

/**
 * Connects to the the stored WiFi network.
 */
bool WifiManagerClass::connect() {
    String credentialsRaw = Filesystem.readFile(CREDENTIALS_PATH);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, credentialsRaw);

    String ssid = doc["ssid"];
    String password = doc["pass"];

    if (ssid.isEmpty()) {
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < WIFI_CONNECT_TIMEOUT) {
        delay(100);
    }

    return WiFi.status() == WL_CONNECTED;
}

/**
 * Returns the name of the access point.
 */
String WifiManagerClass::apName() {
    String apName = AP_NAME_PREFIX;
    apName.concat(WiFi.macAddress());
    apName.replace(":", "");

    return apName;
}

/**
 * Starts the setup acces point.
 */
void WifiManagerClass::setupAP() { WiFi.softAP(apName().c_str(), AP_PW); }

/**
 * Returns the ip of the access point.
 */
String WifiManagerClass::apIP() { return WiFi.softAPIP().toString(); }

/**
 * Saves the given wifi credentials in the filesystem.
 */
void WifiManagerClass::saveCredentials(String ssid, String password) {
    DynamicJsonDocument doc(1024);

    doc["ssid"] = ssid;
    doc["pass"] = password;

    String raw;
    serializeJson(doc, raw);

    Filesystem.writeFile(CREDENTIALS_PATH, raw.c_str());
}

/**
 * Saves the given password in the filesystem.
 */
void WifiManagerClass::reset() {
    Filesystem.removeFile(CREDENTIALS_PATH);
}

WifiManagerClass WifiManager;