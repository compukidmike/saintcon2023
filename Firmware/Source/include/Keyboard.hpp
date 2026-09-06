#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <Arduino.h>

#define KEY_UP      0
#define KEY_DOWN    1
#define KEY_LEFT    2
#define KEY_RIGHT   3
#define KEY_A       4
#define KEY_B       5
#define KEY_JOY     6

#define BUTTON_B_PIN                    39
#define BUTTON_A_PIN                    40
#define JOY_UP_PIN                      15
#define JOY_DOWN_PIN                    18
#define JOY_LEFT_PIN                    17
#define JOY_RIGHT_PIN                   8
#define JOY_CENTER_PIN                  16

const int key_pins[7] = {JOY_UP_PIN, JOY_DOWN_PIN, JOY_LEFT_PIN, JOY_RIGHT_PIN, BUTTON_A_PIN, BUTTON_B_PIN, JOY_CENTER_PIN};

void KEYBOARD_ISR();

class Keyboard {
public:
    Keyboard() : keystate{false,false,false,false,false,false,false}, keyevent{false,false,false,false,false,false,false} {};
    ~Keyboard() {
        for (int i=0; i<7; ++i)
            detachInterrupt(key_pins[i]);
    };

    void Begin() {
        for (int i=0; i<7; ++i) {
            pinMode(key_pins[i], INPUT_PULLUP);
            attachInterrupt(key_pins[i], KEYBOARD_ISR, CHANGE);
        }
        ClearEvents();
    };

    void ClearEvents() {
        for (int i=0; i<7; ++i)
            keyevent[i] = false;
    }

    void Update() {
        for (int i=0; i<7; ++i) {
            bool pressed = !digitalRead(key_pins[i]);
            if (keystate[i] != pressed) {
                keystate[i] = pressed;
                keyevent[i] = true;
            }
        }
    };

    bool IsKeyDown(int key) {
        return keystate[key];
    }

    bool KeyPressEvent(int key) {
        if ((keystate[key] == true) && (keyevent[key] == true)) {
            keyevent[key] = false;
            return true;
        }
        return false;
    }

    bool KeyReleaseEvent(int key) {
        if ((keystate[key] == false) && (keyevent[key] == true)) {
            keyevent[key] = false;
            return true;
        }
        return false;
    }

private:
    bool keystate[7];
    bool keyevent[7];
};

extern Keyboard keyboard;

#endif