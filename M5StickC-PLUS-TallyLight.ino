#include <M5StickCPlus.h>
#include "NetworkModule.h"
#include <millisDelay.h>

millisDelay setTimeDelay;
void setTime();


void setup () {

    Serial.begin(115200);
    
    M5.begin();
    setCpuFrequencyMhz(80); //Save battery by turning down the CPU clock
    btStop();               //Save battery by turning off Bluetooth

    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setRotation(3);
    M5.Lcd.println(F("Booting..."));

    setupWiFi();
    setTimeDelay.start(1000);
}

void loop () {

    if (setTimeDelay.justFinished()) setTime();

}


void setTime() {
    
    configTime(-21600, 3600, "time.apple.com");
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println(F("Failed to obtain time"));
        setTimeDelay.repeat();
        return;
    }
    
    setTimeDelay.stop();
    Serial.println(&timeinfo, "%A %B %d, %Y %H:%M:%S");

}