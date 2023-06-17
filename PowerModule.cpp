#include <M5StickCPlus.h>
#include <millisDelay.h>
#include "ScreenModule.h"
#include "PowerModule.h"

millisDelay md_power;

power pwr;
const int batPercentage_C = 60;
int batPercentage_I = 0;
float batPercentage_A [batPercentage_C];


// Define Functions
void power_onLoop();
void doPowerManagement();


void power_setup() {
    doPowerManagement();
    md_power.start(1000);
}


void power_onLoop() {
    if (md_power.justFinished()) {
        doPowerManagement();
        md_power.repeat();
    }
}


void doPowerManagement() {
  
  if (M5.Axp.GetWarningLevel()) {
    strcpy(pwr.batWarningLevel, "LOW BATTERY");
  } else {
    strcpy(pwr.batWarningLevel, "");
  }
  
  pwr.batVoltage = M5.Axp.GetBatVoltage();

  float batPercentage1 = (pwr.batVoltage - 3.0) / (4.2-3.0) * 100; // min = 3.0 max = 4.2
  float batPercentage2 = (batPercentage1 <= 100) ? batPercentage1 : 100;
  pwr.batPercentage = batPercentage2;

  if (batPercentage_A[batPercentage_I] == pwr.batPercentage_M) {
    pwr.batPercentage_M = 0;
  }

  batPercentage_A[batPercentage_I] = batPercentage2;
  batPercentage_I = batPercentage_I + 1;
  if (batPercentage_I > batPercentage_C) {
    batPercentage_I = 0;
  }

  if (pwr.batPercentage > pwr.batPercentage_M) {
    for (int i = 0; i <= batPercentage_C; ++i) {
      if (batPercentage_A[i] > pwr.batPercentage_M) {
        pwr.batPercentage_M = batPercentage_A[i];
      }
    }
  }

  pwr.batCurrent = M5.Axp.GetBatCurrent();
  pwr.batChargeCurrent = M5.Axp.GetBatChargeCurrent();
  pwr.vbusVoltage = M5.Axp.GetVBusVoltage();
  pwr.vbusCurrent = M5.Axp.GetVBusCurrent();
  pwr.vinVoltage = M5.Axp.GetVinVoltage();
  pwr.vinCurrent = M5.Axp.GetVinCurrent();
  pwr.apsVoltage = M5.Axp.GetAPSVoltage();
  pwr.tempInAXP192 = M5.Axp.GetTempInAXP192();


  // Power Mode
  if (pwr.vinVoltage > 3.8) {         // 5v IN Charge
    
    pwr.maxBrightness = 12;
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);  //WIFI_PS_NONE

    if ((pwr.batPercentage_M < 80) && (pwr.chargeCurrent != 780)) {
      strcpy(pwr.powerMode, "Fast Charge");
      pwr.chargeCurrent = 780;
      M5.Axp.Write1Byte(0x33, 0xc8);
    } else if (pwr.batPercentage_M >= 80 && pwr.batPercentage_M < 90 && pwr.chargeCurrent != 280) {
      strcpy(pwr.powerMode, "Performance");
      pwr.chargeCurrent = 280;
      M5.Axp.Write1Byte(0x33, 0xc2);
    } else if (pwr.batPercentage_M >= 90 && pwr.chargeCurrent != 190) {
      strcpy(pwr.powerMode, "Performance");
      pwr.chargeCurrent = 190;
      M5.Axp.Write1Byte(0x33, 0xc1);
    }

  } else if (pwr.vbusVoltage > 3.8) {   // 5v USB Charge

    strcpy(pwr.powerMode, "Performance");
    pwr.maxBrightness = 12;
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    
    if (pwr.chargeCurrent != 100) {
      pwr.chargeCurrent = 100;
      M5.Axp.Write1Byte(0x33, 0xc0);
    }

  } else {
    
    // 3v Battery

    if (pwr.chargeCurrent != 100) {
      pwr.chargeCurrent = 100;
      M5.Axp.Write1Byte(0x33, 0xc0);
    }

    if (pwr.batPercentage_M >= 80) {
      strcpy(pwr.powerMode, "Balanced");
      pwr.maxBrightness = 12;
      //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    } else {
      strcpy(pwr.powerMode, "Power Saver");
      pwr.maxBrightness = 9;
      if (currentBrightness > pwr.maxBrightness) {
        currentBrightness = pwr.maxBrightness;
        M5.Axp.ScreenBreath(currentBrightness);
      }
      //esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    }

  }

}