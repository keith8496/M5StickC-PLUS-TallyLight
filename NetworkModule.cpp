#include <M5StickCPlus.h>
#include <WiFiManager.h>
#include <millisDelay.h>
#include "PrefsModule.h"
#include "WebSocketsModule.h"
#include "ScreenModule.h"

WiFiManager wm;
millisDelay md_setNtpTime;

// Parameters
WiFiManagerParameter wm_friendlyName("friendlyName", "Friendly Name");
WiFiManagerParameter wm_inputIds("inputIds", "Input IDs (0000000000000001)");
WiFiManagerParameter wm_nodeRED_ServerIP("nr_ServerIP", "Node-RED Server IP");
WiFiManagerParameter wm_nodeRED_ServerPort("nr_ServerPort", "Node-RED Server Port");
WiFiManagerParameter wm_nodeRED_ServerUrl("nr_ServerUrl", "Node-RED Server URL");
WiFiManagerParameter wm_ntpServer("ntpServer", "NTP Server");
WiFiManagerParameter wm_gmtOffset_sec("gmtOffset_sec", "GMT Offset Seconds");
WiFiManagerParameter wm_daylightOffset_sec("daylightOffset", "Daylight Offset Seconds");

char deviceId[9];
char deviceName[33];
bool time_isSet = false;


// Define Functions
void WiFi_setup();
void WiFi_onLoop();
void WiFi_onEvent(WiFiEvent_t event);
void WiFi_onSaveParams();


void WiFi_onLoop() {
    
    if (wm.getWebPortalActive()) wm.process();
    
    if (md_setNtpTime.justFinished()) {               
        if (currentScreen == 0) M5.Lcd.println("Initializing NTP...");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        struct tm timeInfo;
        if (!getLocalTime(&timeInfo)) {
            Serial.println(F("Failed to obtain time"));
            md_setNtpTime.repeat();
        } else {
            RTC_TimeTypeDef TimeStruct;
            RTC_DateTypeDef DateStruct;
            TimeStruct.Hours   = timeInfo.tm_hour;
            TimeStruct.Minutes = timeInfo.tm_min;
            TimeStruct.Seconds = timeInfo.tm_sec;
            DateStruct.WeekDay = timeInfo.tm_wday;
            DateStruct.Month = timeInfo.tm_mon + 1;
            DateStruct.Date = timeInfo.tm_mday;
            DateStruct.Year = timeInfo.tm_year + 1900;
            M5.Rtc.SetTime(&TimeStruct);
            M5.Rtc.SetData(&DateStruct);
            time_isSet = true;
            md_setNtpTime.stop();
            Serial.println(&timeInfo, "%A %B %d, %Y %H:%M:%S");
            if (currentScreen == 0) M5.Lcd.println(&timeInfo, "%A %B %d, %Y %H:%M:%S");
        }
    }

}


void WiFi_setup () {

    WiFi.mode(WIFI_STA);
    WiFi.onEvent(WiFi_onEvent);
    
    ultoa(WIFI_getChipId(), deviceId, 16);
    strcpy(deviceName, "M5StickC-Plus-");
    strcat(deviceName, deviceId);

    
    // wm_addParameters
    wm.addParameter(&wm_friendlyName);
    wm.addParameter(&wm_inputIds);
    wm.addParameter(&wm_nodeRED_ServerIP);
    wm.addParameter(&wm_nodeRED_ServerPort);
    wm.addParameter(&wm_nodeRED_ServerUrl);
    wm.addParameter(&wm_ntpServer);
    wm.addParameter(&wm_gmtOffset_sec);
    wm.addParameter(&wm_daylightOffset_sec);
    
    // set wm values
    char buff[17];
    wm_friendlyName.setValue(friendlyName, sizeof(friendlyName));
    ultoa(inputIds, buff, 2);
    wm_inputIds.setValue(buff, sizeof(buff));
    wm_nodeRED_ServerIP.setValue(nodeRED_ServerIP, sizeof(nodeRED_ServerIP));
    itoa(nodeRED_ServerPort, buff, 10);
    wm_nodeRED_ServerPort.setValue(buff, sizeof(buff));
    wm_nodeRED_ServerUrl.setValue(nodeRED_ServerUrl, sizeof(nodeRED_ServerUrl));
    wm_ntpServer.setValue(ntpServer, sizeof(ntpServer));
    itoa(gmtOffset_sec, buff, 10);
    wm_gmtOffset_sec.setValue(buff, sizeof(wm_gmtOffset_sec));
    itoa(daylightOffset_sec, buff, 10);
    wm_daylightOffset_sec.setValue(buff, sizeof(wm_daylightOffset_sec));

    
    std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
    wm.setMenu(menu);
    wm.setConfigPortalBlocking(false);    
    wm.setDebugOutput(false);
    wm.setSaveParamsCallback(WiFi_onSaveParams);
    wm.setClass("invert"); // set dark theme
    wm.setCountry("US");
    
    if (!wm.autoConnect(deviceName)) {
        M5.Lcd.println(F("Config Portal Active"));
        while (wm.getConfigPortalActive()) {
            wm.process();
            M5.update();
            if (M5.BtnA.wasReleased()) {
                wm.stopConfigPortal();
            }
        }
        M5.Lcd.println(F("Config Portal Closed"));
    }

    md_setNtpTime.start(1000);
    
}


