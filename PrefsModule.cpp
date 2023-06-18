#include <Preferences.h>
#include <WiFiManager.h>

Preferences preferences;

char friendlyName[17];
uint16_t inputIds = 0b0000000000000001;
char nodeRED_ServerIP[16];
int nodeRED_ServerPort;
char nodeRED_ServerUrl[33];
char localTimeZone[17];
char ntpServer[33];


void preferences_setup() {
    preferences.begin("custom", true);
    strcpy(friendlyName, preferences.getString("friendlyName", "CamX").c_str());
    if (preferences.getBytesLength("inputIds") > 0) preferences.getBytes("inputIds", &inputIds, 2);
    strcpy(nodeRED_ServerIP, preferences.getString("nr_ServerIP", "192.168.13.54").c_str());
    nodeRED_ServerPort = preferences.getInt("nr_ServerPort", 1880);
    strcpy(nodeRED_ServerUrl, preferences.getString("nr_ServerUrl", "/ws/tally").c_str());
    strcpy(localTimeZone, preferences.getString("localTimeZone", "America/Chicago").c_str());
    strcpy(ntpServer, preferences.getString("ntpServer", "time.apple.com").c_str());
    preferences.end();
}


void preferences_save() {
    preferences.begin("custom", false);
    preferences.putString("friendlyName", friendlyName);
    preferences.putBytes("inputIds", &inputIds, 2);
    preferences.putString("nr_ServerIP", nodeRED_ServerIP);
    preferences.putInt("nr_ServerPort", nodeRED_ServerPort);
    preferences.putString("nr_ServerUrl", nodeRED_ServerUrl);
    preferences.putString("ntpServer", ntpServer);
    preferences.putString("localTimeZone", localTimeZone);
    preferences.end();
}