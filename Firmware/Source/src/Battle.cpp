#include "Battle.h"
#include <Arduino_GFX_Library.h>
#include "request.h"
#include "badge.h"

#include "assets/maps/computer.h"
#include "assets/maps/computer2.h"
#include "assets/maps/field.h"
#include "assets/maps/star.h"
#include "assets/maps/tron.h"

#include "assets/heros/heroattack.h"
#include "assets/bosses/bossattack.h"

extern Arduino_Canvas *gfx;

int player_pos[4][2] = {
    {0, 20},
    {0, 100},
    {40, 6},
    {40, 140},
};

Battle::Battle(JsonObject doc, int background) : m_faiure_count(0), m_frame(0), uptodate(false) {
    
    JsonObject enemy = doc["enemy"];
    boss.health = enemy["hp"];
    boss.SetNameImage(enemy["name"].as<String>());

    m_player_count=0;
    for (JsonVariant player : doc["characters"].as<JsonArray>()) {
        players[m_player_count].setCharacter(player["character"]);
        players[m_player_count].uuid = player["badge_id"].as<String>();
        players[m_player_count].health = player["current_hp"];
        players[m_player_count].max_health = player["max_hp"];
        players[m_player_count].name = player["handle"].as<String>();
        players[m_player_count].is_dead = player["in_burnout"] > 0;
        m_player_count++;
    }

    switch(background) {
        case 0:
        case BG_COMPUTER1:  m_background=computer_img; break;
        case BG_COMPUTER2:  m_background=computer2_img; break;
        case 9:
        case BG_FIELD:  m_background=field_img; break;
        case BG_STAR:  m_background=star_img; break;
        case 6:
        case 7:
        case 8:
        default:
        case BG_TRON:  m_background=tron_img; break;
    }

    
}

void Battle::SetMenuItems(bool usable_item) {
    menu.ClearOptions();
    menu.AllowEscape(false);
    menu.AddOption("Attack");
    menu.AddOption("Cower");
    if (usable_item)
        menu.AddOption("Use Item");
}


bool Battle::UpdateGear(bool force) {
    if (force || minibadgesChanged) { //Minibadges
        bool usable_item = false;
        DynamicJsonDocument doc(2048);
        RequestSendMinibadges();
        minibadgesChanged = false;
        RequestWaitingScreen("Loading gear");
        if (!RequestGetResponse(doc)) {
            ErrorScreen("gear check failed", RequestGetError()).Run();
            return false; 
        }
        if (doc["response"].containsKey("item")) {
            usable_item = doc["response"]["item"]["usable"];
        }
        SetMenuItems(usable_item);
    }
    return true;
}

