#include "AttractScreen.h"

#include <Arduino_GFX_Library.h>
#include <math.h>

#include "Keyboard.hpp"
#include "AnimatedSprite.h"
#include "sclogo.h"
#include "utils.h"

#include "assets/maps/tron.h"
#include "assets/maps/field.h"
#include "assets/maps/star.h"
#include "assets/maps/computer.h"

#include "assets/bosses/Sheep_gif.h"
#include "assets/bosses/Planet_gif.h"
#include "assets/bosses/LadyRobot_gif.h"
#include "assets/bosses/Monitors_gif.h"

#include "assets/heros/hero1.h"
#include "assets/heros/hero2.h"
#include "assets/heros/hero3.h"
#include "assets/heros/hero4.h"
#include "assets/heros/avatar1.h"
#include "assets/heros/avatar2.h"
#include "assets/heros/avatar3.h"
#include "assets/heros/avatar4.h"

#include "assets/ui/inventory.h"

extern Arduino_Canvas *gfx;
extern void setLCDBacklight(uint8_t level);
extern unsigned long lastButtonPress;

volatile bool demoModeActive = false;

static const unsigned long ATTRACT_LOOP_MS = 30000UL;
static const unsigned long FRAME_MS = 80UL;

static const char *committeeNames[] = {
    "Berly", "BryceKunz", "Chunk", "Compukidmike", "honki", "Jup1t3r", "Kampf",
    "Katie", "Klipper", "Pali", "Pope", "Ray-man", "Redactd", "Scr4m",
    "Sirashrum", "SJ", "Supertechguy", "Zevlag", "Zodiak"
};
static const char *committeeLabels[] = {
    "Berly", "BryceKunz", "Chunk", "Compukidmike", "Honki", "Jup1t3r", "Kampf",
    "Katie", "Klipper", "Pali", "Pope", "Ray-man", "Redactd", "Scr4m",
    "Sirashrum", "SJ", "Supertechguy", "Zevlag", "Zodiak"
};
static const int COMMITTEE_COUNT = sizeof(committeeNames) / sizeof(committeeNames[0]);

static const int player_pos[4][2] = {
    {0, 20}, {0, 100}, {40, 6}, {40, 140},
};

enum AttractScene {
    SC_PRESS = 0,
    SC_MENU,
    SC_SHEEP,
    SC_PLANET,
    SC_LADY,
    SC_AVATARS,
    SC_INVENTORY,
    SC_ROSTER,
    SC_FINALE
};

static AttractScene sceneForTime(unsigned long t, unsigned long &localT) {
    if (t < 3500) { localT = t; return SC_PRESS; }
    if (t < 7500) { localT = t - 3500; return SC_MENU; }
    if (t < 13500) { localT = t - 7500; return SC_SHEEP; }
    if (t < 16000) { localT = t - 13500; return SC_PLANET; }
    if (t < 18500) { localT = t - 16000; return SC_LADY; }
    if (t < 21500) { localT = t - 18500; return SC_AVATARS; }
    if (t < 25000) { localT = t - 21500; return SC_INVENTORY; }
    if (t < 28000) { localT = t - 25000; return SC_ROSTER; }
    localT = t - 28000;
    return SC_FINALE;
}

static bool anyKeyPressed() {
    for (int i = 0; i < 7; ++i) {
        if (keyboard.KeyPressEvent(i))
            return true;
    }
    return false;
}

static void textCenter(const char *text, int y, uint8_t size, uint16_t color) {
    int16_t x1, y1;
    uint16_t w, h;
    gfx->setTextSize(size);
    gfx->setTextColor(color);
    gfx->getTextBounds((char *)text, 0, y, &x1, &y1, &w, &h);
    gfx->setCursor(160 - (int)w / 2, y);
    gfx->print(text);
}

static void loadHeroes(AnimatedSprite *heroes) {
    heroes[0].SetGif((uint8_t *)hero_1, HERO1_SIZE);
    heroes[1].SetGif((uint8_t *)hero_2, HERO2_SIZE);
    heroes[2].SetGif((uint8_t *)hero_3, HERO3_SIZE);
    heroes[3].SetGif((uint8_t *)hero_4, HERO4_SIZE);
}

static void loadAvatars(AnimatedSprite *sprites) {
    sprites[0].SetGif((uint8_t *)avatar1, AVATAR1_SIZE);
    sprites[1].SetGif((uint8_t *)avatar2, AVATAR2_SIZE);
    sprites[2].SetGif((uint8_t *)avatar3, AVATAR3_SIZE);
    sprites[3].SetGif((uint8_t *)avatar4, AVATAR4_SIZE);
}

