#include <Arduino.h>
#include "Menu.h"
#include "AnimatedSprite.h"

struct party_member {
    AnimatedSprite avatar;
    String name;
};

void ShowPartyManager();

class NoPartyScreen {
public:
    NoPartyScreen();
    ~NoPartyScreen();

    bool Run();
    bool Update();
    void Draw();

private:
    Menu options;
};

class ListPartyScreen {
public:
    ListPartyScreen();
    ~ListPartyScreen();

    virtual bool Run();
    virtual bool Update();
    virtual void Draw();

protected:
    void ParsePartyInfo();
    party_member members[4];
    int member_count;
    int nextUpdate;
    bool inRequest;
    int frame;
    ulong hold_start;
};

class FindPartyScreen {
public:
    FindPartyScreen();
    ~FindPartyScreen();

    bool Run();
    bool Update();
    void Draw();

private:
    int frame;
};

class SharePartyScreen : public ListPartyScreen{
public:
    SharePartyScreen();
    ~SharePartyScreen();

    bool Run();
    void Draw();

private:
};