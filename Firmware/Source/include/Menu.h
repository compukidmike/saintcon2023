#ifndef _MENU_H_
#define _MENU_H_

#include <vector>
#include <Arduino.h>
#include <Window.hpp>
#include "badge.h"

extern void setLCDBacklight(uint8_t level);
extern unsigned long lastButtonPress;
extern int screenDimDelay;
extern int screenOffDelay;

class Menu : public Window {
public:
    Menu() : selected(0), wind_width(16), can_escape(false) {};
    ~Menu() {};

    void AddOption(String item) {
        uint16_t x1,y1;
        int16_t w1,h1;
        int iwidth = (item.length()+1) * 12;
        //gfx->getTextBounds(item, 10, 10, &x1, &y1, &w1, &h1);
        if (iwidth > wind_width)
            wind_width = iwidth;
        options.push_back(item);
        int wind_height = options.size() * 24;
        SetBounds(160 - wind_width/2, 140 - wind_height/2, wind_width, wind_height);
    };
    void ClearOptions() {
        options.clear();
    }

    void SetTitle(String t) { title = t;}
    void SetSelected(int s) {selected = s;};

    void ChangeSelected(int delta) {
        selected += delta + options.size();
        if (options.size())
            selected %= options.size();
    };

    String GetSelected() {if (selected>=0) return options[selected]; return String();};
    int GetSelectedValue() {return selected;};
    void AllowEscape(bool a) {can_escape = a;};

    virtual void Draw() {
        if (!title.isEmpty()) {
            int16_t x1, y1;
            uint16_t w, h;
            gfx->fillScreen(0x01e7);
            gfx->setTextSize(3);
            gfx->setTextColor(0xFFFF);
            gfx->getTextBounds(title, 0, 10, &x1, &y1, &w, &h);
            gfx->setCursor(160 - w / 2, 10);
            gfx->print(title);
        }

        int wind_height = options.size() * 24;
        int x = 160 - wind_width/2;
        int y = 140 - wind_height/2;

        Window::Draw();

        gfx->fillRoundRect(x, y + selected*24, wind_width, 24, 4, 0x14db);

        gfx->setTextSize(2);
        gfx->setTextColor(0xce99);
        for (int i=0; i< options.size(); ++i){
            gfx->setCursor(x+6, 6+y+24*i);
            gfx->print(options[i]);
        }
    };

    bool Update() {
        int count = options.size();
        if (keyboard.KeyPressEvent(KEY_UP)) 
            selected = selected + count - 1;
        if (keyboard.KeyPressEvent(KEY_DOWN))
            selected++;
        if (selected >= count)
            selected -= count;
        if (keyboard.KeyPressEvent(KEY_A))
            return true;
        if (can_escape && keyboard.KeyPressEvent(KEY_B)) {
            selected = -1;
            return true;
        }
        return false;
    };

    String Run() {
        keyboard.ClearEvents();
        lastButtonPress = millis();
        bool wifiup = true;
        while (true) {
            if (Update()) {
                return GetSelected();
            }
            Draw();
            gfx->flush();
            if(millis() - lastButtonPress > screenDimDelay){
                if(millis() - lastButtonPress > screenOffDelay){
                    setLCDBacklight(0);
                    if (wifiup) {
                        WifiDisable();
                        wifiup = false;
                        gfx->displayOff();
                    }
                } else {
                    setLCDBacklight(25);
                }
            } else {
                setLCDBacklight(128);
                if (!wifiup) {
                    if (!WifiEnable())
                        ESP.restart();
                    wifiup = true;
                    gfx->displayOn();
                }
            }
        }
    };

private:
    String title;
    bool can_escape;
    std::vector<String> options;
    int selected, wind_width;
};

#endif