static void drawMatrixIcon(int ox, int oy, unsigned long t) {
    gfx->fillRect(ox + 2, oy + 1, 28, 26, 0x1a24);
    gfx->drawRect(ox + 2, oy + 1, 28, 26, 0x354a);
    gfx->fillRect(ox + 5, oy + 4, 22, 18, 0x0080);
    gfx->drawPixel(ox + 27, oy + 3, 0x07e0);
    for (int i = 0; i < 4; ++i)
        gfx->fillRect(ox + 6 + i * 5, oy + 27, 3, 4, 0xd520);

    int sw = 22, sh = 18;
    int sx = ox + 5, sy = oy + 4;
    for (int c = 0; c < sw; c += 2) {
        int speed = 1 + (c % 3);
        int head = (int)((t / 80) * speed + c * 3) % (sh + 8);
        for (int r = 0; r < sh; ++r) {
            int dist = (head - r + (sh + 8)) % (sh + 8);
            if (dist > 6)
                continue;
            uint16_t color = (dist == 0) ? 0xCFFC : (dist <= 2) ? 0x07E0 : 0x03A0;
            gfx->drawPixel(sx + c, sy + r, color);
        }
    }
}

static void scenePressStart(unsigned long t) {
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)sclogo, 320, 240);
    gfx->fillRect(0, 0, 320, 18, 0x0000);
    textCenter("SAINTCON 2023  -  BADGE DEMO", 3, 1, 0xADF5);
    if ((t / 500) % 2 == 0)
        textCenter("PRESS A TO START", 210, 2, 0xFFE0);
}

static void sceneMenu(unsigned long t) {
    gfx->fillScreen(0x01e7);
    textCenter("Main Menu", 12, 3, 0xFFFF);

    const char *opts[] = {
        "Incidents", "Party", "Inventory", "Vending machine",
        "Edit Character", "Level-up", "Wifi"
    };
    const int n = 7;
    int selected = (int)(t / 600) % n;
    int box_w = 180;
    int x0 = 160 - box_w / 2;
    int y0 = 55;
    gfx->fillRoundRect(x0 - 6, y0 - 6, box_w + 12, n * 22 + 12, 6, 0x0948);
    for (int i = 0; i < n; ++i) {
        int y = y0 + i * 22;
        if (i == selected)
            gfx->fillRoundRect(x0, y, box_w, 20, 4, 0x14db);
        gfx->setTextSize(2);
        gfx->setTextColor(0xce99);
        gfx->setCursor(x0 + 10, y + 2);
        gfx->print(opts[i]);
    }
    textCenter("A: select   B: back", 220, 1, 0x8C71);
}

static void sceneBattle(unsigned long lt, const uint16_t *bg, AnimatedSprite &boss,
                        AnimatedSprite *heroes, const char *bossName,
                        const char *status, bool showMenu) {
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)bg, 320, 240);
    for (int i = 0; i < 4; ++i)
        heroes[i].Draw(player_pos[i][0], player_pos[i][1]);
    boss.Draw(0, 8);

    gfx->setTextSize(1);
    gfx->setTextColor(0xFFE0);
    gfx->setCursor(8, 2);
    gfx->print(bossName);
    gfx->fillRect(0, 226, 320, 14, 0x0000);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(8, 228);
    gfx->print(status);

    if (showMenu) {
        const char *opts[] = {"Attack", "Cower", "Use Item"};
        int sel = (millis() / 500) % 3;
        int mw = 110, mh = 72;
        int mx = 320 - mw - 10, my = 240 - mh - 20;
        gfx->fillRoundRect(mx, my, mw, mh, 4, 0x0948);
        for (int i = 0; i < 3; ++i) {
            if (i == sel)
                gfx->fillRoundRect(mx + 2, my + i * 24 + 2, mw - 4, 22, 3, 0x14db);
            gfx->setTextSize(2);
            gfx->setTextColor(0xce99);
            gfx->setCursor(mx + 10, my + 5 + i * 24);
            gfx->print(opts[i]);
        }
    }
    (void)lt;
}

