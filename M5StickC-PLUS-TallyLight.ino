#include <M5StickCPlus.h>
#include "NetworkModule.h"
#include "WebSocketsModule.h"
#include "PrefsModule.h"
#include "PowerModule.h"
#include "ScreenModule.h"
#include "millisDelay.h"

millisDelay ms_startup;


void setup () {

    M5.begin();
    setCpuFrequencyMhz(80); //Save battery by turning down the CPU clock
    btStop();               //Save battery by turning off Bluetooth

    ms_startup.start(30000);

    currentScreen = 0;      // boot screen
    M5.Lcd.setRotation(3);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextWrap(true);
    M5.Lcd.println(F("Starting..."));

    M5.Lcd.println(F("Initializing preferences..."));
    preferences_setup();
    
    M5.Lcd.println(F("Initializing power management..."));
    power_setup();
    
    M5.Lcd.println(F("Initializing WiFi..."));
    WiFi_setup();
    
    M5.Lcd.println(F("Initializing webSockets..."));
    webSockets_setup();

    while (ms_startup.isRunning()) {
        WiFi_onLoop();
        webSockets_onLoop();
        power_onLoop();
        if (ws_isConnected & time_isSet) ms_startup.stop();
    }

    M5.Lcd.println();
    M5.Lcd.setTextSize(2);
    M5.Lcd.println(F("Startup complete."));
    M5.Lcd.println(F("Press \"M5\" button \r\nto continue."));
    M5.Lcd.println();
    
}

void loop () {

    M5.update();
    WiFi_onLoop();
    webSockets_onLoop();
    power_onLoop();

    // M5 Button
    if (M5.BtnA.wasReleased()) {
        if (currentScreen == 3) currentScreen = 0;  // reset
        currentScreen = currentScreen + 1;
        changeScreen();
    }

    // Action Button
    if (M5.BtnB.wasReleased()) {
        setBrightness(0);
    }

    refreshScreen();

}