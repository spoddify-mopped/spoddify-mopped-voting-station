/**
 * Spoddify Mopped connector
 */

#ifndef Station_h
#define Station_h

class StationClientClass {
   public:
    static bool init();
    static void loop();
};

extern StationClientClass StationClient;

#endif