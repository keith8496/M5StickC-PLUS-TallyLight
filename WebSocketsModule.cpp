#include <M5StickCPlus.h>
#include <WebSocketsClient.h>
#include <millisDelay.h>
#include <ArduinoJson.h>
#include "PrefsModule.h"
#include "ScreenModule.h"
#include "PowerModule.h"
#include "NetworkModule.h"

WebSocketsClient ws;
bool ws_isConnected = false;

int atem_pgm1_input_id = 0;
int atem_pvw1_input_id = 0;

millisDelay md_sendStatus;


// Define Functions
void webSockets_setup();
void webSockets_onLoop();
void webSockets_onEvent(WStype_t type, uint8_t* payload, size_t length);
void webSockets_onTally(DynamicJsonDocument doc);
void webSockets_getTally();


void webSockets_onLoop() {
    
    if (md_sendStatus.justFinished()) {
        
        md_sendStatus.repeat();
        
        char buff[17];
        ultoa(inputIds, buff, 2);
        StaticJsonDocument<512> doc;
        
        doc["deviceId"] = deviceId;
        doc["MessageType"] = "DeviceStatus";
        doc["MessageData"]["friendlyName"] = friendlyName;
        doc["MessageData"]["inputIds"] = buff;
        doc["MessageData"]["batPercentage"] = pwr.batPercentage;
        doc["MessageData"]["batCurrent"] = pwr.batCurrent;
        doc["MessageData"]["batChargeCurrent"] = pwr.batChargeCurrent;
        doc["MessageData"]["maxChargeCurrent"] = pwr.chargeCurrent;
        doc["MessageData"]["tempInAXP192"] = pwr.tempInAXP192;
        doc["MessageData"]["powerMode"] = pwr.powerMode;
        doc["MessageData"]["currentBrightness"] = currentBrightness;
        doc["MessageData"]["currentScreen"] = currentScreen;
        doc["MessageData"]["webPortalActive"] = wm.getWebPortalActive();
        doc["MessageData"]["ntp"] = time_isSet;
        doc["MessageData"]["ssid"] = String(wm.getWiFiSSID());
        doc["MessageData"]["rssi"] = String(WiFi.RSSI());
        doc["MessageData"]["ip"] = WiFi.localIP().toString();
        doc["MessageData"]["hostname"] = wm.getWiFiHostname();

        String json;
        json.reserve(512);
        serializeJson(doc, json);
        //Serial.println(json);
        ws.sendTXT(json);

    }
    
    ws.loop();
}


void webSockets_setup() {
    
  Serial.println(F("Attempting to connect to websockets..."));
  ws.onEvent(webSockets_onEvent);
  ws.setReconnectInterval(5000);
  //ws.enableHeartbeat(60000, 1000, 60);
  ws.begin(nodeRED_ServerIP, nodeRED_ServerPort, nodeRED_ServerUrl);
  ws.loop();

}


void webSockets_onEvent(WStype_t type, uint8_t* payload, size_t length) {

    switch(type) {
	
        case WStype_ERROR:
        Serial.println("Websockets error detected.");
        break;
        
        case WStype_DISCONNECTED:
            ws_isConnected = false;
            Serial.println("Websockets disconnected.");
            break;
        
        case WStype_CONNECTED:
            ws_isConnected = true;
            Serial.println("Websockets connected.");
            if (currentScreen == 0) startupLog("Websockets connected.", 1);
            webSockets_getTally();
            md_sendStatus.start(60000);
            break;
            
        case WStype_TEXT: {
        
            DynamicJsonDocument doc(length);
            DeserializationError error = deserializeJson(doc, payload, length);
            const char* MessageType = doc["MessageType"];

            if (MessageType == nullptr) {
                Serial.print(F("MessageType is nullptr"));
                return;
            } else if (strcmp(MessageType, "SetTally") == 0) {
                webSockets_onTally(doc);
            }
            
            break;
        }
        
        case WStype_BIN:
            break;
        
        case WStype_PING: {
            char buff[65];
            strcpy(buff, "Local Time: ");
            strcat(buff, localTime.dateTime(ISO8601).c_str());
            Serial.print(buff);
            Serial.println(F(" Websockets PING"));
            break;
        }
        
        case WStype_PONG: {
            char buff[65];
            strcpy(buff, "Local Time: ");
            strcat(buff, localTime.dateTime(ISO8601).c_str());
            Serial.print(buff);
            Serial.println(F(" Websockets PONG"));
            break;
        }
       
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }

}


void webSockets_onTally(DynamicJsonDocument doc) {

    /*Serial.println("webSockets_onTally()");
    serializeJson(doc, Serial);
    Serial.println();*/
    
    const char* Source = doc["deviceId"];
    const char* EventType = doc["MessageData"]["EventType"];
    int EventValue;

    if (Source == nullptr || EventType == nullptr) {
        return;
    } else if (strcmp(EventType, "atem_pgm1_input_id") == 0) {
        EventValue = doc["MessageData"][EventType];
        atem_pgm1_input_id = EventValue;
    } else if (strcmp(EventType, "atem_pvw1_input_id") == 0) {
        EventValue = doc["MessageData"][EventType];
        atem_pvw1_input_id = EventValue;
    }

    /*Serial.print("Source: ");
    Serial.println(Source);
    Serial.print("EventType: ");
    Serial.println(EventType);
    Serial.print("EventValue: ");
    Serial.println(EventValue);*/

}


void webSockets_getTally() {
    ws.sendTXT("{\"deviceId\": \"" + String(deviceId) + "\", " +
                "\"MessageType\": \"GetTally\"}");
}