static void sceneAvatars(unsigned long t, AnimatedSprite *avatars) {
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)field_img, 320, 240);
    for (int y = 0; y < 240; y += 2)
        gfx->drawFastHLine(0, y, 320, 0x1082);

    textCenter("CHOOSE YOUR OPERATIVE", 8, 2, 0xFFFF);
    const char *labels[] = {"Alpha", "Bravo", "Charlie", "Delta"};
    int selected = (int)(t / 800) % 4;
    for (int i = 0; i < 4; ++i) {
        int x = 12 + i * 78;
        int y = 50;
        if (i == selected) {
            y += (int)(4.0f * sinf((float)t / 160.0f));
            gfx->fillRoundRect(x - 4, y - 4, 98, 98, 6, 0x14db);
        }
        avatars[i].Draw(x - 15, y - 15);
        gfx->setTextSize(1);
        gfx->setTextColor(0xFFFF);
        gfx->setCursor(x + 10, 155);
        gfx->print(labels[i]);
    }
    if ((t / 333) % 2 == 0)
        textCenter("A: confirm character", 210, 2, 0xFFE0);
}

static void sceneInventory(unsigned long t) {
    gfx->fillScreen(0x49a5);
    textCenter("Level 3/4", 4, 2, 0xFFFF);

    char goldLine[32];
    char lootLine[32];
    snprintf(goldLine, sizeof(goldLine), "%d coin(s)", 120 + (int)((t / 140) % 40));
    snprintf(lootLine, sizeof(lootLine), "%d loot drop(s)", 2 + (int)((t / 1000) % 3));

    const char *names[] = {
        "Tactical Hood", "Packet Jacket", "TheOne - redactd",
        "Zero-day Charm", goldLine, lootLine
    };
    const uint16_t *icons[] = {
        helmet_ico, armor_ico, nullptr, item_ico, gold_ico, chest_ico
    };

    for (int i = 0; i < 6; ++i) {
        int y = 28 + i * 34;
        if (i == 2) {
            drawMatrixIcon(10, y, t);
            gfx->drawRect(48, y + 2, 252, 26, 0x07E0);
        } else {
            gfx->draw16bitRGBBitmap(10, y, (uint16_t *)icons[i], 32, 32);
        }
        gfx->setTextSize(2);
        gfx->setTextColor(0xFFFF);
        gfx->setCursor(52, y + 8);
        gfx->print(names[i]);
    }
    textCenter("equip gear  -  spend loot codes", 220, 1, 0xDDF0);
}

static void sceneRoster(unsigned long t, AnimatedSprite &face, AnimatedSprite &monitors, int &lastIdx) {
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)star_img, 320, 240);
    monitors.Draw(0, 8);

    int idx = (int)(t / 100) % COMMITTEE_COUNT;
    if (idx != lastIdx) {
        face.SetGif(String(committeeNames[idx]));
        lastIdx = idx;
    }
    face.Draw(0, 8);

    textCenter("COMMITTEE HIGH SCORE", 6, 2, 0xFFE0);
    gfx->fillRect(60, 200, 200, 28, 0x0000);
    textCenter(committeeLabels[idx], 205, 2, 0xA7F5);
}

static void sceneFinale(unsigned long t) {
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)sclogo, 320, 240);
    int y = (int)((t / 16) % 240);
    gfx->fillRect(0, y, 320, 3, 0x8410);
    gfx->fillRect(0, 100, 320, 40, 0x0000);
    textCenter("INSERT BADGE", 108, 3, 0xFFE0);
    if ((t / 400) % 2 == 0)
        textCenter("TO CONTINUE", 132, 2, 0xADF5);
}

// Ring LEDs 10-17, accents elsewhere — demo-only light show
static const uint8_t RING_LEDS[8] = {
    RING_LED_1, RING_LED_2, RING_LED_3, RING_LED_4,
    RING_LED_5, RING_LED_6, RING_LED_7, RING_LED_8
};
static const uint8_t ACCENT_LEDS[] = {
    BATT_LED, MB1_LED, MB2_LED, MB3_LED, MB4_LED,
    BOTTOM_LED_1, BOTTOM_LED_2, BOTTOM_LED_3, BUTTON_LED, WIFI_LED, IR_LED
};
static const int ACCENT_COUNT = sizeof(ACCENT_LEDS) / sizeof(ACCENT_LEDS[0]);

