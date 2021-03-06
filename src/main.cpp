/**
 * Spoddify Mopped voting remote
 */

#include <GxEPD2_BW.h>
#include <SD.h>
#include <fonts/FiraSans_Medium_1_6pt7b.h>
#include <fonts/FiraSans_Medium_1_8pt7b.h>

#include "SPIFFS.h"
#include "assets/icons.h"
#include "filesystem.h"
#include "station.h"
#include "webserver.h"
#include "wifimanager.h"

#include <WiFi.h>

/**
 * Display configuration
 */
#define LILYGO_T5_V213
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_213_B74
#define GxEPD2_BW_IS_GxEPD2_BW true

#define SKIP_BUTTON 21
#define LIKE_BUTTON 26
#define SLEEP_BUTTON 12
#define VOTE_RESET_TIME 3000

bool likeButtonPressed = false;
bool dislikeButtonPressed = false;
bool sleepButtonPressed = false;

uint32_t resetDisplay = 0;

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

void drawCenteredTextWithLogo(const char* text) {
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;

    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(75, 10, logo, 100, 77, GxEPD_BLACK);
        display.setCursor(x, 110);
        display.print(text);
    } while (display.nextPage());
}

void drawSetup() {
    display.setFont(&FiraSans_Medium_1_6pt7b);

    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(-15, 22, logo, 100, 77, GxEPD_BLACK);
        display.setCursor(85, 40);
        display.print("Connect with Wi-Fi to:");
        display.setCursor(85, 60);
        display.print(WifiManager.apName());
        display.setCursor(85, 80);
        display.printf("and open http://%s", WifiManager.apIP().c_str());
    } while (display.nextPage());

    display.setFont(&FiraSans_Medium_1_8pt7b);
}

void setupDisplay() {
    display.init();
    display.setRotation(1);
    display.setFont(&FiraSans_Medium_1_8pt7b);
    display.setTextColor(GxEPD_BLACK);
}

void drawSongInfos(String songName, String artistName, String albumName,
                   float heights[23]) {
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawInvertedBitmap(0, 4, icon_title, 18, 18, GxEPD_BLACK);
        display.drawInvertedBitmap(0, 22, icon_album, 18, 18, GxEPD_BLACK);
        display.drawInvertedBitmap(0, 40, icon_artist, 18, 18, GxEPD_BLACK);
        display.setCursor(18, 18);
        display.print(songName.c_str());
        display.setCursor(18, 36);
        display.print(artistName.c_str());
        display.setCursor(18, 54);
        display.print(albumName.c_str());
        display.drawInvertedBitmap(5, 76, icon_spotify, 40, 40, GxEPD_BLACK);
        for (int i = 0; i < 23; i++) {
            display.fillRect(55 + i * 8, 95 - (heights[i]) / 20.0 * 4, 5,
                             (heights[i]) / 20.0 * 8.1, GxEPD_BLACK);
        }
    } while (display.nextPage());
}

void downvoteCallback() { dislikeButtonPressed = true; }

void upvoteCallback() { likeButtonPressed = true; }

void sleepCallback() { sleepButtonPressed = true; }

void setup() {
    Serial.begin(115200);

    setupDisplay();

    drawCenteredTextWithLogo("Loading...");

    if (!Filesystem.init()) {
        return;
    }

    bool isReady = WifiManager.connect();

    if (isReady) {
        StationClient.addConnectionChangedHandler([](bool connected) {
            if (connected) {
                drawCenteredTextWithLogo("Connection established");
            } else {
                drawCenteredTextWithLogo("No station found!");
            }
        });

        StationClient.addPlayerEventHandler(
            [](String track, String artist, String album, float heights[23]) {
                drawSongInfos(track, artist, album, heights);
            });

        StationClient.init();
    } else {
        WifiManager.setupAP();
        drawSetup();
    }

    webserver_start(isReady);
    pinMode(SKIP_BUTTON, INPUT_PULLUP);
    pinMode(LIKE_BUTTON, INPUT_PULLUP);
    pinMode(SLEEP_BUTTON, INPUT_PULLUP);
    attachInterrupt(SKIP_BUTTON, downvoteCallback, RISING);
    attachInterrupt(LIKE_BUTTON, upvoteCallback, RISING);
    attachInterrupt(SLEEP_BUTTON, sleepCallback, RISING);
}

void loop() {
    if (restartTime > 0 && millis() >= restartTime) {
        restartTime = 0;
        ESP.restart();
    }

    if (likeButtonPressed) {
        likeButtonPressed = false;

        if (StationClient.isConnected()) {
            drawCenteredTextWithLogo("Liked");
            StationClient.like();
            resetDisplay = millis() + VOTE_RESET_TIME;
        }
    }

    if (dislikeButtonPressed) {
        dislikeButtonPressed = false;
        if (StationClient.isConnected()) {
            drawCenteredTextWithLogo("Disliked");
            StationClient.dislike();
            resetDisplay = millis() + VOTE_RESET_TIME;
        }
    }

    if (sleepButtonPressed) {
        drawCenteredTextWithLogo("Zzzzzzz");
        esp_sleep_enable_ext0_wakeup((gpio_num_t) SLEEP_BUTTON, 0);
        esp_deep_sleep_start();
    }

    if (resetDisplay > 0 && millis() >= resetDisplay) {
        resetDisplay = 0;

        if (StationClient.isConnected()) {
            StationClient.refreshPlayer();
        }
    }

    StationClient.loop();
}