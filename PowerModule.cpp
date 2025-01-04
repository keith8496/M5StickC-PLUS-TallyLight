#include <M5StickCPlus.h>
#include <millisDelay.h>
#include "RunningAverage.h"
#include "PowerModule.h"
#include "ScreenModule.h"
#include "PrefsModule.h"


power pwr;
millisDelay md_power;
millisDelay md_chargeToOff;
const int runningAvgCnt = 600;
RunningAverage ravg_batVoltage(runningAvgCnt);
float coulomb_adjust = 0;


// Function to estimate battery percentage with a non-linear discharge curve
float getBatteryPercentage(float voltage) {
    // Voltage-capacity segments for a single-cell LiPo battery
    const float voltageLevels[] = {4.2, 4.0, 3.85, 3.7, 3.2};
    const float percentageLevels[] = {100.0, 75.0, 50.0, 25.0, 0.0};
    const int numLevels = sizeof(voltageLevels) / sizeof(voltageLevels[0]);

    // Check for out-of-range values
    if (voltage >= voltageLevels[0]) {
        return 100.0; // Fully charged
    } else if (voltage <= voltageLevels[numLevels - 1]) {
        return 0.0;   // Fully discharged
    }

    // Interpolate within the appropriate segment
    for (int i = 0; i < numLevels - 1; i++) {
        if (voltage <= voltageLevels[i] && voltage > voltageLevels[i + 1]) {
            // Linear interpolation between the two points
            float percentage = percentageLevels[i] +
                               (voltage - voltageLevels[i]) * 
                               (percentageLevels[i + 1] - percentageLevels[i]) /
                               (voltageLevels[i + 1] - voltageLevels[i]);
            return percentage;
        }
    }

    // Default return (should not reach here)
    return 0.0;
}


// Define Functions
void power_onLoop();
void doPowerManagement();


void power_setup() {
    M5.Axp.EnableCoulombcounter();
    doPowerManagement();
    ravg_batVoltage.fillValue(M5.Axp.GetBatVoltage(),runningAvgCnt);
    md_power.start(100);
}


void power_onLoop() {
    if (md_power.justFinished()) {
        md_power.repeat();
        doPowerManagement();
    }
}


void doPowerManagement() {

  if (currentScreen != 2 && md_chargeToOff.isRunning()) md_chargeToOff.stop();
  
  if (M5.Axp.GetWarningLevel()) {
    strcpy(pwr.batWarningLevel, "LOW BATTERY");
  } else {
    strcpy(pwr.batWarningLevel, "");
  }  
  
  ravg_batVoltage.addValue(M5.Axp.GetBatVoltage());
  pwr.batVoltage = ravg_batVoltage.getAverageLast(10);
  
  pwr.batPercentage = getBatteryPercentage(pwr.batVoltage);
  pwr.batPercentageMin = getBatteryPercentage(ravg_batVoltage.getMinInBuffer());
  pwr.batPercentageMax = getBatteryPercentage(ravg_batVoltage.getMaxInBuffer());
  pwr.batCurrent = M5.Axp.GetBatCurrent();
  pwr.batChargeCurrent = M5.Axp.GetBatChargeCurrent();
  pwr.vbusVoltage = M5.Axp.GetVBusVoltage();
  pwr.vbusCurrent = M5.Axp.GetVBusCurrent();
  pwr.vinVoltage = M5.Axp.GetVinVoltage();
  pwr.vinCurrent = M5.Axp.GetVinCurrent();
  pwr.apsVoltage = M5.Axp.GetAPSVoltage();
  pwr.tempInAXP192 = M5.Axp.GetTempInAXP192();
  pwr.coulombCount = M5.Axp.GetCoulombData();
  pwr.batPercentageCoulomb = (2200 + pwr.coulombCount) / 2200 * 100;


  // Power Mode
  if (pwr.vinVoltage > 3.8) {         // 5v IN Charge
    
    pwr.maxBrightness = 100;
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);  //WIFI_PS_NONE
    
    if (currentBrightness > 20) {

      strcpy(pwr.powerMode, "5v Charge");
      md_chargeToOff.stop();

      if ((pwr.batPercentageMin < 75) && (pwr.chargeCurrent != 780)) {
        pwr.chargeCurrent = 780;
        M5.Axp.Write1Byte(0x33, 0xc8);
      } else if (pwr.batPercentageMin >= 75 && pwr.batPercentageMin < 90 && pwr.chargeCurrent != 550) {
        pwr.chargeCurrent = 550;
        M5.Axp.Write1Byte(0x33, 0xc5);
      } else if (pwr.batPercentageMin >= 90 && pwr.batPercentageMin < 100 && pwr.chargeCurrent != 280) {
        pwr.chargeCurrent = 280;
        M5.Axp.Write1Byte(0x33, 0xc2);
      } /*else if (pwr.batPercentage >= 90 && pwr.chargeCurrent != 190) {
        pwr.chargeCurrent = 190;
        M5.Axp.Write1Byte(0x33, 0xc1);
      }*/

    } else {

      // Charge to Off
      if (md_chargeToOff.justFinished()) {
        M5.Axp.ClearCoulombcounter();
        M5.Axp.PowerOff();
      }
      
      if (pwr.batVoltage >= 3.99 && floor(pwr.batChargeCurrent) == 0 && !md_chargeToOff.isRunning()) {
        md_chargeToOff.start(60000);
      }

      if (md_chargeToOff.isRunning()) {
        strcpy(pwr.powerMode, "Charge-to-Off <60");
      } else {
        strcpy(pwr.powerMode, "Charge-to-Off");
      }
      
      if (pwr.chargeCurrent != 280) {
        pwr.chargeCurrent = 280;
        M5.Axp.Write1Byte(0x33, 0xc2);
      }     

    }

  } else if (pwr.vbusVoltage > 3.8) {   // 5v USB Charge

    strcpy(pwr.powerMode, "USB Charge");
    md_chargeToOff.stop();
    pwr.maxBrightness = 100;
    //esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    
    if (pwr.chargeCurrent != 100) {
      pwr.chargeCurrent = 100;
      M5.Axp.Write1Byte(0x33, 0xc0);
    }

  } else {
    
    // 3v Battery

    md_chargeToOff.stop();
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

}