static void ledsClearAll() {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

static void attractUpdateLeds(unsigned long t, AttractScene sc) {
    // Pattern changes with scene + a slow morph inside each scene
    uint8_t mode = (uint8_t)sc;
    uint8_t phase = (uint8_t)(t / 40);

    switch (mode) {
    case SC_PRESS: {
        // Breathing cyan/magenta (SC23 split)
        uint8_t breath = beatsin8(24, 40, 220);
        for (int i = 0; i < 8; ++i) {
            bool left = i < 4;
            leds[RING_LEDS[i]] = left ? CHSV(224, 255, breath) : CHSV(128, 255, breath);
        }
        leds[BUTTON_LED] = CHSV(160, 200, breath);
        leds[BATT_LED] = CHSV(224, 255, breath / 2);
        break;
    }
    case SC_MENU: {
        // Soft rotating highlight around the ring
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        for (int i = 0; i < 8; ++i) {
            uint8_t bri = sin8(phase * 8 + i * 32);
            leds[RING_LEDS[i]] = CHSV(160, 180, scale8(bri, 200));
        }
        leds[BUTTON_LED] = CHSV(96, 255, 180);
        break;
    }
    case SC_SHEEP:
    case SC_PLANET:
    case SC_LADY: {
        // Minibadge slots: single-LED Knight Rider; ring: rotating circle.
        // Color ↔ white each bounce — sheep red, planet blue, lady purple.
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        CRGB primary = (mode == SC_SHEEP) ? CRGB::Red
                       : (mode == SC_PLANET) ? CRGB::Blue
                       : CRGB(160, 0, 255);  // lady: purple
        static const uint8_t MB_LEDS[4] = {MB1_LED, MB2_LED, MB3_LED, MB4_LED};
        const int kStepMs = 110;
        const int kSteps = 6;   // 0→3→1 then repeat
        const int kHalf = 3;
        int step = (int)((t / kStepMs) % kSteps);
        int pos = (step <= kHalf) ? step : (kSteps - step);
        bool useWhite = (((t / kStepMs) / kHalf) & 1) != 0;
        CRGB c = useWhite ? CRGB::White : primary;

        // Single eye bouncing across the 4 minibadge RGB slots
        leds[MB_LEDS[pos]] = c;

        // Ring: smooth rotating circle in primary, white accent matching bounce
        int ringHead = (int)((t / 40) % 8);
        for (int i = 0; i < 8; ++i) {
            int dist = (i - ringHead + 8) % 8;
            if (dist > 4) dist = 8 - dist;
            uint8_t bri = (dist == 0) ? 255 : (dist == 1) ? 160 : (dist == 2) ? 70 : 25;
            CRGB rc = (dist == 0 && useWhite) ? CRGB::White : primary;
            leds[RING_LEDS[i]] = rc;
            leds[RING_LEDS[i]].nscale8(bri);
        }
        leds[BUTTON_LED] = c;
        leds[BUTTON_LED].nscale8(140);
        break;
    }
    case SC_AVATARS: {
        // Rainbow swirl on the ring
        for (int i = 0; i < 8; ++i)
            leds[RING_LEDS[i]] = CHSV(phase * 2 + i * 32, 240, 220);
        for (int i = 0; i < ACCENT_COUNT; ++i)
            leds[ACCENT_LEDS[i]] = CHSV(phase + i * 20, 200, 120);
        break;
    }
    case SC_INVENTORY: {
        // Matrix green rain dripping down the ring
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        for (int i = 0; i < 8; ++i) {
            int head = (phase / 3) % 8;
            int dist = (head - i + 8) % 8;
            uint8_t bri = (dist == 0) ? 255 : (dist == 1) ? 160 : (dist < 4) ? 60 : 15;
            leds[RING_LEDS[i]] = CHSV(96, 255, bri);
        }
        leds[BUTTON_LED] = CHSV(96, 255, 200);
        leds[MB1_LED] = CHSV(96, 200, beatsin8(30, 40, 180));
        leds[MB2_LED] = CHSV(96, 200, beatsin8(30, 40, 180, 0, 64));
        leds[MB3_LED] = CHSV(96, 200, beatsin8(30, 40, 180, 0, 128));
        leds[MB4_LED] = CHSV(96, 200, beatsin8(30, 40, 180, 0, 192));
        break;
    }
    case SC_ROSTER: {
        // Twinkle / high-score sparkle
        fadeToBlackBy(leds, NUM_LEDS, 40);
        leds[RING_LEDS[random8(8)]] = CHSV(random8(40) + 20, 200, 255);  // gold-ish
        leds[RING_LEDS[random8(8)]] = CRGB::White;
        leds[ACCENT_LEDS[random8(ACCENT_COUNT)]] = CHSV(random8(), 180, 200);
        leds[BUTTON_LED] = CHSV(32, 255, beatsin8(20, 100, 255));
        break;
    }
    case SC_FINALE:
    default: {
        // Full badge party: comet + color waves
        for (int i = 0; i < 8; ++i)
            leds[RING_LEDS[i]] = CHSV(phase * 3 + i * 28, 255, 230);
        for (int i = 0; i < ACCENT_COUNT; ++i) {
            leds[ACCENT_LEDS[i]] = CHSV(phase * 5 + i * 18, 255, beatsin8(18, 60, 220, 0, i * 16));
        }
        break;
    }
    }

    FastLED.show();
}

void AttractScreenRun() {
    demoModeActive = true;
    setLCDBacklight(128);
    gfx->displayOn();
    // Do not touch WiFi here — disable/enable was popping ErrorScreen and aborting the loop.
    keyboard.ClearEvents();
    // Ignore spurious presses from the idle→demo transition for a beat
    unsigned long ignoreKeysUntil = millis() + 400;

    uint8_t savedBrightness = FastLED.getBrightness();
    FastLED.setBrightness(80);
    ledsClearAll();

    // 4 party slots + boss + aux = 6 (MAX_GIFS)
    AnimatedSprite party[4];
    AnimatedSprite boss;
    AnimatedSprite aux;

    loadHeroes(party);
    boss.SetGif((uint8_t *)Sheep, SHEEPBOSS_SIZE);

    AttractScene prev = SC_PRESS;
    int rosterIdx = -1;
    unsigned long loopStart = millis();
    unsigned long lastFrame = 0;

    while (true) {
        if (millis() >= ignoreKeysUntil && anyKeyPressed())
            break;
        else if (millis() < ignoreKeysUntil)
            keyboard.ClearEvents();

        unsigned long now = millis();
        if (now - lastFrame < FRAME_MS)
            continue;
        lastFrame = now;

        unsigned long t = (now - loopStart) % ATTRACT_LOOP_MS;
        unsigned long localT = 0;
        AttractScene sc = sceneForTime(t, localT);

        if (sc != prev) {
            // Load assets needed for the new scene
            switch (sc) {
            case SC_SHEEP:
                loadHeroes(party);
                boss.SetGif((uint8_t *)Sheep, SHEEPBOSS_SIZE);
                break;
            case SC_PLANET:
                loadHeroes(party);
                boss.SetGif((uint8_t *)Planet_Boss, PLANETBOSS_SIZE);
                break;
            case SC_LADY:
                loadHeroes(party);
                boss.SetGif((uint8_t *)Lady_Robot_Boss, LADYBOSS_SIZE);
                break;
            case SC_AVATARS:
                loadAvatars(party);
                break;
            case SC_ROSTER:
                aux.SetGif((uint8_t *)Monitors_Boss, MONITORSBOSS_SIZE);
                rosterIdx = -1;
                boss.SetGif(String(committeeNames[0]));
                break;
            default:
                break;
            }
            prev = sc;
        }

        switch (sc) {
        case SC_PRESS:
            scenePressStart(localT);
            break;
        case SC_MENU:
            sceneMenu(localT);
            break;
        case SC_SHEEP:
            sceneBattle(localT, tron_img, boss, party, "BOSS: sheep",
                        localT < 3000 ? "Party engages sheep..." : "klipper attacks for 42 damage",
                        localT < 2500);
            break;
        case SC_PLANET:
            sceneBattle(localT, star_img, boss, party, "BOSS: planet", "Incident in progress...", false);
            break;
        case SC_LADY:
            sceneBattle(localT, computer_img, boss, party, "BOSS: Lady Robot", "Incident in progress...", false);
            break;
        case SC_AVATARS:
            sceneAvatars(localT, party);
            break;
        case SC_INVENTORY:
            sceneInventory(localT);
            break;
        case SC_ROSTER:
            sceneRoster(localT, boss, aux, rosterIdx);
            break;
        case SC_FINALE:
            sceneFinale(localT);
            break;
        }

        attractUpdateLeds(localT, sc);
        gfx->flush();
    }

    ledsClearAll();
    FastLED.setBrightness(savedBrightness);
    keyboard.ClearEvents();
    demoModeActive = false;
    lastButtonPress = millis();
    setLCDBacklight(128);
}
