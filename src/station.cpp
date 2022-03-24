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

#include <ArduinoJson.h>
#include <ESPmDNS.h>

#define MDNS_QUERY_TYPE "http"
#define MDNS_QUERY_PROTOCOL "tcp"
#define MDNS_NAME_PREFIX "spoddify-mopped-"

#define SOCKET_IO_PATH "/socket.io/?EIO=4"

SocketIOclient socketIO;

String host = "";
u16_t port = 0;

String track = "";
String artist = "";
String album = "";
float heights[23];

ConnectionChangedCallback connectionChangedCallback;
PlayerEventCallback playerEventCallback;

bool connected = false;

/**
 * Requests a player event.
 */
void getPlayer() {
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();
    array.add("action");

    JsonObject event = array.createNestedObject();
    event["type"] = "WS_TO_SERVER_GET_PLAYER_STATE";

    String raw;
    serializeJson(doc, raw);
    socketIO.sendEVENT(raw);
}

/**
 * Handles Socket.io events.
 */
void handleEvent(socketIOmessageType_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case sIOtype_DISCONNECT:
            Serial.printf("[Socket.io] Disconnected!\n");

            if (connected) {
                connected = false;
                connectionChangedCallback(connected);
            }

            break;
        case sIOtype_CONNECT:
            Serial.printf("[Socket.io] Connected to url: %s\n", payload);
            socketIO.send(sIOtype_CONNECT, "/");

            if (!connected) {
                connected = true;
                connectionChangedCallback(connected);
                getPlayer();
            }
            break;
        case sIOtype_EVENT: {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload, length);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }

            String eventName = doc[0];

            if (eventName == "action") {
                String eventType = doc[1]["type"];
                if (eventType == "WS_TO_CLIENT_SET_PLAYER_STATE") {
                    String pTrack = doc[1]["payload"]["item"]["name"];
                    String pArtist =
                        doc[1]["payload"]["item"]["artists"][0]["name"];
                    String pAlbum = doc[1]["payload"]["item"]["album"]["name"];

                    bool changed = false;

                    if (pTrack != "null" && pTrack != track) {
                        track = pTrack;
                        changed = true;
                    }

                    if (pArtist != "null" && pArtist != artist) {
                        artist = pArtist;
                        changed = true;
                    }

                    if (pAlbum != "null" && pAlbum != album) {
                        album = pAlbum;
                        changed = true;
                    }

                    for (int i = 0; i < 23; i++) {
                        heights[i] = doc[1]["payload"]["heights"][i];
                    }

                    if (changed) {
                        playerEventCallback(track, artist, album, heights);
                    }
                }
            }

            break;
        }
        default: {
        }
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
void StationClientClass::init() {
    if (searchStation()) {
        socketIO.begin(host, port, SOCKET_IO_PATH);
        socketIO.onEvent(handleEvent);
    } else {
        connectionChangedCallback(connected);
    }
}

/**
 * Adds the connection changed handler.
 */
void StationClientClass::addConnectionChangedHandler(
    ConnectionChangedCallback callback) {
    connectionChangedCallback = callback;
}

/**
 * Adds the player event handler.
 */
void StationClientClass::addPlayerEventHandler(PlayerEventCallback callback) {
    playerEventCallback = callback;
}

/**
 * Determine wheather the client is connected or not.
 */
bool StationClientClass::isConnected() { return connected; }

/**
 * Get the host of the station.
 */
String StationClientClass::getStationHost() {
    if (host.isEmpty() || port == 0) {
        return "";
    }

    return "http://" + host + ":" + port;
}

/**
 * Put this in the loop.
 */
void StationClientClass::loop() { socketIO.loop(); }

StationClientClass StationClient;