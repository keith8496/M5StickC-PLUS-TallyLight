#include <M5StickCPlus.h>
#include "NetworkModule.h"
#include "WebSocketsModule.h"
#include "PrefsModule.h"
#include "PowerModule.h"
#include "ScreenModule.h"
#include "millisDelay.h"

millisDelay ms_startup;

uint64_t getHashedInt64(uint64_t u);
String getUniqueId();

char deviceId[17];
char deviceName[33];


void setup () {

    M5.begin();
    M5.Lcd.setRotation(3);
    setCpuFrequencyMhz(80); //Save battery by turning down the CPU clock
    btStop();               //Save battery by turning off Bluetooth

    ms_startup.start(30000);

    // Set deviceId and deviceName
    uint8_t macAddress[6];
    WiFi.macAddress(macAddress);
    sprintf(deviceId, "%02X%02X%02X", macAddress[3], macAddress[4], macAddress[5]);
    strcpy(deviceName, "M5StickC-Plus-");
    strcat(deviceName, deviceId);
    
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

    Serial.println("Entering ms_startup_isRunning");
    while (ms_startup.isRunning()) {
        WiFi_onLoop();
        webSockets_onLoop();
        power_onLoop();
        if (ws_isConnected & time_isSet) ms_startup.stop();
        if (ms_startup.justFinished()) ms_startup.stop();
    }
    Serial.println("Exited ms_startup_isRunning");
    
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


/** Returns a hashed uint64_t */
uint64_t getHashedInt64(uint64_t u)
{
    uint64_t v = u * 3935559000370003845 + 2691343689449507681;

    v ^= v >> 21;
    v ^= v << 37;
    v ^= v >> 4;

    v *= 4768777513237032717;

    v ^= v << 20;
    v ^= v >> 41;
    v ^= v << 5;

    return v;
}

/** Return a device unique identifier based on device's mac address, eg. 6C4080B51A5D3659 */
String getUniqueId()
{
    uint64_t mac = ESP.getEfuseMac();
    uint64_t hash = getHashedInt64(mac);
    uint32_t msb = hash >> 32;
    uint32_t lsb = hash & 0xFFFFFFFF;

    char unique_id[20];
    sprintf(unique_id, "%08X%08X", msb, lsb);
    return String(unique_id);
}