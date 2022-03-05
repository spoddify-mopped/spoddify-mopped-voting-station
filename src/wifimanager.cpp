/**
 * WiFi manager
 */

#include "wifimanager.h"

#include <WiFi.h>

#include "filesystem.h"

#define AP_NAME_PREFIX "SM_setup_"
#define AP_PW NULL

#define WIFI_CONNECT_TIMEOUT 10000

#define SSID_PATH "/ssid.txt"
#define PASS_PATH "/pass.txt"

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
    String ssid = Filesystem.readFile(SSID_PATH);
    String password = Filesystem.readFile(PASS_PATH);

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
 * Saves the given ssid in the filesystem.
 */
void WifiManagerClass::saveSSID(String ssid) {
    Filesystem.writeFile(SSID_PATH, ssid.c_str());
}

/**
 * Saves the given password in the filesystem.
 */
void WifiManagerClass::savePassword(String password) {
    Filesystem.writeFile(PASS_PATH, password.c_str());
}

/**
 * Saves the given password in the filesystem.
 */
void WifiManagerClass::reset() {
    Filesystem.removeFile(PASS_PATH);
    Filesystem.removeFile(SSID_PATH);
}

WifiManagerClass WifiManager;