/**
 * Spoddify Mopped connector
 */

#ifndef Station_h
#define Station_h

#include <WString.h>

typedef void (*PlayerEventCallback)(String track, String artist, String album,
                                    float heights[23]);

typedef void (*ConnectionChangedCallback)(bool connected);

class StationClientClass {
   public:
    static void init();
    static void addConnectionChangedHandler(ConnectionChangedCallback callback);
    static void addPlayerEventHandler(PlayerEventCallback callback);
    static bool isConnected();
    static String getStationHost();
    static uint16_t getStationPort();
    static void refreshPlayer();
    static void dislike();
    static void like();
    static void loop();
};

extern StationClientClass StationClient;

#endif