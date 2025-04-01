#include <M5StickCPlus.h>
#include <millisDelay.h>
#include "RunningAverage.h"
#include "PowerModule.h"
#include "ScreenModule.h"
#include "PrefsModule.h"


power pwr;

static const int md_power_milliseconds = 100;
static millisDelay md_power;
//static millisDelay md_chargeControlWait;
static const int runningAvgCnt = 200;
static RunningAverage ravg_batVoltage(runningAvgCnt);

static const int chargeControlSteps = 9;
static const uint8_t chargeControlArray[chargeControlSteps] = {0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8};
static const int chargeCurrentArray[chargeControlSteps] = {100, 190, 280, 360, 450, 550, 630, 700, 780};


// Define Functions
void power_onLoop();
void doPowerManagement();


// Function to estimate battery percentage with a non-linear discharge curve
float getBatteryPercentage(float voltage) {
    // Voltage-capacity segments for a single-cell LiPo battery
    const int numLevels = 21;
    const float batLookup[numLevels][2] = {
      {4.20, 100.0}, 
      {4.15, 95.0}, 
      {4.11, 90.0}, 
      {4.08, 85.0}, 
      {4.02, 80.0}, 
      {3.98, 75.0}, 
      {3.95, 70.0}, 
      {3.91, 65.0}, 
      {3.87, 60.0}, 
      {3.85, 55.0}, 
      {3.84, 50.0}, 
      {3.82, 45.0}, 
      {3.80, 40.0}, 
      {3.79, 35.0}, 
      {3.77, 30.0}, 
      {3.75, 25.0}, 
      {3.73, 20.0}, 
      {3.71, 15.0}, 
      {3.69, 10.0}, 
      {3.61, 5.0}, 
      {3.27, 0.0}
    };

    // Check for out-of-range values
    if (voltage >= batLookup[0][0]) {
        return 100.0; // Fully charged
    } else if (voltage <= batLookup[numLevels - 1][0]) {
        return 0.0;   // Fully discharged
    }

    // Interpolate within the appropriate segment
    for (int i = 0; i < numLevels - 1; i++) {
        if (voltage <= batLookup[i][0] && voltage > batLookup[i + 1][0]) {
            // Linear interpolation between the two points
            float percentage = batLookup[i][1] +
                               (voltage - batLookup[i][0]) * 
                               (batLookup[i + 1][1] - batLookup[i][1]) /
                               (batLookup[i + 1][0] - batLookup[i][0]);
            return percentage;
        }
    }

    // Default return (should not reach here)
    return 0.0;
}

float getBatPercentageCoulomb() {
  const float bat = (batteryCapacity + pwr.coulombCount) / batteryCapacity * 100;
  if (bat > 100.0) {
    return 100.0;
  } else if (bat < 0.0) {
    return 0.0;
  } else {
    return bat;
  }
}


int getChargeCurrent() {
  const uint8_t chargeControlNow = M5.Axp.Read8bit(0x33);
  for (int i = 0; i < chargeControlSteps; i++) {
    if (chargeControlNow == chargeControlArray[i]) {
      return chargeCurrentArray[i];
    }
  }
  return -1; // should never get here
}


/*void setChargeCurrent(int reqChargeCurrent) {
  
  if (reqChargeCurrent == 100) {
    md_chargeControlWait.stop();
  }
  if (reqChargeCurrent == getChargeCurrent()) {
    return;
  }
  if (md_chargeControlWait.isRunning()) {
    return;
  }
  
  uint8_t reqChargeControl = 0xc0;
  for (int i = 0; i < chargeControlSteps; i++) {
    if (reqChargeCurrent == chargeCurrentArray[i]) {
      reqChargeControl = chargeControlArray[i];
      break;
    }
  }
  
  M5.Axp.Write1Byte(0x33, reqChargeControl);
  pwr.maxChargeCurrent = getChargeCurrent();
  md_chargeControlWait.start(20000);

}*/


void power_setup() {
  M5.Axp.EnableCoulombcounter();
  doPowerManagement();
  ravg_batVoltage.fillValue(M5.Axp.GetBatVoltage(),runningAvgCnt);
  md_power.start(md_power_milliseconds);
}


void power_onLoop() {
  if (md_power.justFinished()) {
      md_power.repeat();
      doPowerManagement();
  }
}


