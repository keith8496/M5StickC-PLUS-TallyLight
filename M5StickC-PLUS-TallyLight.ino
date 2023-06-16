#include <M5StickCPlus.h>
#include "NetworkModule.h"


void setup () {

    Serial.begin(115200);
    
    M5.begin();
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setRotation(3);
    M5.Lcd.println(F("Booting..."));

    setCpuFrequencyMhz(80); //Save battery by turning down the CPU clock
    btStop();               //Save battery by turning off Bluetooth

    setupWiFi();
 
}

void loop () {

}