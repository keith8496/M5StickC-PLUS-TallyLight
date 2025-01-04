struct power {
    char batWarningLevel[17] = "";
    char powerMode[17] = "";
    float coulomb_count = 0;
    float batVoltage = 0;
    //float batVoltageMin = 0;
    //float batVoltageMax = 0;
    float batPercentage = 0;
    float batPercentageMin = 0;
    float batPercentageMax = 0;
    float batCurrent = 0;
    float batChargeCurrent = 0;
    float vbusVoltage = 0;
    float vbusCurrent = 0;
    float vinVoltage = 0;
    float vinCurrent = 0;
    float apsVoltage = 0;
    float tempInAXP192 = 0;
    int chargeCurrent = 100;
    int maxBrightness = 100;
};

extern power pwr;

void power_setup();
void power_onLoop();
//void set_chargeToPowerOff(unsigned int delay);