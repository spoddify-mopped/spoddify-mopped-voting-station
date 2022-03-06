/**
 * Spoddify Mopped voting remote
 */

#include <GxEPD2_BW.h>
#include <SD.h>
#include <fonts/FiraSans_Medium_1_8pt7b.h>

#include "SPIFFS.h"
#include "filesystem.h"
#include "station.h"
#include "webserver.h"
#include "wifimanager.h"

/**
 * Display configuration
 */
#define LILYGO_T5_V213
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_213_B74
#define GxEPD2_BW_IS_GxEPD2_BW true

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(
    /*CS=5*/ 5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));  // GxEPD2_213_B74
#define MAX_HEIGHT(EPD)                                        \
    (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) \
         ? EPD::HEIGHT                                         \
         : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8));

void drawCenteredText(const char* text) {
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;

    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y);
        display.print(text);
    } while (display.nextPage());
}

void drawSetup() {
    String text = "Connect with Wi-Fi to: '";
    text.concat(WifiManager.apName());
    text.concat("' and open http://");
    text.concat(WifiManager.apIP());
    drawCenteredText(text.c_str());
}

void setupDisplay() {
    display.init();
    display.setRotation(1);
    display.setFont(&FiraSans_Medium_1_8pt7b);
    display.setTextColor(GxEPD_BLACK);
}

void setup() {
    Serial.begin(115200);

    setupDisplay();

    drawCenteredText("Loading...");

    if (!Filesystem.init()) {
        return;
    }

    bool isReady = WifiManager.connect();
    
    if (isReady) {
        if (StationClient.init()) {
            drawCenteredText("Connection established");
            StationClient.init();
        } else {
            drawCenteredText("No station found!");
        }
    } else {
        WifiManager.setupAP();
        drawSetup();
    }

    webserver_start(isReady);
}

void loop() { StationClient.loop(); }