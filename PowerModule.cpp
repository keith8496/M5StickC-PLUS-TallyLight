#include <M5StickCPlus.h>
#include <millisDelay.h>
#include "RunningAverage.h"
#include "PowerModule.h"
#include "ScreenModule.h"

#define Vmax 4200
#define Vmin 3000


power pwr;
millisDelay md_power;
millisDelay md_powerOneMin;
millisDelay md_chargeToOff;
RunningAverage ravg_batVoltage(60);
RunningAverage ravg_batPercentage(60);
RunningAverage ravg_batCurrent(60);


// Define Functions
void power_onLoop();
void doPowerManagement();


// from BatterySence https://github.com/rlogiacco/BatterySense
/**
 * Symmetric sigmoidal approximation
 * https://www.desmos.com/calculator/7m9lu26vpy
 *
 * c - c / (1 + k*x/v)^3
 */
static inline uint8_t sigmoidal(uint16_t voltage, uint16_t minVoltage, uint16_t maxVoltage) {
	// slow
	// uint8_t result = 110 - (110 / (1 + pow(1.468 * (voltage - minVoltage)/(maxVoltage - minVoltage), 6)));

	// steep
	// uint8_t result = 102 - (102 / (1 + pow(1.621 * (voltage - minVoltage)/(maxVoltage - minVoltage), 8.1)));

	// normal
	uint8_t result = 105 - (105 / (1 + pow(1.724 * (voltage - minVoltage)/(maxVoltage - minVoltage), 5.5)));
	return result >= 100 ? 100 : result;
}


void power_setup() {
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

  const float batPercentageNow = (float)sigmoidal(batVoltageNow*1000, Vmin, Vmax);
  ravg_batPercentage.addValue(batPercentageNow);
  pwr.batPercentage = ravg_batPercentage.getFastAverage();
  pwr.batPercentage_M = ravg_batPercentage.getMax();

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

    // Charge to Off
    if (md_chargeToOff.justFinished()) {
      if (pwr.batPercentage_M >= 100) {
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