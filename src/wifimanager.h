/**
 * WiFi manager
 */

#ifndef Wifimanager_h
#define Wifimanager_h

#include <WString.h>

class WifiManagerClass {
   public:
    static String SSID();
    static String IP();
    static bool connect();
    static void setupAP();
    static String apName();
    static String apIP();
    static void saveCredentials(String ssid, String password);
    static void reset();
};

extern WifiManagerClass WifiManager;

#endif