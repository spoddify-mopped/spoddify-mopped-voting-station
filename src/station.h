/**
 * Spoddify Mopped connector
 */

#ifndef Station_h
#define Station_h

#include <WString.h>

class StationClientClass {
   public:
    static bool init();
    static bool isConnected();
    static String getStationHost();
    static void loop();
};

extern StationClientClass StationClient;

#endif