void Battle::Run() {
    unsigned long last = 0;
    if (!UpdateGear(true))
        return;
    state = CheckEquip;
    while (state != BattleComplete) {
        if (m_faiure_count > 10) {
            ErrorScreen("Communication error", "Booted from battle");
            return;
        }
        unsigned long now = millis();
        if (now > (last + FRAME_RATE)) {
            DrawScene();
            last = now;
        }
        switch (state) {
        case CheckEquip:
            if (minibadgesChanged)
                state = SendingEquip;
            else   
                state = WaitingInput;
            break;
        case SendingEquip:
            if (RequestSendMinibadges()) {
                state = ReceivingEquipResponse;
                minibadgesChanged = false;
            }
            else {
                m_faiure_count++;
            }
            break;
        case ReceivingEquipResponse:
            if (RequestIsComplete()) {
                bool usable_item = false;
                DynamicJsonDocument doc(2048);
                if (!RequestGetResponse(doc)) {
                    m_faiure_count++;
                    state = SendingEquip;
                    break;
                }
                if (doc["response"].containsKey("item")) {
                    usable_item = doc["response"]["item"]["useable"];
                }
                SetMenuItems(usable_item);
                state = WaitingInput;
            }
            break;
        case WaitingInput:
            for (int i=0; i<m_player_count; ++i) {
                if (players[i].is_dead && players[i].uuid.equals(badge_id)) { //Skip input as we're dead
                    message = "In cooldown, waiting for others...";
                    state = SendingPoll;
                    break;
                }
            }
            if (menu.Update()) {
                m_action = menu.GetSelected();
                state = SendingInput;
            }
            break;
        case SendingInput:
            if (RequestBattleAction(m_action)) {
                state = ReceivingInputResponse;
            }
            else {
                m_faiure_count++;
            }
            break;
        case ReceivingInputResponse:
            if (RequestIsComplete()) {
                DynamicJsonDocument doc(512);
                if (RequestGetResponse(doc)){
                    message = "Waiting for players...";
                    state = SendingPoll;
                    if (m_action.equals("Use Item")) {
                        SetMenuItems(false);
                    }
                } else { //Server didn't like our action, 
                    if (doc["error"].as<String>().equals("enemy defeated")) {
                        m_frame = 0;
                        state = ActionEnemyDefeat;
                    }
                    else
                        state = WaitingInput;
                    m_faiure_count++;
                }
            }
            break;
        case SendingPoll:
            if (RequestBattleUpdate()) {
                state = ReceivingPollResponse;
            }
            else {
                m_faiure_count++;
            }
            break;
        case ReceivingPollResponse:
            if (RequestIsComplete()) {
                DynamicJsonDocument doc(2048);
                if (RequestGetResponse(doc)) {
                    String mtype = doc["response"]["type"].as<String>();
                    uptodate = doc["response"]["latest_update"];
                    if (mtype.equals("admin")) {
                        message = doc["response"]["message"].as<String>();
                        if (message.equals("send_action"))
                            state = CheckEquip;
                        else
                            state = SendingPoll;
                        break;
                    }
                    else if (!doc["response"].containsKey("data")) {
                        m_faiure_count++;
                        state = SendingPoll;
                        break;
                    }
                    else if (!ParseRound(doc)) { //Not a recoverable error
                        ErrorScreen("Error parsing response", "Booted from battle");
                        return;
                    }
                }
                else { 
                    state = SendingPoll;
                    m_faiure_count++;
                }
            }
            break;
        case RoundReady:
            if (attack_order.empty()) {
                state = ActionEnemyAttack;
                m_frame = 0;
                effect.SetGif((uint8_t*)boss_attack, BOSS_ATTACK_SIZE);
            }
            else {
                m_frame = 0;
                effect.SetGif((uint8_t*)hero_attack, HERO_ATTACK_SIZE);
                state = ActionPlayerAttack;
            }
            break;
        case ActionResolveRound:
            state = CheckEquip; 
            if (!uptodate) {
                state = SendingPoll;
            }
            else {
                bool all_dead = true;
                for (int i=0; i<m_player_count; ++i) {
                    players[i].action = None;
                    players[i].health += players[i].healing;
                    players[i].healing = 0;
                    if (!players[i].is_dead){
                        all_dead = false; //atleast one player is alive
                    }
                }
                if (all_dead) {
                    state = Defeat;
                    m_frame = 0;
                }
            }
            break;
        default:
            break;
        }
    }
}

