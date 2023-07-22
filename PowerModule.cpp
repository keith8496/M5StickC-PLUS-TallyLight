#include <M5StickCPlus.h>
#include <millisDelay.h>
#include "RunningAverage.h"
#include "PowerModule.h"
#include "ScreenModule.h"
#include "PrefsModule.h"


power pwr;
millisDelay md_power;
millisDelay md_powerOneMin;
millisDelay md_chargeToOff;
RunningAverage ravg_batVoltage(60);
RunningAverage ravg_batPercentage(60);
RunningAverage ravg_batCurrent(60);
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
    ravg_batVoltage.fillValue(ravg_batVoltage.getValue(0),60);
    ravg_batPercentage.fillValue(ravg_batPercentage.getValue(0),60);
    ravg_batCurrent.fillValue(ravg_batCurrent.getValue(0),60);
    md_power.start(1000);
    md_powerOneMin.start(60000);
}


void power_onLoop() {
    if (md_powerOneMin.justFinished()) {
        md_powerOneMin.repeat();
        ravg_batVoltage.getAverage();
        ravg_batPercentage.getAverage();
        ravg_batCurrent.getAverage();
    }
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
  
  const float batVoltageNow = M5.Axp.GetBatVoltage();
  ravg_batVoltage.addValue(batVoltageNow);
  pwr.batVoltage = ravg_batVoltage.getFastAverage();

  pwr.coulomb_count = M5.Axp.GetCoulombData();
  if (pwr.coulomb_count > 0) {
      coulomb_adjust = coulomb_adjust + pwr.coulomb_count;
      Serial.printf("\n\rCoulomb Count: %.4f", pwr.coulomb_count);
      Serial.printf("\n\rCoulomb Adjust: %.4f", coulomb_adjust);
      pwr.coulomb_count = 0;
      M5.Axp.ClearCoulombcounter();
  }

  //const float batPercentageNow = (batteryCapacity + pwr.coulomb_count) / batteryCapacity * 100;
  if (pwr.batVoltage >= 4.2) {
    ravg_batPercentage.addValue(100.0);
  } else {
    const int batVoltageInt = batVoltageNow * 100;
    for (int i = 0; i < batLookupElements; i++ ) {
      if (batLookup[i][0] <= batVoltageInt) {
        int batPercentageInt = map(batVoltageInt, batLookup[i-1][0], batLookup[i][0], batLookup[i-1][1], batLookup[i][1]);
        ravg_batPercentage.addValue(batPercentageInt);
        break;
      }
    }
  }
    
  pwr.batPercentage = ravg_batPercentage.getFastAverage();
  pwr.batPercentageMin = ravg_batPercentage.getMinInBuffer();
  ravg_batCurrent.addValue(M5.Axp.GetBatCurrent());
  pwr.batCurrent = ravg_batCurrent.getFastAverage();
  pwr.batChargeCurrent = M5.Axp.GetBatChargeCurrent();
  pwr.vbusVoltage = M5.Axp.GetVBusVoltage();
  pwr.vbusCurrent = M5.Axp.GetVBusCurrent();
  pwr.vinVoltage = M5.Axp.GetVinVoltage();
  pwr.vinCurrent = M5.Axp.GetVinCurrent();
  pwr.apsVoltage = M5.Axp.GetAPSVoltage();
  pwr.tempInAXP192 = M5.Axp.GetTempInAXP192();


  // Power Mode
  if (pwr.vinVoltage > 3.8) {         // 5v IN Charge
    
    strcpy(pwr.powerMode, "5v Charge");
    pwr.maxBrightness = 12;
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);  //WIFI_PS_NONE

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

    // Charge to Off
    if (md_chargeToOff.justFinished()) {
      if (ravg_batVoltage.getMaxInBuffer() >= (4.2*0.95) && pwr.batChargeCurrent == 0) {
        // 100% battery after 15 minutes => PowerOff
        M5.Axp.PowerOff();
      } else {
        // Set 1 minute timer and wait for 100% battery
        md_chargeToOff.start(60000);
      }
    }
    if (md_chargeToOff.isRunning() && md_chargeToOff.remaining() <= 600000) {
      // We have been on the power screen for >= 5 minutes (900000 - 300000 = 600000)
      strcpy(pwr.powerMode, "Charge-to-Off");
    }

  } else if (pwr.vbusVoltage > 3.8) {   // 5v USB Charge

    strcpy(pwr.powerMode, "USB Charge");
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

    if (pwr.batPercentageMin >= 70) {
      strcpy(pwr.powerMode, "Balanced");
      pwr.maxBrightness = 12;
      //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    } else if (strcmp(pwr.batWarningLevel, "LOW BATTERY") == 0) {
      strcpy(pwr.powerMode, "Low Battery");
    } else {
      strcpy(pwr.powerMode, "Power Saver");
      pwr.maxBrightness = 9;
      if (currentBrightness > pwr.maxBrightness) setBrightness(pwr.maxBrightness);
      //esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    }

  }

}


void set_chargeToPowerOff(unsigned int delay) {
  if (delay == 0) {
    md_chargeToOff.stop();
  } else {
    md_chargeToOff.start(delay);
  }

}