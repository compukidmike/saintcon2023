#ifndef NEWCHARACTER_H
#define NEWCHARACTER_H

#include <Arduino.h>
#include "Keyboard.hpp"
#include "EntryScreen.h"
#include "request.h"
#include "Window.hpp"
#include "badge.h"
#include "AnimatedSprite.h"
#include "Menu.h"

#include "assets/heros/avatar1.h"
#include "assets/heros/avatar2.h"
#include "assets/heros/avatar3.h"
#include "assets/heros/avatar4.h"

class CharacterPicker : public Window {
public:
    CharacterPicker(int start) : selected(start) { 
        SetBounds(110, 90, 100, 100);
        setFace();
    };
    ~CharacterPicker() {};

    int Run() {
        keyboard.ClearEvents();
        while (true) {
            if (Update()) {
                return selected;
            }
            Draw();
            gfx->flush();
        }
    };

    bool Update() {
        bool update = false;
        if (keyboard.KeyPressEvent(KEY_LEFT)) {
            selected = selected + 4 - 1;
            update = true;
        }
        if (keyboard.KeyPressEvent(KEY_RIGHT)) {
            selected++;
            update = true;
        }
        if (selected >= 4)
            selected -= 4;
        if (update)
            setFace();
        if (keyboard.KeyPressEvent(KEY_A))
            return true;
        return false;
    };

    void Draw() {
        gfx->fillScreen(0x01e7);
        gfx->setTextSize(3);
        gfx->setTextColor(0xFFFF);
        gfx->setCursor(20, 10);
        gfx->print("Pick your avatar");
        Window::Draw();
        face.Draw(100,80);
    };

private:
    void setFace() {
        switch(selected) {
            default:
            case 0:
                face.SetGif((uint8_t*)avatar1, AVATAR1_SIZE); break;
            case 1:
                face.SetGif((uint8_t*)avatar2, AVATAR2_SIZE); break;
            case 2:
                face.SetGif((uint8_t*)avatar3, AVATAR3_SIZE); break;
            case 3:
                face.SetGif((uint8_t*)avatar4, AVATAR4_SIZE); break;
                break;
        }
    };
    int selected;
    AnimatedSprite face;
};

bool UpdateCharacter();

#endif