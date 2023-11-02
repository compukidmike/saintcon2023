#include "Window.hpp"
#include "request.h"
#include "Keyboard.hpp"
#include "assets/ui/inventory.h"
#include "badge.h"


const char* slots[] ={"helmet", "armor", "weapon", "item"};

class InventoryScreen {
public:
    InventoryScreen() {
        for (int i=0; i<4; ++i) {
            names[i] = "Nothing equiped";
        }
    };
    ~InventoryScreen() {};

    void Run() {
        if (!GetLootData())
            return;
        keyboard.ClearEvents();
        bool in_request = true;
        RequestLoadBadge();
        RequestSendMinibadges();
        minibadgesChanged = false;
        RequestWaitingScreen("Loading gear");
        while (true) {
            uint32_t now = millis();
            if (in_request && RequestIsComplete()) {
                if (!ParseMiniBadgeData()) 
                    return;
                in_request = false;
            }
            if ((!in_request) && minibadgesChanged) {
                RequestSendMinibadges();
                minibadgesChanged = false;
                in_request = true;
            }
            if (Update())
                return;
            Draw();
            gfx->flush();
        }
    }

    void Draw() {
        String line= "Level " + String(g_badgeinfo.level) + "/4";
        const int rw = 36;
        gfx->setTextSize(2);
        gfx->fillScreen(0x49a5);
        gfx->setCursor(80, 0);
        gfx->print(line);
        gfx->draw16bitRGBBitmap(8,  24+rw*0, (uint16_t*)helmet_ico, 32, 32);
        gfx->draw16bitRGBBitmap(8, 24+rw*1, (uint16_t*)armor_ico, 32, 32);
        gfx->draw16bitRGBBitmap(8, 24+rw*2, (uint16_t*)sword_ico, 32, 32);
        gfx->draw16bitRGBBitmap(8, 24+rw*3, (uint16_t*)item_ico, 32, 32);
        gfx->draw16bitRGBBitmap(8, 24+rw*4, (uint16_t*)gold_ico, 32, 32);
        gfx->draw16bitRGBBitmap(8, 24+rw*5, (uint16_t*)chest_ico, 32, 32);
        gfx->setTextColor(0xFFFF);
        for (int i=0; i<4; ++i) {
            gfx->setCursor(56, 30+rw*i);
            gfx->print(names[i]);
        }
        gfx->setCursor(56, 30+rw*4);
        line= String(gold) + " coin(s)";
        gfx->print(line);
        line = String(loot_codes) + " loot drop(s)";
        gfx->setCursor(56, 30+rw*5);
        gfx->print(line);

        gfx->setTextSize(1);
        for (int i=0; i<4; ++i) {
            gfx->setCursor(72, 48 + i*rw);
            gfx->print(desc[i]);
        }       
    }

    bool Update() {
        if (keyboard.KeyPressEvent(KEY_A) || keyboard.KeyPressEvent(KEY_B))
            return true;
        return false;
    }

private:
    bool GetLootData() {
        StaticJsonDocument<256> doc;
        RequestCheckBank();
        RequestWaitingScreen("Checking vault");
        if (!RequestGetResponse(doc)) {
            ErrorScreen("Unable to check account", RequestGetError());
            return false;
        }
        gold = doc["response"]["balance"];
        RequestCheckVend();
        RequestWaitingScreen("Checking vault");
        if (!RequestGetResponse(doc)) {
            ErrorScreen("Unable to check account", RequestGetError());
            return false;
        }
        loot_codes = doc["response"]["pending_lootdrop"];
        return true;
    };

    bool ParseMiniBadgeData() {
        DynamicJsonDocument doc(2048);
        if (!RequestGetResponse(doc)) {
            ErrorScreen("Unable to identify gear", RequestGetError());
            return false;
        }
        JsonObject resp = doc["response"];
        for (int i=0; i<4; ++i) {
            names[i] = "Nothing equiped";
            desc[i] = "";
        }
        for (int i=0; i<4 ;++i) {
            const char* slot = slots[i];
            if (resp.containsKey(slot)) {
                names[i] = resp[slot]["name"].as<String>();
                desc[i] = resp[slot]["stat"].as<String>();
            }
        }
        return true;
    }

    uint gold, loot_codes;
    String names[4];
    String desc[4];
};