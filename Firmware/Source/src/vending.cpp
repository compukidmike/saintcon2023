#include "vending.h"
#include "request.h"
#include "Keyboard.hpp"
#include "Window.hpp"
#include "assets/ui/inventory.h"

const int timespan = 60;

void ShowNoVendCodes() {
    gfx->fillScreen(0x49a5);
    gfx->setTextColor(0xFFFF);
    gfx->setTextSize(2);
    gfx->setCursor(16, 100);
    gfx->print("You have no loot drops");
    gfx->flush();
}

void ShowVendConfirm(int gold, int loot_drops, int hold_start, int seconds) {
    gfx->fillScreen(0x49a5);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(50, 20);
    gfx->setTextSize(2);
    gfx->print("Hold A to get code");
    gfx->draw16bitRGBBitmap(28, 100, (uint16_t*)chest_ico, 32, 32);
    gfx->draw16bitRGBBitmap(28, 140, (uint16_t*)gold_ico, 32, 32);
    String line = String(loot_drops) + " loot drop(s)";
    gfx->setCursor(76, 108);
    gfx->print(line);
    line =  String(gold) + " coin(s)";
    gfx->setCursor(76, 148);
    gfx->print(line);
    if (hold_start) {
        ulong dt = millis() - hold_start;
        int hw = 320 * dt / 3000;
        gfx->fillRect(0, 0, hw, 4, 0x7800);
    }
    gfx->setCursor(4, 220);
    gfx->setTextSize(1);
    gfx->print("You will have " + String(seconds) + " seconds to use your code at the");
    gfx->setCursor(4, 230);
    gfx->print("vending machine once it has been opened.");
    gfx->flush();
}

void ShowVendCode(int code, int seconds) {
    gfx->fillScreen(0x49a5);
    gfx->setTextColor(0xFFFF);
    gfx->setTextSize(4);
    gfx->setCursor(80, 80);
    gfx->print(String(code));
    gfx->setCursor(4, 220);
    gfx->setTextSize(1);
    String line = "You have " + String(seconds) + " seconds to use your code at the";
    gfx->print(line);
    gfx->setCursor(4, 230);
    gfx->print("vending machine before it expires");
    gfx->flush();
}

void ShowVendingScreen() {
    StaticJsonDocument<256> doc;
    RequestCheckVend();
    RequestWaitingScreen("Checking vault");
    if (!RequestGetResponse(doc)) {
        ErrorScreen("Error opening vault", RequestGetError()).Run();
        return;
    }
    int code_count = doc["response"]["pending_lootdrop"];

    RequestCheckBank();
    RequestWaitingScreen("Checking vault");
    if (!RequestGetResponse(doc)) {
        ErrorScreen("Unable to check account", RequestGetError());
        return;
    }
    int gold = doc["response"]["balance"];

    int hold_start = 0;
    keyboard.ClearEvents();
    while(true) {
        ShowVendConfirm(gold, code_count, hold_start, timespan);
        if (keyboard.KeyPressEvent(KEY_B))
            return;
        if (keyboard.IsKeyDown(KEY_A)) {
            if (hold_start == 0) 
                hold_start = millis();
        }
        else
            hold_start = 0;

        if ((hold_start != 0) && (millis() > (hold_start + 3000))) {
            break;
        }
    }

    RequestGetVendCode(timespan);
    RequestWaitingScreen("Generating code");
    if (!RequestGetResponse(doc)){
        ErrorScreen("Couldn't get code", RequestGetError()).Run();
        return;
    }
    int code = doc["response"]["unlock_code"];

    uint32_t endtime = millis() + timespan * 1000;
    keyboard.ClearEvents();
    while(true) {
        if (millis() > endtime)
            return;
        ShowVendCode(code, (endtime - millis())/1000);
        if (keyboard.KeyPressEvent(KEY_B))
            return;
    }

}