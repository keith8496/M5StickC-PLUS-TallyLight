#include <M5StickCPlus.h>
#include "NetworkModule.h"
#include "WebSocketsModule.h"
#include "PrefsModule.h"


void setup () {

    Serial.begin(115200);
    
    M5.begin();
    setCpuFrequencyMhz(80); //Save battery by turning down the CPU clock
    btStop();               //Save battery by turning off Bluetooth

    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setRotation(3);
    M5.Lcd.println(F("Booting..."));

    preferences_setup();
    WiFi_setup();
    webSockets_setup();

}

void loop () {

    M5.update();
    WiFi_onLoop();
    webSockets_onLoop();

    // M5 Button
    if (M5.BtnA.wasReleased()) {
        WiFi_toggleWebPortal();
    } 

}