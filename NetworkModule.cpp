#include <M5StickCPlus.h>
#include <WiFiManager.h>
#include <millisDelay.h>

WiFiManager wm;
millisDelay md_setNtpTime;

// Define Functions
void WiFi_setup();
void WiFi_onLoop();
void WiFi_onEvent(WiFiEvent_t event);
void saveParamCallback();


void WiFi_onLoop() {
    
    if (wm.getWebPortalActive()) wm.process();
    
    if (md_setNtpTime.justFinished()) {
        const long gmtOffset_sec = -21600;
        const int daylightOffset_sec = 3600;
        const char* server1 = "time.apple.com";                
        configTime(gmtOffset_sec, daylightOffset_sec, server1);
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            Serial.println(F("Failed to obtain time"));
            md_setNtpTime.repeat();
            return;
        }        
        md_setNtpTime.stop();
        Serial.println(&timeinfo, "%A %B %d, %Y %H:%M:%S");
    }

}


void WiFi_setup () {

    WiFi.mode(WIFI_STA);
    WiFi.onEvent(WiFi_onEvent);
    
    char deviceId[9];
    char deviceName[33];
    ultoa(WIFI_getChipId(), deviceId, 16);
    strcpy(deviceName, "M5StickC-Plus-");
    strcat(deviceName, deviceId);

    std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
    wm.setMenu(menu);
    wm.setConfigPortalBlocking(false);    
    wm.setDebugOutput(false);
    wm.setSaveParamsCallback(saveParamCallback);
    wm.setClass("invert"); // set dark theme
    wm.setCountry("US");
    
    M5.Lcd.println(F("Starting WiFi..."));
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

    M5.Lcd.println(wm.getWLStatusString());
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
          M5.Lcd.println(F("Connected to access point"));          
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


void saveParamCallback () {

}