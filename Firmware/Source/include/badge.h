#ifndef _BADGE_H_
#define _BADGE_H_
#include <Arduino.h>

#define MB1LED 1
#define MB2LED 4
#define MB3LED 5
#define MB4LED 8

#define SSID_ADDRESS        0
#define PASSWORD_ADDRESS    32


struct badge_info {
    int pclass;
    int character;
    String handle;
    String email;
    int haircolor;
    int eyecolor;
    bool trained;
    bool isStaff;
    int level;
};

extern badge_info g_badgeinfo;
extern String badge_id;
extern uint8_t badgeSerialNumbers [4][16];
extern String badgeLabels [4];
extern volatile bool minibadgesChanged;

void LoadBadgeId();
bool ScanMinibadges();
String SerialString(uint8_t bytes[]);
void updateMinibadgeLEDs();

bool WifiConnect(String ssid, String password);
bool WifiEnable();
bool WifiDisable();
bool ConfigWifi();
#endif
