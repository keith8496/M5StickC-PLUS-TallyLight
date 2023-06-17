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
    M5.Lcd.setRotation(3);
    setCpuFrequencyMhz(80); //Save battery by turning down the CPU clock
    btStop();               //Save battery by turning off Bluetooth

    ms_startup.start(30000);

    changeScreen(0);
    startupLog("Starting...", 1);
    startupLog("Initializing preferences...", 1);
    preferences_setup();
    startupLog("Initializing power management...", 1);
    power_setup();
    startupLog("Initializing WiFi...", 1);
    WiFi_setup();
    startupLog("Initializing webSockets...", 1);
    webSockets_setup();

    while (ms_startup.isRunning()) {
        WiFi_onLoop();
        webSockets_onLoop();
        power_onLoop();
        if (ws_isConnected & time_isSet) ms_startup.stop();
    }
    
    startupLog("Startup complete.", 1);
    startupLog("", 1);
    startupLog("Press \"M5\" button \r\nto continue.", 2);

}

void loop () {

    M5.update();
    WiFi_onLoop();
    webSockets_onLoop();
    power_onLoop();

    // M5 Button
    if (M5.BtnA.wasReleased()) {
        changeScreen(-1);
    }

    // Action Button
    if (M5.BtnB.wasReleased()) {
        setBrightness(0);
    }

    refreshScreen();

}