#include <M5StickCPlus.h>
#include "NetworkModule.h"
#include <millisDelay.h>

millisDelay delay_setNtpTime;


void setup () {

    Serial.begin(115200);
    
    M5.begin();
    setCpuFrequencyMhz(80); //Save battery by turning down the CPU clock
    btStop();               //Save battery by turning off Bluetooth

    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setRotation(3);
    M5.Lcd.println(F("Booting..."));

    setupWiFi();
    delay_setNtpTime.start(1000);
}

void loop () {

    if (delay_setNtpTime.justFinished()) setNtpTime(&delay_setNtpTime);

}