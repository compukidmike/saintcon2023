#include "EntryScreen.h"
#include "Keyboard.hpp"
#include <Arduino_GFX_Library.h>
#include "request.h"
#include "Keyboard.hpp"
#include "utils.h"
#include "badge.h"

extern Arduino_Canvas *gfx;

const char keypad[5][14] = {
    {"0123456789@._"},
    {"ABCDEFGHIJKLM"},
    {"NOPQRSTUVWXYZ"},
    {"abcdefghijklm"},
    {"nopqrstuvwxyz"}
};

bool EntryScreen::Update() {

    if (keyboard.KeyPressEvent(KEY_UP)) 
        sely = (sely+5)%6;
    if (keyboard.KeyPressEvent(KEY_DOWN))
        sely = (sely+1)%6; 
    if (keyboard.KeyPressEvent(KEY_LEFT))
        selx = (selx+12)%13;
    if (keyboard.KeyPressEvent(KEY_RIGHT))
        selx = (selx+1)%13;
    if (keyboard.KeyPressEvent(KEY_A)) {
        if (sely < 5) {
            char c = keypad[sely][selx];
            if (text.length() >= maxChars) 
                text.setCharAt(maxChars-1, c);
            else
                text.concat(c);
        }
        else {
            return true;
        }
    }
    if (keyboard.KeyPressEvent(KEY_B)) {
        if (text.length() > 0) {
            text.remove(text.length()-1);
        }
    }
    return false;
}
    
void EntryScreen::Draw() {
    int16_t x1, y1;
    uint16_t w, h;
    gfx->fillScreen(0x11a8);

    String ent = text;

    if (((frame++ % 10) > 4) && (text.length() < maxChars)){
        ent += "_";
    }

    gfx->setTextSize(3);
    gfx->setTextColor(0xFFFF);
    gfx->getTextBounds(title, 0, 10, &x1, &y1, &w, &h);
    gfx->setCursor(160 - w / 2, 10);
    gfx->print(title);
    gfx->setTextSize(fsize);
    gfx->drawRect(155 - maxChars * 3 * fsize, 48, maxChars * 16 * fsize + 8, 12 * fsize, 0x7bcf);
    gfx->setCursor(160 - maxChars * 3 * fsize, 53);
    gfx->setTextColor(0x0525);
    gfx->print(ent);
    gfx->setTextSize(2);

    if (sely < 5)
        gfx->fillRoundRect(selx*24 + 4, sely*24 + 90, 24, 24, 4, 0x14db);
    else
        gfx->fillRoundRect(135, sely*24 + 90, 48, 24, 4, 0x14db);

    for (int x = 0; x< 13; ++x) {
        for (int y=0; y< 5; ++ y) {
            gfx->drawChar(x*24 + 10, y*24 + 94, keypad[y][x], 0xce99, 0xce99);
        }
    }

    gfx->setTextSize(1);
    gfx->setTextColor(0xce99);
    gfx->getTextBounds("DONE", 160, 216, &x1, &y1, &w, &h);
    gfx->setCursor(160 - w / 2, 218);
    gfx->print("DONE");
}

String EntryScreen::Run() {
    if (maxChars > 25) fsize = 1;
    keyboard.ClearEvents();
    while (true) {
        if (Update()) {
            return GetInput();
        }
        Draw();
        gfx->flush();
    }
}

String EntryScreenIR::Run() {
    decode_results results;
    if (maxChars > 25) fsize = 1;
    keyboard.ClearEvents();
    irrecv.enableIRIn(); 
    leds[IR_LED] = CRGB::Blue;
    FastLED.show();
    while (true) {
        if (Update()) {
            leds[IR_LED] = CRGB::Black;
            FastLED.show();
            irrecv.disableIRIn();
            return GetInput();
        }
        Draw();
        gfx->flush();
        if (irrecv.decode(&results)) {
            uint32_t code = results.value;
            text = String(code, 10);
        }
    }
}