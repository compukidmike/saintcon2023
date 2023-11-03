#include "badge.h"
#include "utils.h"
#include <WiFi.h>
#include "Window.hpp"
#include <EEPROM.h>
#include "Menu.h"
#include "EntryScreen.h"

String badge_id;
badge_info g_badgeinfo = {0};


uint8_t badgeSerialNumbers [4][16];
String badgeLabels [4];

volatile bool minibadgesChanged = true;

const uint8_t minibadgeLEDNums[4] = {1,4,5,8};

const String simpleMinibadges[4] = {
    {"IDBadge "},
    {"Keyboard"},
    {"Glasses "},
    {"Polo    "}
};

const String minibadgeList[25] = {
    {"KlipperH"},
    {"Sword   "},
    {"CustKeeb"},
    {"SCHoodie"},
    {"SCCap   "},
    {"Pizza   "},
    {"Mustache"},
    {"MtnDew  "},
    {"Monster "},
    {"T-Rex   "},
    {"Coffee  "},
    {"Stress  "},
    {"1up     "},
    {"CatVideo"},
    {"Cheetos "},
    {"Taco    "},
    {"Phone   "},
    {"BlackHat"},
    {"WhiteHat"},
    {"Tinfoil "},
    {"Lockpick"},
    {"Solderin"},
    {"Candy   "},
    {"Hawaiian"},
    {"PepsiCan"},
};

const CRGB minibadgeColors[25] = {
    CRGB::Orange,
    CRGB::Blue,
    CRGB::Purple,
    CRGB::Blue,
    CRGB::Red,
    CRGB::Red,
    CRGB::Yellow,
    CRGB::Green,
    CRGB::Yellow,
    CRGB::Orange,
    CRGB::Yellow,
    CRGB::Red,
    CRGB::Green,
    CRGB::Pink,
    CRGB::Orange,
    CRGB::Yellow,
    CRGB::Blue,
    CRGB::Red,
    CRGB::Green,
    CRGB::Green,
    CRGB::Yellow,
    CRGB::Cyan,
    CRGB::Orange,
    CRGB::Purple,
    CRGB::Blue
};

void LoadBadgeId() {
    char baseMacChr[13] = {0};
    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    badge_id = String(baseMacChr);
}

String SerialString(uint8_t bytes[]) {
    const char* hex = "0123456789ABCDEF";
    String uuid;
    for (int i=0; i<16; ++i) {
        uuid += hex[bytes[i] >> 4];
        uuid += hex[bytes[i] & 0xF];
    }
    return uuid;
}


bool ScanMinibadges() {
    bool changed = false;
            
    for (int i=0; i<4; ++i) {
        uint8_t id[16] = {0};
        char label[9] = {0};
        bool has_sn = true;
        bool has_label = false;
        bool redactd = false;

        readMBEEPROM(i, (uint8_t*)label);

        for (int j=0; j<4; ++j) {
            if (memcmp(label, simpleMinibadges[j].c_str(), 8) == 0){
                has_sn = false;
            }
        }
        for (int j=0; j<25; ++j) {
            if (memcmp(label, minibadgeList[j].c_str(), 8) == 0){
                has_label = true;
            }
        }
        if(has_sn == true and has_label == false){
            readMBEEPROMIDspecial(i, id);
            int serialsum = 0;
            for(int x=0; x<16; x++){
                serialsum += id[x];
            }
            if(serialsum > 0){
                label[0] = 'r';
                label[1] = 'e';
                label[2] = 'd';
                label[3] = 'a';
                label[4] = 'c';
                label[5] = 't';
                label[6] = 'd';
                label[7] = ' ';
            }
            redactd = true;
        }
        if (has_sn == true and redactd == false)
            readMBEEPROMID(i, id);
        
        if (strcmp(label, badgeLabels[i].c_str())!=0) {
            changed = true;
            badgeLabels[i] = label;
        }
        if (memcmp(badgeSerialNumbers[i], id, sizeof(id))!=0) {
            changed = true;
            memcpy(badgeSerialNumbers[i], id, sizeof(id));
        }
    }
    if(changed) {
        updateMinibadgeLEDs();
        minibadgesChanged = true;
    }
    return changed;
}

void updateMinibadgeLEDs() {
    for(int x=0; x<4; x++){
        leds[minibadgeLEDNums[x]] = CRGB::Black;
        for(int y=0; y<4; y++){
            if(badgeLabels[x] == simpleMinibadges[y]){
                leds[minibadgeLEDNums[x]] = CRGB::White;
            }
        }
        for(int y=0; y<25; y++){
            if(badgeLabels[x] == minibadgeList[y]){
                leds[minibadgeLEDNums[x]] = minibadgeColors[y];
            }
        }
    }
    //FastLED.show();
}

bool WifiEnable() {
    if (WifiConnect("BadgeNet-OutOfScope", "compukidmikeismycopilot"))
        return true;
    String ssid =  EEPROM.readString(SSID_ADDRESS);    
    String password = EEPROM.readString(PASSWORD_ADDRESS);
    if (WifiConnect(ssid, password))
        return true;
    ErrorScreen("WiFi Connection Error", "Could not connect").Run();
    return false;
}


bool WifiConnect(String ssid, String password) {
    puts("Starting Wifi");
    WiFi.begin(ssid.c_str(), password.c_str());
    int wificonncounter = 0;
    while(WiFi.status() != WL_CONNECTED){
        leds[WIFI_LED] = CRGB::Red;
        FastLED.show();
        delay(100);
        leds[WIFI_LED] = CRGB::Black;
        FastLED.show();
        delay(100);
        wificonncounter ++;
        if(wificonncounter > 75){
            return false;
        }
    }
    return true;
}

bool WifiDisable() {
    puts("Stopping Wifi");
    return WiFi.disconnect(true);
}

String ChooseSsid() {
    
    leds[WIFI_LED] = CRGB::Yellow;
    FastLED.show();
    gfx->setTextSize(1);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(10, 230);
    gfx->draw16bitRGBBitmap(0,0,(uint16_t*)sclogo,320,240);
    gfx->print("Scanning networks...");
    gfx->flush();
    WiFi.disconnect();
    int numSsid = WiFi.scanNetworks();
    leds[WIFI_LED] = CRGB::Black;
    FastLED.show();
    if (numSsid <= 0) {
        ErrorScreen("Couldn't get a wifi connection", "No networks found").Run();
        return String();
    }
    gfx->draw16bitRGBBitmap(0,0,(uint16_t*)sclogo,320,240);
    gfx->setTextSize(2);
    gfx->setCursor(80, 0);
    gfx->print("Choose SSID");
    gfx->flush();
    
    Menu ssidmenu;
    ssidmenu.AllowEscape(true);
    for (int i=0; i< numSsid; ++i) {
        ssidmenu.AddOption(WiFi.SSID(i));
        if (i>=7) break;
    }
    return ssidmenu.Run();  
}

bool ConfigWifi() {
    String ssid = ChooseSsid();
    if (ssid.isEmpty()) {
        return false;
    }
    
    String password = EntryScreen("Wifi Password", 50, "").Run();
    gfx->setTextSize(1);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(10, 230);
    gfx->draw16bitRGBBitmap(0,0,(uint16_t*)sclogo,320,240);
    gfx->print("Connecting...");
    gfx->flush();

    if (WifiConnect(ssid, password)) {
        EEPROM.writeString(SSID_ADDRESS, ssid);
        EEPROM.writeString(PASSWORD_ADDRESS, password);
        EEPROM.commit();
        return true;
    }
    else {
        ErrorScreen("Wifi error", "Unable to connect").Run();
        return false;
    }
    return false;
}