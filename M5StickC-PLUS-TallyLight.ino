#include <M5StickCPlus.h>
#include "NetworkModule.h"
#include "WebSocketsModule.h"


void setup () {

    Serial.begin(115200);
    
    M5.begin();
    setCpuFrequencyMhz(80); //Save battery by turning down the CPU clock
    btStop();               //Save battery by turning off Bluetooth

    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setRotation(3);
    M5.Lcd.println(F("Booting..."));

    WiFi_setup();
    webSockets_setup();

}

void loop () {

    WiFi_onLoop();
    webSockets_onLoop();

}