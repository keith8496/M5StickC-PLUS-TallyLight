
#include <M5StickCPlus.h>
#include "PrefsModule.h"
#include "WebSocketsModule.h"
#include "NetworkModule.h"
#include "PowerModule.h"

int currentScreen;              // 1-Tally, 2-Power, 3-Setup
int currentBrightness = 11;     // default 11, max 12

const int tft_width = 240;
const int tft_heigth = 135;

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
    tallyScreen.createSprite(tft_width, tft_heigth);
    tallyScreen.setRotation(3);
    
}


void showPowerScreen() {

    clearScreen();
    powerScreen.createSprite(tft_width, tft_heigth);
    powerScreen.setRotation(3);

}


void showSetupScreen() {
    
    if (!wm.getWebPortalActive()) wm.startWebPortal();
    clearScreen();
    setupScreen.createSprite(tft_width, tft_heigth);
    setupScreen.setRotation(3);

}


void refreshTallyScreen() {

    RTC_TimeTypeDef time;
    M5.Rtc.GetTime(&time);
    
    char timeStr[14];
    const char* period = (time.Hours >= 12) ? "PM" : "AM";
    const int hours12 = (time.Hours > 12) ? time.Hours - 12 : time.Hours;
    sprintf(timeStr, "%02d:%02d:%02d %s", hours12, time.Minutes, time.Seconds, period);
    
    bool isProgram = false;
    bool isPreview = false;
    
    // Bitshift on inputIds. Rightmost bit = input 1
    for (int i = 1; i <= 16; i++) {
        bool bitValue = (inputIds >> i-1) & 0x01;
        if ((bitValue) & (i == atem_pgm1_input_id)) isProgram = true;
        if ((bitValue) & (i == atem_pvw1_input_id)) isPreview = true;
    }

    if (isProgram) {
        tallyScreen.fillRect(0,0,240,135, TFT_RED);
    } else if (isPreview) {
        tallyScreen.fillRect(0,0,240,135, TFT_GREEN);
    } else {
        tallyScreen.fillRect(0,0,240,135, TFT_BLACK);
    }
    
    tallyScreen.setTextSize(2);
    tallyScreen.setCursor((tft_width/2)-20, 8);
    tallyScreen.setTextColor(TFT_WHITE, TFT_BLACK);
    tallyScreen.print(timeStr);
    tallyScreen.setTextSize(9);
    tallyScreen.setCursor(10,80);
    tallyScreen.setTextColor(TFT_WHITE);
    tallyScreen.print(friendlyName);
    tallyScreen.pushSprite(0,0);
    
}


void refreshPowerScreen() {

    powerScreen.fillSprite(TFT_BLACK);
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
    
    setupScreen.fillSprite(TFT_BLACK);
    setupScreen.setTextColor(TFT_WHITE);
    setupScreen.setCursor(0,0);
    setupScreen.setTextSize(2);
    setupScreen.println(F("Setup Screen"));
    setupScreen.println();
    setupScreen.setTextSize(1);
    setupScreen.println("SSID: " + String(wm.getWiFiSSID()) + " " + String(WiFi.RSSI()));
    setupScreen.println("Webportal Active: " + String(wm.getWebPortalActive()));
    setupScreen.println("Hostname: " + wm.getWiFiHostname());
    setupScreen.println("IP: " + WiFi.localIP().toString());
    setupScreen.println("NTP: " + String(time_isSet));
    setupScreen.println();
    setupScreen.println("Node-RED Server: " + String(nodeRED_ServerIP) + ":" + String(nodeRED_ServerPort));
    setupScreen.println("Connected: " + String(ws_isConnected));
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


void setBrightness(int newBrightness = 0) {
    if (newBrightness >= 7 & newBrightness <= pwr.maxBrightness) {
        currentBrightness = newBrightness;
    } else {
        currentBrightness++;
        if (currentBrightness > pwr.maxBrightness) currentBrightness = 7;
    }
    M5.Axp.ScreenBreath(currentBrightness);
}