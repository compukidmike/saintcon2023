#include "AnimatedSprite.h"
#include "assets/heros/hero1.h"
#include "assets/heros/hero2.h"
#include "assets/heros/hero3.h"
#include "assets/heros/hero4.h"
#include "assets/heros/out.h"
#include <Arduino_GFX_Library.h>
#include "assets/heros/miniico.h"

extern Arduino_Canvas *gfx;


enum haction {
    None,
    Attack,
    Cower,
    Item
};

class Hero : public AnimatedSprite {
public:
    Hero() : max_health(100) , health(100), is_dead(false) , action(None), healing(0) {};

    void setCharacter(int id) {
        switch(id) {
            case 0:  SetGif((uint8_t*)hero_4, HERO4_SIZE); break;
            case 1:  SetGif((uint8_t*)hero_3, HERO3_SIZE); break;
            case 2:  SetGif((uint8_t*)hero_2, HERO2_SIZE); break;
            case 3:  SetGif((uint8_t*)hero_1, HERO1_SIZE); break;
        }
    };
    ~Hero() {};

    bool Draw(int x, int y, int disolve=0) override {
        if (health > 0 ) {
            int h_per = 40 * health / max_health;
            gfx->drawRect(x + 7, y+64, 42, 8, 0xB5B6);
            uint16_t c = 0x03C0;
            if (health < (max_health >> 2))
                c = 0xb0e0;
            else if (health < (max_health >> 1))
                c = 0xef00;
            gfx->fillRect(x + 8, y+65, h_per, 6, c);
            bool ret= AnimatedSprite::Draw(x,y);
            switch (action) {
                case Attack:
                gfx->draw16bitRGBBitmapWithTranColor(x, y, (uint16_t*)mini_attack_ico, 0xf81f, 8, 8);
                break;
                case Cower:
                gfx->draw16bitRGBBitmapWithTranColor(x, y, (uint16_t*)mini_defend_ico, 0xf81f, 8, 8);
                break;
                case Item:
                gfx->draw16bitRGBBitmapWithTranColor(x, y, (uint16_t*)mini_item_ico, 0xf81f, 8, 8);
                break;
            }
            return ret;
        }
        else {
            bool ret = AnimatedSprite::Draw(x,y);
            gfx->draw16bitRGBBitmapWithTranColor(x+20, y, (uint16_t*)out_img, 0xf81f, OUT_IMG_WIDTH, OUT_IMG_HEIGHT);
            return ret;
        }
    };

    int health, max_health, healing;
    int round_attack_dmg, round_dmg_taken;
    bool is_dead;
    String uuid, name;
    haction action;
};