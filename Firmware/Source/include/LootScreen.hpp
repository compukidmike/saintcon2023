#include "Window.hpp"
#include "request.h"
#include "Keyboard.hpp"
#include "assets/ui/inventory.h"

class LootScreen {
public:
    LootScreen() : coins(0), loot_codes(0) {};
    ~LootScreen() {};

    void Run(JsonObject &loot) {
        if (loot.containsKey("coins"))
            coins = loot["coins"];
        if (loot.containsKey("lootdrop"))
            loot_codes = loot["lootdrop"];
        keyboard.ClearEvents();
        while (true) {
            if (Update())
                return;
            Draw();
            gfx->flush();
        }
    }

    void Draw() {
        String line= String(coins) + " coin(s)";
        gfx->fillScreen(0x49a5);
        gfx->draw16bitRGBBitmap(8, 192, (uint16_t*)gold_ico, 32, 32);
        gfx->setTextSize(3);
        gfx->setTextColor(0xFFFF);
        gfx->setCursor(0, 10);
        gfx->print("Incident Complete");
        gfx->setTextSize(2);
        gfx->setCursor(110, 104);
        gfx->print("You found");
        gfx->setCursor(56, 200);
        gfx->print(line);
        if (loot_codes) {
            line = String(loot_codes) + "x loot drops";
            gfx->draw16bitRGBBitmap(8, 144, (uint16_t*)chest_ico, 32, 32);
            gfx->setCursor(56, 152);
            gfx->print(line);
        }
    }

    bool Update() {
        if (keyboard.KeyPressEvent(KEY_A) || keyboard.KeyPressEvent(KEY_B))
            return true;
        return false;
    }

private:
    int coins, loot_codes;
};