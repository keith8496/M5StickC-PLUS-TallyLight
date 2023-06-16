#include <WebSocketsClient.h>
#include <millisDelay.h>
#include <ArduinoJson.h>

WebSocketsClient ws;

int atem_pgm1_input_id = 0;
int atem_pvw1_input_id = 0;


// Define Functions
void webSockets_setup();
void webSockets_onLoop();
void webSockets_onEvent(WStype_t type, uint8_t* payload, size_t length);


void webSockets_onLoop() {
    ws.loop();
}


void webSockets_setup() {
  
  const char nodeRED_ServerIP[16] = "192.168.13.54";
  const int nodeRED_ServerPort = 1880;
  const char nodeRED_ServerUrl[33] = "/ws/tally";
  
  Serial.println(F("Attempting to connect to websockets..."));
  ws.onEvent(webSockets_onEvent);
  ws.setReconnectInterval(5000);
  ws.enableHeartbeat(60000, 1000, 60);
  ws.begin(nodeRED_ServerIP, nodeRED_ServerPort, nodeRED_ServerUrl);

}


void webSockets_onEvent(WStype_t type, uint8_t* payload, size_t length) {

    switch(type) {
	
        case WStype_ERROR:
        Serial.println("Websockets error detected.");
        break;
        
        case WStype_DISCONNECTED:
            Serial.println("Websockets disconnected.");
            break;
        
        case WStype_CONNECTED:
            Serial.println("Websockets connected.");            
            //ws.sendTXT("{\"deviceId\": \"" + String(deviceId) + "\", " +
            //                       "\"MessageType\": \"GetConfig\"}");
            break;
            
        case WStype_TEXT: {
        
            /*DynamicJsonDocument doc(length);
            DeserializationError error = deserializeJson(doc, payload, length);
            const char* MessageType = doc["MessageType"];

            if (MessageType == nullptr) {
                Serial.print("MessageType is nullptr");
                return;
            } else if (strcmp(MessageType, "Tally") == 0) {
                webSocketOnTextTally(doc);
            }*/
            
            break;
        }
        
        case WStype_BIN:
            break;
        case WStype_PING:
            Serial.println("Websockets PING.");
            break;
        case WStype_PONG:
            Serial.println("Websockets PONG.");
            break;
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }

}