/**
 * Spoddify Mopped connector
 */

#include "station.h"

/* The order of these two #includes is important!
 *
 *  #include <WebSocketsClient.h>
 *  #include <SocketIOclient.h>
 */
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#include <HTTPClient.h>
#include <ESPmDNS.h>

#define MDNS_QUERY_TYPE "http"
#define MDNS_QUERY_PROTOCOL "tcp"
#define MDNS_NAME_PREFIX "spoddify-mopped-"

#define SOCKET_IO_PATH "/socket.io/?EIO=4"

SocketIOclient socketIO;
HTTPClient http;

String host;
u16_t port;

/**
 * Fetches the stations player state. 
 */
void getPlayer() {
    String url = "http://" + host + ":" + port + "/api/player";

    http.begin(url.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

/**
 * Handles Socket.io events.
 */
void handleEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case sIOtype_DISCONNECT:
            Serial.printf("[Socket.io] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            Serial.printf("[Socket.io] Connected to url: %s\n", payload);
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT: {
            char* sptr = NULL;
            int id = strtol((char*)payload, &sptr, 10);
            Serial.printf("[IOc] get event: %s id: %d\n", payload, id);
            break;
        }
        default: {}
    }
}

/**
 * Searches for stations via mDNS.
 */
bool searchStation() {
    if (mdns_init() != ESP_OK) {
        return false;
    }

    int serviceCount = MDNS.queryService(MDNS_QUERY_TYPE, MDNS_QUERY_PROTOCOL);

    if (serviceCount == 0) {
        return false;
    } else {
        for (int i = 0; i < serviceCount; i = i + 1) {
            if (MDNS.hostname(i).startsWith(MDNS_NAME_PREFIX)) {
                host = MDNS.IP(i).toString();
                port = MDNS.port(i);
                return true;
            }
        }
    }

    return false;
}

/**
 * Initalizes the station client.
 * Discovery of the station and socket.io connetion.
 */
bool StationClientClass::init() {
    if (!searchStation()) {
        return false;
    }

    socketIO.begin(host, port, SOCKET_IO_PATH);
    socketIO.onEvent(handleEvent);

    return true;
}

/**
 * Put this in the loop.
 */
void StationClientClass::loop() { socketIO.loop(); }

StationClientClass StationClient;