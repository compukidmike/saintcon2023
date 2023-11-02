#include "AnimatedSprite.h"


class Boss : public AnimatedSprite {
public:
    Boss() : is_dead(false) {};
    ~Boss() {};

    void SetNameImage(String Name) {
        name = Name;
        SetGif(name);
    }

    bool is_dead;
    String name;
    int health, max_health;
};