void doPowerManagement() {

  const int md_chargeToOff_milliseconds = 60000;
  static millisDelay md_chargeToOff;
  static millisDelay md_lowBattery;

  const bool isBatWarningLevel = M5.Axp.GetWarningLevel();
  if (isBatWarningLevel) {
    strcpy(pwr.batWarningLevel, "LOW BATTERY");
  } else {
    strcpy(pwr.batWarningLevel, "");
  }
  
  const float batVoltage = M5.Axp.GetBatVoltage();
  ravg_batVoltage.addValue(batVoltage);
  pwr.batVoltage = batVoltage;
  
  pwr.batPercentage = getBatteryPercentage(ravg_batVoltage.getAverage()); //ravg_batVoltage.getAverageLast(50)
  pwr.batPercentageMin = getBatteryPercentage(ravg_batVoltage.getMinInBuffer());
  pwr.batPercentageMax = getBatteryPercentage(ravg_batVoltage.getMaxInBuffer());
  pwr.batCurrent = M5.Axp.GetBatCurrent();
  pwr.batChargeCurrent = M5.Axp.GetBatChargeCurrent();
  pwr.maxChargeCurrent = getChargeCurrent();
  pwr.vbusVoltage = M5.Axp.GetVBusVoltage();
  pwr.vbusCurrent = M5.Axp.GetVBusCurrent();
  pwr.vinVoltage = M5.Axp.GetVinVoltage();
  pwr.vinCurrent = M5.Axp.GetVinCurrent();
  pwr.apsVoltage = M5.Axp.GetAPSVoltage();
  pwr.tempInAXP192 = M5.Axp.GetTempInAXP192();
  pwr.coulombCount = M5.Axp.GetCoulombData();
  pwr.batPercentageCoulomb = getBatPercentageCoulomb();


  // Power Mode
  if (pwr.vinVoltage > 3.8) {         // 5v IN Charge
    
    pwr.maxBrightness = 100;
    
    /*if (pwr.batPercentageMax < 70 && pwr.maxChargeCurrent < 780) {
      setChargeCurrent(780);
    } else if (pwr.batPercentageMax < 75 && pwr.maxChargeCurrent < 700) {
      setChargeCurrent(700);
    } else if (pwr.batPercentageMax < 80 && pwr.maxChargeCurrent < 630) {
      setChargeCurrent(630);
    } else if (pwr.batPercentageMax < 85 && pwr.maxChargeCurrent < 550) {
      setChargeCurrent(550);
    } else if (pwr.batPercentageMax < 90 && pwr.maxChargeCurrent < 450) {
      setChargeCurrent(450);
    } else if (pwr.batPercentageMax < 95 && pwr.maxChargeCurrent < 360) {
      setChargeCurrent(360);
    } else if (pwr.maxChargeCurrent != 280) {
      setChargeCurrent(280);
    }*/

    if (pwr.maxChargeCurrent != 780) {
      M5.Axp.Write1Byte(0x33, 0xc8);
    }

    
    if (currentBrightness > 20) {

      strcpy(pwr.powerMode, "5v Charge");
      md_chargeToOff.stop();

    } else {
                 
      // Charge to Off
      
      if (md_chargeToOff.justFinished()) {
        M5.Axp.ClearCoulombcounter();
        M5.Axp.PowerOff();
      }

      if (!md_chargeToOff.isRunning() && pwr.batVoltage >= 3.99 && floor(pwr.batChargeCurrent) == 0) {
        md_chargeToOff.start(md_chargeToOff_milliseconds);
      }
      
      if (md_chargeToOff.isRunning()) {
        char powerMode[20];
        const int md_chargeToOffRemaining = floor(md_chargeToOff.remaining() / 1000);
        snprintf(powerMode, 20, "Charge to Off (%i)", md_chargeToOffRemaining);
        strcpy(pwr.powerMode, powerMode);
      } else {
        strcpy(pwr.powerMode, "Charge to Off");
      }

    }

  } else if (pwr.vbusVoltage > 3.8) {   // 5v USB Charge

    md_chargeToOff.stop();
    strcpy(pwr.powerMode, "USB Charge");
    pwr.maxBrightness = 100;
    
    if (pwr.maxChargeCurrent != 100) {
      M5.Axp.Write1Byte(0x33, 0xc0);
    }

  } else {                              // 3v Battery

    md_chargeToOff.stop();
    if (pwr.maxChargeCurrent != 100) {
      M5.Axp.Write1Byte(0x33, 0xc0);
    }

    if (isBatWarningLevel) {
      strcpy(pwr.powerMode, "Low Battery");
      pwr.maxBrightness = pmPowerSaverBright;
      if (md_lowBattery.justFinished()) {
        md_lowBattery.repeat();
        batteryCapacity = pwr.coulombCount*(-1);
        preferences_save();
      } else if (!md_lowBattery.isRunning()) {
        md_lowBattery.start(60000);
      }
    } else if (floor(pwr.batPercentageMin) <= pmPowerSaverBatt) {
      strcpy(pwr.powerMode, "Power Saver");
      pwr.maxBrightness = pmPowerSaverBright;
      if (currentBrightness > pwr.maxBrightness) setBrightness(pwr.maxBrightness);
    } else {
      strcpy(pwr.powerMode, "Balanced");
      pwr.maxBrightness = 100;
    }

  }

}