void WiFi_onEvent(WiFiEvent_t event){
  
  //Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
      
      /*case ARDUINO_EVENT_WIFI_READY: 
          Serial.println("WiFi interface ready");
          break;
      
      case ARDUINO_EVENT_WIFI_SCAN_DONE:
          Serial.println("Completed scan for access points");
          break;
      
      case ARDUINO_EVENT_WIFI_STA_START:
          Serial.println("WiFi client started");
          break;
      
      case ARDUINO_EVENT_WIFI_STA_STOP:
          Serial.println("WiFi clients stopped");
          break;*/
      
      case ARDUINO_EVENT_WIFI_STA_CONNECTED:          
          Serial.println(F("Connected to access point"));
          if (currentScreen == 0) M5.Lcd.println(F("Connected to access point"));          
          break;

      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
          Serial.println("Disconnected from WiFi access point");
          break;
      
      /*case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
          Serial.println("Authentication mode of access point has changed");
          break;*/
      
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
          Serial.print(F("Obtained IP address: "));
          Serial.println(WiFi.localIP());
          if (currentScreen == 0) {
            M5.Lcd.print(F("Obtained IP address: "));
            M5.Lcd.println(WiFi.localIP());
          }
          break;
      
      case ARDUINO_EVENT_WIFI_STA_LOST_IP:
          Serial.println("Lost IP address and IP address is reset to 0");
          break;
      
      /*case ARDUINO_EVENT_WPS_ER_SUCCESS:
          Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
          break;
      
      case ARDUINO_EVENT_WPS_ER_FAILED:
          Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
          break;
      
      case ARDUINO_EVENT_WPS_ER_TIMEOUT:
          Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
          break;
      
      case ARDUINO_EVENT_WPS_ER_PIN:
          Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
          break;
      
      case ARDUINO_EVENT_WIFI_AP_START:
          Serial.println("WiFi access point started");
          break;
      
      case ARDUINO_EVENT_WIFI_AP_STOP:
          Serial.println("WiFi access point  stopped");
          break;
      
      case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
          Serial.println("Client connected");
          break;
      
      case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
          Serial.println("Client disconnected");
          break;
      
      case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
          Serial.println("Assigned IP address to client");
          break;
      
      case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
          Serial.println("Received probe request");
          break;
      
      case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
          Serial.println("AP IPv6 is preferred");
          break;
      
      case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
          Serial.println("STA IPv6 is preferred");
          break;
      
      case ARDUINO_EVENT_ETH_GOT_IP6:
          Serial.println("Ethernet IPv6 is preferred");
          break;
      
      case ARDUINO_EVENT_ETH_START:
          Serial.println("Ethernet started");
          break;
      
      case ARDUINO_EVENT_ETH_STOP:
          Serial.println("Ethernet stopped");
          break;`
      
      case ARDUINO_EVENT_ETH_CONNECTED:
          Serial.println("Ethernet connected");
          break;
      
      case ARDUINO_EVENT_ETH_DISCONNECTED:
          Serial.println("Ethernet disconnected");
          break;
      
      case ARDUINO_EVENT_ETH_GOT_IP:
          Serial.println("Obtained IP address");
          break;*/
      
      default: 
        break;
  }

}


void WiFi_onSaveParams() {

    strcpy(friendlyName, wm_friendlyName.getValue());
    inputIds = static_cast<uint16_t>(strtol(wm_inputIds.getValue(), NULL, 2));
    strcpy(nodeRED_ServerIP, wm_nodeRED_ServerIP.getValue());
    nodeRED_ServerPort = atoi(wm_nodeRED_ServerPort.getValue());
    strcpy(nodeRED_ServerUrl, wm_nodeRED_ServerUrl.getValue());
    gmtOffset_sec = atoi(wm_gmtOffset_sec.getValue());
    daylightOffset_sec = atoi(wm_daylightOffset_sec.getValue());
    strcpy(ntpServer, wm_ntpServer.getValue());

    preferences_save();
    webSockets_getTally();

}