void Battle::DrawScene() {
    String line;
    gfx->draw16bitRGBBitmap(0,0,(uint16_t*)m_background,320,240);
    gfx->setTextSize(1);
    gfx->setCursor(10, 230);
    gfx->setTextColor(0xFFFF);
    for (int i = 0; i< m_player_count; ++i)
        players[i].Draw(player_pos[i][0], player_pos[i][1]);
    if (state == ActionEnemyDefeat) {
        boss.Draw(0, 8, m_frame);
        if (m_frame == 9) {
            state = BattleComplete;
        }
    }
    else {
        boss.Draw(0, 8);
    }
    switch (state) {
        case SendingInput:
            message = "Sending action...";
        case ReceivingInputResponse:
            gfx->print(message);
            break;
        case SendingPoll:
        case ReceivingPollResponse:
            gfx->print(message);
            break;
        case WaitingInput:
            menu.Draw();
            break;
        case ActionPlayerAttack:
            { 
                int pidx = attack_order.front();
                int targetx = 208, targety = 88;
                if (m_frame < 8) {
                    targetx += (player_pos[pidx][0] - targetx)*(8-m_frame)/8;
                    targety += (player_pos[pidx][1] - targety)*(8-m_frame)/8;
                }
                if (effect.Draw(targetx, targety) == 0) {
                    state = ActionEnemyDamage;
                    m_frame=0;
                }
                line = players[pidx].name + " attacks for " + players[pidx].round_attack_dmg  + " damage";
                gfx->print(line);
            }
            break;
        case ActionEnemyDamage:
            {
                int pidx = attack_order.front();
                line = players[pidx].name + " attacks for " + players[pidx].round_attack_dmg  + " damage";
                gfx->print(line);
                gfx->setCursor(240, 120 - m_frame);
                gfx->print(players[attack_order.front()].round_attack_dmg);
                if (m_frame > 10) {
                    attack_order.pop();
                    if (attack_order.empty()) {
                        if (boss.is_dead) {
                            state = ActionEnemyDefeat;
                            m_frame = 0;
                        }
                        else {
                            state = ActionEnemyAttack;
                            m_frame = 0;
                            effect.SetGif((uint8_t*)boss_attack, BOSS_ATTACK_SIZE);
                        }
                    }
                    else {
                        state = ActionPlayerAttack;
                        m_frame = 0;
                        effect.SetGif((uint8_t*)hero_attack, HERO_ATTACK_SIZE);
                    }
                }
            }
            break;
        case ActionEnemyAttack: 
            if (effect.Draw(0, 56) == 0) {
                for (int i=0; i< m_player_count; ++i)
                    players[i].health -= players[i].round_dmg_taken;
                    
                state = ActionPlayerDamage;
                m_frame = 0;
            }
            break;
        case ActionPlayerDamage:
            for (int i=0; i< m_player_count; ++i) {
                gfx->setCursor(20 + player_pos[i][0], 40 + player_pos[i][1] - m_frame);
                gfx->print(players[i].round_dmg_taken);
            }
            if (m_frame > 10) {
                state = ActionResolveRound;
            }
            break;
        case Defeat:
            gfx->setTextSize(2);
            gfx->setCursor(100, 114);
            gfx->print("Defeat");
            if (m_frame > 10) {
                state = BattleComplete;
            }
            break;
        default:
            break;
    }
    m_frame++;
    gfx->flush();
}


bool Battle::ParseRound(JsonDocument &doc) {
    while (!attack_order.empty())
        attack_order.pop();
    bool roundready = doc["battle_result"];
    for (JsonObject action : doc["response"]["data"].as<JsonArray>()) {
        String actor = action["actor"].as<String>();
        if (actor.equals("enemy")) {
            if (action.containsKey("targets")) {
                for (int i=0; i< m_player_count; ++i) {
                    players[i].round_dmg_taken = 0;
                }
                for(const auto& kv : action["targets"].as<JsonObject>()) {
                    for (int i=0; i< m_player_count; ++i) {
                        if (players[i].uuid.equals(kv.key().c_str())) {
                            players[i].round_dmg_taken = kv.value()["damage_dealt"];
                            players[i].is_dead = kv.value()["target_died"];
                        }
                    }
                }
            }
        }
        else {
            for (int i=0; i< m_player_count; ++i) {
                if (actor.equals(players[i].uuid)) {
                    if (action["action"].as<String>().equals("attack")) {    
                        players[i].action = Attack;
                        players[i].round_attack_dmg = action["damage_dealt"].as<int>();
                        attack_order.push(i);
                        if (action["target_died"].as<bool>())
                            boss.is_dead = true;
                    }
                    else if (action["action"].as<String>().equals("cower")) {
                        players[i].action = Cower;
                    }
                    else if (action["action"].as<String>().equals("use_item")) {
                        players[i].action = Item;
                        players[i].healing = action["heal"].as<int>();
                        printf("Player %d healed for %d\n", i, players[i].healing);
                    }
                }
            }
        }
    }
    if (roundready) 
        state = RoundReady;
    else 
        state = SendingPoll;
    return true;
}