
#include <M5StickCPlus.h>
#include "PrefsModule.h"
#include "WebSocketsModule.h"
#include "NetworkModule.h"
#include "PowerModule.h"

int currentScreen = 1;          // 1-Tally, 2-Power, 3-Setup
int currentBrightness;

TFT_eSprite tallyScreen = TFT_eSprite(&M5.Lcd);
TFT_eSprite powerScreen = TFT_eSprite(&M5.Lcd);
TFT_eSprite setupScreen = TFT_eSprite(&M5.Lcd);


void clearScreen() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10,10);
}


void showTallyScreen() {
    clearScreen();
    tallyScreen.createSprite(240, 135);
    tallyScreen.setRotation(3);
    
}


void showPowerScreen() {

    clearScreen();
    powerScreen.createSprite(240, 135);
    powerScreen.setRotation(3);

}


void showSetupScreen() {
    
    if (!wm.getWebPortalActive()) wm.startWebPortal();
    clearScreen();
    setupScreen.createSprite(240, 135);
    setupScreen.setRotation(3);

}


void refreshTallyScreen() {

    bool isProgram = false;
    bool isPreview = false;
    
    // Bitshift on inputIds. Rightmost bit = input 1
    for (int i = 1; i <= 16; i++) {
        bool bitValue = (inputIds >> i-1) & 0x01;
        if ((bitValue) & (i == atem_pgm1_input_id)) isProgram = true;
        if ((bitValue) & (i == atem_pvw1_input_id)) isPreview = true;
    }

    tallyScreen.setTextColor(TFT_WHITE);
    tallyScreen.setCursor(10,80);
    tallyScreen.setTextSize(9);

    if (isProgram) {
        tallyScreen.fillRect(0,0,240,135, TFT_RED);
    } else if (isPreview) {
        tallyScreen.fillRect(0,0,240,135, TFT_GREEN);
    } else {
        tallyScreen.fillRect(0,0,240,135, TFT_BLACK);
    }

    tallyScreen.print(friendlyName);
    tallyScreen.pushSprite(0,0);
    
}


void refreshPowerScreen() {

    powerScreen.fillSprite(BLACK);
    powerScreen.setTextColor(TFT_WHITE);
    powerScreen.setCursor(0,0);
    powerScreen.setTextSize(2);
    powerScreen.println(F("Power Management"));

    powerScreen.setTextSize(1);
    powerScreen.println(pwr.powerMode);
    powerScreen.printf("Bat: %s\r\n  V: %.3fv     %.3f%%\r\n", pwr.batWarningLevel, pwr.batVoltage, pwr.batPercentage);
    powerScreen.printf("  I: %.3fma  I: %.3fma\r\n", pwr.batCurrent, pwr.batChargeCurrent);
    powerScreen.printf("  Imax: %ima  Cmax: %.3f%%\r\n", pwr.chargeCurrent, pwr.batPercentage_M);
    powerScreen.printf("USB:\r\n  V: %.3fv  I: %.3fma\r\n", pwr.vbusVoltage, pwr.vbusCurrent);
    powerScreen.printf("5V-In:\r\n  V: %.3fv  I: %.3fma\r\n", pwr.vinVoltage, pwr.vinCurrent);
    powerScreen.printf("APS:\r\n  V: %.3fv\r\n", pwr.apsVoltage);
    powerScreen.printf("AXP:\r\n  Temp: %.1fc", pwr.tempInAXP192);

    powerScreen.pushSprite(10,10);

}


void refreshSetupScreen() {
    
    setupScreen.setCursor(0,0);
    setupScreen.setTextColor(TFT_WHITE);
    setupScreen.setTextSize(2);
    setupScreen.println(F("Setup Screen"));
    setupScreen.println();
    setupScreen.setTextSize(1);
    setupScreen.println("SSID: " + String(wm.getWiFiSSID()) + " " + String(WiFi.RSSI()));
    setupScreen.println("Webportal Active: " + String(wm.getWebPortalActive()));
    setupScreen.println("Hostname: " + wm.getWiFiHostname());
    setupScreen.println("IP: " + WiFi.localIP().toString());
    setupScreen.println();
    setupScreen.println("Node-RED Server: " + String(nodeRED_ServerIP) + ":" + String(nodeRED_ServerPort));
    setupScreen.pushSprite(10,10);

}


void changeScreen() {

    if (wm.getWebPortalActive()) wm.stopWebPortal();
    tallyScreen.deleteSprite();
    powerScreen.deleteSprite();
    setupScreen.deleteSprite();

    switch (currentScreen) {
        case 1:
            showTallyScreen();
            break;
        case 2:
            showPowerScreen();
            break;
        case 3:
            showSetupScreen();
            break;
        default:
            clearScreen();
            M5.Lcd.println("Invalid Screen!");
            break; 
    }
}


void refreshScreen() {
    switch (currentScreen) {
        case 1:
            refreshTallyScreen();
            break;
        case 2:
            refreshPowerScreen();
            break;
        case 3:
            refreshSetupScreen();
            break;
        default:
            break; 
    }
}


