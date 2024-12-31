#include <M5StickCPlus.h>
#include <millisDelay.h>
#include "RunningAverage.h"
#include "PowerModule.h"
#include "ScreenModule.h"
#include "PrefsModule.h"


power pwr;
millisDelay md_power;
millisDelay md_chargeToOff;
const int runningAvgCnt = 60;
RunningAverage ravg_batVoltage(runningAvgCnt);
float coulomb_adjust = 0;


const int batLookupElements = 21;
const int batLookup[batLookupElements][2] = {
  {420, 100}, 
  {415, 95}, 
  {411, 90}, 
  {408, 85}, 
  {402, 80}, 
  {398, 75}, 
  {395, 70}, 
  {391, 65}, 
  {387, 60}, 
  {385, 55}, 
  {384, 50}, 
  {382, 45}, 
  {380, 40}, 
  {379, 35}, 
  {377, 30}, 
  {375, 25}, 
  {373, 20}, 
  {371, 15}, 
  {369, 10}, 
  {361, 5}, 
  {327, 0}
};


// Define Functions
void power_onLoop();
void doPowerManagement();


void power_setup() {
    M5.Axp.EnableCoulombcounter();
    doPowerManagement();
    ravg_batVoltage.fillValue(M5.Axp.GetBatVoltage(),runningAvgCnt);
    md_power.start(1000);
    currentBrightness = 50;
    setBrightness(currentBrightness);
}


void power_onLoop() {
    if (md_power.justFinished()) {
        md_power.repeat();
        doPowerManagement();
    }
}


void doPowerManagement() {

  if (M5.Axp.GetWarningLevel()) {
    strcpy(pwr.batWarningLevel, "LOW BATTERY");
  } else {
    strcpy(pwr.batWarningLevel, "");
  }

  pwr.coulomb_count = M5.Axp.GetCoulombData();
  if (pwr.coulomb_count > 0) {
      coulomb_adjust = coulomb_adjust + pwr.coulomb_count;
      Serial.printf("\n\rCoulomb Count: %.4f", pwr.coulomb_count);
      Serial.printf("\n\rCoulomb Adjust: %.4f", coulomb_adjust);
      pwr.coulomb_count = 0;
      M5.Axp.ClearCoulombcounter();
  }
  
  ravg_batVoltage.addValue(M5.Axp.GetBatVoltage());
  pwr.batVoltage = ravg_batVoltage.getAverage();
  pwr.batVoltageMin = ravg_batVoltage.getMinInBuffer();
  
  const int batVoltageInt = pwr.batVoltage * 100;
  const int batVoltageIntMin = pwr.batVoltageMin * 100;
  for (int i = 0; i < batLookupElements; i++ ) {
    if (batLookup[i][0] <= batVoltageInt) {
      pwr.batPercentage = map(batVoltageInt, batLookup[i-1][0], batLookup[i][0], batLookup[i-1][1], batLookup[i][1]);
      pwr.batPercentageMin = map(batVoltageIntMin, batLookup[i-1][0], batLookup[i][0], batLookup[i-1][1], batLookup[i][1]);
      break;
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
    
    pwr.maxBrightness = 100;
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);  //WIFI_PS_NONE
    
    if (currentBrightness > 20) {

      strcpy(pwr.powerMode, "5v Charge");
      if (md_chargeToOff.isRunning()) md_chargeToOff.stop();

      if ((pwr.batPercentageMin < 70) && (pwr.chargeCurrent != 780)) {
        pwr.chargeCurrent = 780;
        M5.Axp.Write1Byte(0x33, 0xc8);
      } else if (pwr.batPercentageMin >= 70 && pwr.batPercentageMin < 80 && pwr.chargeCurrent != 550) {
        pwr.chargeCurrent = 550;
        M5.Axp.Write1Byte(0x33, 0xc5);
      } else if (pwr.batPercentageMin >= 80 && pwr.batPercentageMin < 90 && pwr.chargeCurrent != 280) {
        pwr.chargeCurrent = 280;
        M5.Axp.Write1Byte(0x33, 0xc2);
      } else if (pwr.batPercentageMin >= 90 && pwr.chargeCurrent != 190) {
        pwr.chargeCurrent = 190;
        M5.Axp.Write1Byte(0x33, 0xc1);
      }

    } else {

      strcpy(pwr.powerMode, "Charge-to-Off");
      if (!md_chargeToOff.isRunning()) md_chargeToOff.start(60000);
      if (pwr.chargeCurrent != 190) {
        pwr.chargeCurrent = 190;
        M5.Axp.Write1Byte(0x33, 0xc1);
      }

    }

    // Charge to Off
    if (md_chargeToOff.justFinished() && pwr.powerMode == "Charge-to-Off") {
      if (ravg_batVoltage.getMaxInBuffer() >= (4.2*0.95) && pwr.batChargeCurrent == 0) {
        // 100% battery after 15 minutes => PowerOff
        M5.Axp.PowerOff();
      } else {
        // Set 1 minute timer and wait for 100% battery
        md_chargeToOff.start(60000);
      }
    }

  } else if (pwr.vbusVoltage > 3.8) {   // 5v USB Charge

    strcpy(pwr.powerMode, "USB Charge");
    pwr.maxBrightness = 100;
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    
    if (md_chargeToOff.isRunning()) md_chargeToOff.stop();
    if (pwr.chargeCurrent != 100) {
      pwr.chargeCurrent = 100;
      M5.Axp.Write1Byte(0x33, 0xc0);
    }

  } else {
    
    // 3v Battery

    if (md_chargeToOff.isRunning()) md_chargeToOff.stop();
    if (pwr.chargeCurrent != 100) {
      pwr.chargeCurrent = 100;
      M5.Axp.Write1Byte(0x33, 0xc0);
    }

    if (pwr.batPercentageMin >= pmPowerSaverBatt) {
      strcpy(pwr.powerMode, "Balanced");
      pwr.maxBrightness = 100;
      //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    } else if (strcmp(pwr.batWarningLevel, "LOW BATTERY") == 0) {
      strcpy(pwr.powerMode, "Low Battery");
      pwr.maxBrightness = pmPowerSaverBright;
      if (currentBrightness > pwr.maxBrightness) setBrightness(pwr.maxBrightness);
    } else {
      strcpy(pwr.powerMode, "Power Saver");
      pwr.maxBrightness = pmPowerSaverBright;
      if (currentBrightness > pwr.maxBrightness) setBrightness(pwr.maxBrightness);
      //esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    }

  }

  //Serial.printf("\n\rpwr.powerMode: %s", pwr.powerMode);
  //Serial.printf("\n\rpwr.maxBrightness: %d", pwr.maxBrightness);

}


/*void set_chargeToPowerOff(unsigned int delay) {
  if (delay == 0 && md_chargeToOff.isRunning()) {
    md_chargeToOff.stop();
  } else if (delay > 0 && !md_chargeToOff.isRunning()) {
    md_chargeToOff.start(delay);
  }

}*/