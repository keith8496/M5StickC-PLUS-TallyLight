extern char friendlyName[17];
extern uint16_t inputIds;
extern char nodeRED_ServerIP[16];
extern int nodeRED_ServerPort;
extern char nodeRED_ServerUrl[33];
extern long gmtOffset_sec;
extern int daylightOffset_sec;
extern char ntpServer[33];

extern char deviceId[9];
extern char deviceName[33];

void preferences_setup();
void preferences_save();