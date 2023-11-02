#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "Keyboard.hpp"
#include "sclogo.h"
#include "assets/ui/wind.h"
#include "assets/ui/committee.h"
#include <vector>
#include"AnimatedSprite.h"

extern Arduino_Canvas *gfx;

class Window {
public:
    Window() {};
    ~Window() {};

    void SetBounds(int left, int top, int w, int h) {
        x = left;
        y = top;
        width = w;
        height = h;
    };

    virtual void Draw() {
        const uint16_t bg = 0x11a8;
        const uint16_t tc = 0xf81f;
        const uint16_t fc = 0x65f;

        gfx->fillRect(x-3, y-3, width+3, height+3, bg);
        gfx->fillRect(x+12, y+height, width-12, 7, bg);
        gfx->fillRect(x+width, y+12, 7, height-12, bg);
        gfx->draw16bitRGBBitmapWithTranColor(x-12, y-12, (uint16_t*)wind_top, tc, WIND_TOP_WIDTH, WIND_TOP_HEIGHT);
        gfx->draw16bitRGBBitmapWithTranColor(x-12, y, (uint16_t*)wind_left, tc, WIND_LEFT_WIDTH, WIND_LEFT_HEIGHT);
        gfx->draw16bitRGBBitmapWithTranColor(x+width, y-12, (uint16_t*)wind_right, tc, WIND_RIGHT_WIDTH, WIND_RIGHT_HEIGHT);
        gfx->draw16bitRGBBitmapWithTranColor(x-12, y+height, (uint16_t*)wind_bot, tc, WIND_BOT_WIDTH, WIND_BOT_HEIGHT);
        gfx->draw16bitRGBBitmapWithTranColor(x+width, y+height, (uint16_t*)wind_botright, tc, WIND_BOTRIGHT_WIDTH, WIND_BOTRIGHT_HEIGHT);
        gfx->drawFastHLine(x+72, y-4, width-72, fc);
        gfx->drawFastHLine(x+72, y-5, width-72, 0);
        gfx->drawFastHLine(x+12, y+height+7, width-12, fc);
        gfx->drawFastHLine(x+12, y+height+8, width-12, 0);
        gfx->drawFastVLine(x-4, y+36, height-36, fc);
        gfx->drawFastVLine(x-5, y+36, height-36, 0);
        gfx->drawFastVLine(x+width+7, y+12, height-12, fc);
        gfx->drawFastVLine(x+width+8, y+12, height-12, 0);

    };
protected:
    int x, y, width, height;
};


class DialogWindow : public Window {
public:
    DialogWindow(String Speaker, String Message) : frame(0) , has_avatar(true){
        speaker = Speaker;
        const int max_chars = 23;
        while (Message.length() > max_chars) {
            int best=max_chars;
            for (int i=max_chars-1; i>0; --i) {
                char c = Message.charAt(i);
                if ((c == ' ') || (c == '.')) {
                    best = i+1;
                    break;
                }
            }
            lines.push_back(Message.substring(0,best));
            Message.remove(0, best);
        }
        lines.push_back(Message);
        SetBounds(24,12,286,lines.size()*20);

        avatar.SetGif(speaker);
    };
    ~DialogWindow() {};

    virtual void Draw() {
        gfx->draw16bitRGBBitmap(0,0,(uint16_t*)sclogo,320,240);
        Window::Draw();
        gfx->setTextColor(0xce99);
        gfx->setTextSize(2);
        for (int i=0; i<lines.size(); ++i) {
            gfx->setCursor(30, 15 + i * 20);
            gfx->print(lines[i]);
        }
        if (speaker.equals("Player")) {
            gfx->setCursor(0, 230);
            avatar.Draw(-32, 144);
        }
        else {
            gfx->setCursor(240, 230);
            avatar.Draw(64, 64);
        }
        gfx->setTextSize(1);
        gfx->print(speaker);
        if (++frame % 2) 
            gfx->fillRect(x+width, 12+height, 4, 4, 0xce99);
    };
    
    virtual void Run() {
        keyboard.ClearEvents();
        while (true) {
            Draw();
            gfx->flush();
            if (keyboard.KeyPressEvent(KEY_A))
                return;
        }
    }

private:
    String speaker;
    bool has_avatar;
    AnimatedSprite avatar;
    std::vector<String>lines;
    int frame;
};

class ErrorScreen {
public:
    ErrorScreen(String err, String detail) : message(err), message2(detail), frame(0) {};
    ~ErrorScreen() {};

    void Draw() {
        gfx->fillScreen(0xf800);
        gfx->setTextSize(3);
        gfx->setTextColor(0xFFFF);
        gfx->setCursor(106, 10);
        gfx->print("ERROR!");
        gfx->setTextSize(2);
        gfx->setCursor(10, 100);
        gfx->print(message);
        gfx->setCursor(10, 120);
        gfx->print(message2);
        if (frame++ % 2)
            gfx->fillRect(310, 230, 4, 4, 0xFFFF);
    }

    void Run() {
        keyboard.ClearEvents();
        while (true) {
            Draw();
            gfx->flush();
            if (keyboard.KeyPressEvent(KEY_A))
                return;
        }
    };
private:
    String message, message2;
    int x,y;
    int frame;
};

#endif