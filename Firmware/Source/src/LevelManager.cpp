#include "LevelManager.h"
#include "Menu.h"
#include "EntryScreen.h"
#include "request.h"
#include "Keyboard.hpp"
#include "utils.h"
#include "badge.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>

void EnterLevelCode() {
    String codestr = EntryScreenIR("Enter Level Code", 6, "").Run();
    int code = codestr.toInt();
    if (code) {
        RequestSendLevelUpCode(code);
        RequestWaitingScreen("Verifing code");
        if (RequestOnlyValidate()) {
            keyboard.ClearEvents();
            int frame = 0;
            while (true) {
                gfx->fillScreen(0x0420);
                gfx->setTextSize(3);
                gfx->setTextColor(0xFFFF);
                gfx->setCursor(106, 10);
                gfx->print("Level UP!!!");
                if (frame++ % 2)
                    gfx->fillRect(310, 230, 4, 4, 0xFFFF);
                gfx->flush();
                if (keyboard.KeyPressEvent(KEY_A))
                    return;
                if (keyboard.KeyPressEvent(KEY_B))
                    return;
            }
        }
        else
            ErrorScreen("Level Up Failed", RequestGetError()).Run();
    }
    else
        ErrorScreen("Level Up Failed", "Invalid code").Run();
}


void ShowLevelCode(int code, int seconds) {
    gfx->fillScreen(0x49a5);
    gfx->setTextColor(0xFFFF);
    gfx->setTextSize(4);
    gfx->setCursor(80, 80);
    gfx->print(String(code));
    gfx->setCursor(4, 220);
    gfx->setTextSize(1);
    String line = "You have " + String(seconds) + " seconds to share the code";
    gfx->print(line);
    gfx->setCursor(4, 230);
    gfx->print("before it expires");
    gfx->flush();
}

void GetLevelCode() {
    StaticJsonDocument<256> doc;
    const int timespan = 100;
    uint32_t last_send = 0;
    RequestGetLevelUpCode(timespan);
    RequestWaitingScreen("Generating code");
    if (RequestGetResponse(doc)) {
        int code = doc["response"]["levelup_code"];
        uint32_t endtime = millis() + timespan * 1000;
        leds[IR_LED] = CRGB::Blue;
        FastLED.show();
        keyboard.ClearEvents();
        while(true) {
            uint32_t now = millis();
            if (now > endtime) {
                leds[IR_LED] = CRGB::Black;
                FastLED.show();
                return;
            }
            ShowLevelCode(code, (endtime - now)/1000);
            if (keyboard.KeyPressEvent(KEY_B)) {
                leds[IR_LED] = CRGB::Black;
                FastLED.show();
                return;
            }
            if (now > last_send + 2000) {
                irsend.sendNEC(code);
                last_send = now + (rand() % 200);
            }
        }
    }
    else   
        ErrorScreen("Level Up Code Error", RequestGetError());
}

void ShowLevelMenu() {
    if (g_badgeinfo.isStaff) {
        Menu mmain;
        mmain.AddOption("Enter Level up code");
        mmain.AddOption("Share Level up code");
        mmain.AllowEscape(true);
        String choice = mmain.Run();
        if (choice == "Enter Level up code") {
            EnterLevelCode();
        }
        else if (choice == "Share Level up code")
        {
            GetLevelCode();
        }
        
    }
    else {
        EnterLevelCode();
    }

}