#include "Boss.h"
#include "Hero.h"
#include "Menu.h"
#include <ArduinoJson.h>
#include <queue>

#define BG_COMPUTER1    1
#define BG_COMPUTER2    2
#define BG_FIELD        3
#define BG_STAR         4
#define BG_TRON         5


//10 fps
#define FRAME_RATE  100

enum game_state {
    CheckEquip,
    SendingEquip,
    ReceivingEquipResponse,
    WaitingInput,
    SendingInput,
    ReceivingInputResponse,
    SendingPoll,
    ReceivingPollResponse,
    RoundReady,
    ActionPlayerAttack,
    ActionEnemyDamage,
    ActionEnemyAttack,
    ActionPlayerDamage,
    ActionResolveRound,
    ActionEnemyDefeat,
    BattleComplete,
    Defeat,
};


class Battle {
public:
    Battle(JsonObject doc, int background);
    ~Battle() {};

    void Run();

protected:
    void DrawScene();
    bool ParseRound(JsonDocument &doc);
    bool UpdateGear(bool force);
    void SetMenuItems(bool item);

    Boss boss;
    Hero players[4];
    AnimatedSprite effect;

    std::queue<int> attack_order;

    int m_player_count;
    const uint16_t* m_background;

    Menu menu;
    int m_frame;
    int m_faiure_count;
    game_state state;
    String m_action;
    bool uptodate;
    String message;
};