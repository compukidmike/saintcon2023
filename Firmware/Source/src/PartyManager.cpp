#include "PartyManager.h"
#include "request.h"
#include "Keyboard.hpp"
#include "Window.hpp"
#include "utils.h"
#include "badge.h"
#include "EntryScreen.h"

#include "assets/heros/avatar1.h"
#include "assets/heros/avatar2.h"
#include "assets/heros/avatar3.h"
#include "assets/heros/avatar4.h"

uint64_t party_id;
String party_owner;

IRrecv irrecv(IR_RX_PIN);
IRsend irsend(IR_TX_PIN);

void ShowPartyManager() {
    if (!g_badgeinfo.trained) {
        ErrorScreen("Can not join a party yet", "Training not completed").Run();
        return;
    }
    while (true) {
        DynamicJsonDocument doc(1024);
        RequestGetParty();
        RequestWaitingScreen("Checking party status");
        if (RequestGetResponse(doc)) {
            party_id = doc["party_code"];
            party_owner = doc["party_onwer"].as<String>();
            if (!SharePartyScreen().Run())
                return;
        }
        else {
            if (!NoPartyScreen().Run()) 
                return;
        }
    }
}

bool LeaveParty() {
    if (!RequestLeaveParty(party_id, party_owner.equals(badge_id)))
        return false;
    RequestWaitingScreen("Leaving party");
    return RequestOnlyValidate();
}

bool FindParty() {
    String codestr = EntryScreenIR("Enter Party Code", 6, "").Run();
    party_id = codestr.toInt();
    if (party_id) {
        if (!RequestJoinParty(party_id))
            return false;
        RequestWaitingScreen("Joining party");
        if (!RequestOnlyValidate()) {
            ErrorScreen("Couldn't Join Party", RequestGetError()).Run();
            return false;
        }
        return true;
    }
    return false;
}

bool StartParty() {
    StaticJsonDocument<256> doc;
    RequestStartParty();
    RequestWaitingScreen("Creating party");
    if (!RequestGetResponse(doc)) {
        ErrorScreen("Couldn't create Party", RequestGetError()).Run();
        return false;
    }
    party_id = doc["response"]["code"];
    return true;
}

NoPartyScreen::NoPartyScreen() {
    options.AddOption("Start Party");
    options.AddOption("Find Party");
    options.AllowEscape(true);
}
NoPartyScreen::~NoPartyScreen() {}

bool NoPartyScreen::Run() {
    keyboard.ClearEvents();
    while (true) {
        if (Update()) {
            String pick = options.GetSelected();
            if (pick.equals("Start Party")) {
                return StartParty();
            }
            else if (pick.equals("Find Party")) {
                return FindParty();
            }
            else    
                return false;
        }
        Draw();
        gfx->flush();
    }
}

bool NoPartyScreen::Update() { 
    return options.Update();
}
void NoPartyScreen::Draw() {
    gfx->fillScreen(0x01e7);
    gfx->setTextColor(0xFFFF);
    gfx->setTextSize(2);
    gfx->setCursor(20,10);
    gfx->print("You are not in a party");
    options.Draw();
}

ListPartyScreen::ListPartyScreen(): nextUpdate(0), member_count(0), inRequest(false), hold_start(0) {}

ListPartyScreen::~ListPartyScreen() {}

bool ListPartyScreen::Run() {
    keyboard.ClearEvents();
    while (true) {
        if (Update()) {
            return false;
        }
        if ((hold_start != 0) && (millis() > (hold_start + 3000))) {
            FastLED.show();
            return LeaveParty();
        }
        Draw();
        gfx->flush();
    }
}


bool ListPartyScreen::Update() {
    if (keyboard.KeyPressEvent(KEY_B))
        return true;
    if (inRequest) {
        if (RequestIsComplete()) {
            ParsePartyInfo();
            inRequest = false;
            nextUpdate = millis() + 3000;
        }
    } else {
        if (millis() > nextUpdate) {
            RequestGetParty();
            inRequest = true;
        }
    }
    if (keyboard.IsKeyDown(KEY_A)) {
        if (hold_start == 0) 
            hold_start = millis();
    }
    else
        hold_start = 0;
    return false;
}
   
void ListPartyScreen::Draw() {
    gfx->fillScreen(0x01e7);
    gfx->setTextColor(0xFFFF);
    gfx->setTextSize(1);
    for (int i=0; i<member_count; ++i) {
        members[i].avatar.Draw(i*80, 40);
        gfx->setCursor(i*80+20, 160);
        gfx->print(members[i].name);
    }
    gfx->setCursor(20, 10);
    gfx->setTextSize(2);
    gfx->print("Hold A to leave party");
    gfx->setCursor(100, 30);
    gfx->print(String(party_id));
    if (hold_start) {
        ulong dt = millis() - hold_start;
        int hw = 320 * dt / 3000;
        gfx->fillRect(0, 0, hw, 4, 0x7800);
    }
    /* 
    gfx->setTextSize(1);
    gfx->setCursor(10, 230);
    String line = "Sending party advertisments";
    for (int i=0; i<=(frame%3); ++i)
        line += ".";
    gfx->print(line);
    //*/
    frame++;
}

void ListPartyScreen::ParsePartyInfo() {
    DynamicJsonDocument doc(1024);
    if (RequestGetResponse(doc)) {
        int i=0;
        for( const auto& member : doc["response"].as<JsonArray>() ) {
            members[i].name = member["handle"].as<String>();
            int avatar = member["character"].as<int>();
            switch (avatar) {
                default:
                case 0:
                    members[i].avatar.SetGif((uint8_t*)avatar1, AVATAR1_SIZE); break;
                case 1:
                    members[i].avatar.SetGif((uint8_t*)avatar2, AVATAR2_SIZE); break;
                case 2:
                    members[i].avatar.SetGif((uint8_t*)avatar3, AVATAR3_SIZE); break;
                case 3:
                    members[i].avatar.SetGif((uint8_t*)avatar4, AVATAR4_SIZE); break;
                    break;
            }
            ++i;
        }
        member_count = i;
    }
}

FindPartyScreen::FindPartyScreen() {}
FindPartyScreen::~FindPartyScreen() {}

bool FindPartyScreen::Run() {
    int party;
    keyboard.ClearEvents();
    while (true) {
        if (Update()) {
            leds[IR_LED] = CRGB::Black;
            FastLED.show();
            return false;
        }
        Draw();
        gfx->flush();
    }
    leds[IR_LED] = CRGB::Black;
    FastLED.show();
    RequestJoinParty(party);
    RequestWaitingScreen("Joining party");
    if (!RequestOnlyValidate()) {
        ErrorScreen("Couldn't join Party", RequestGetError()).Run();
        return false;
    }
    return true;
}
bool FindPartyScreen::Update() {
    if (keyboard.KeyPressEvent(KEY_B))
        return true;
    //TODO: scan for badges?
    return false;
}

void FindPartyScreen::Draw() {
    gfx->setTextSize(1);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(10, 230);
    String line("Looking for party");
    for (int i=0; i<=(frame%3); ++i) 
        line += ".";
    gfx->draw16bitRGBBitmap(0,0,(uint16_t*)sclogo,320,240);
    gfx->print(line);
    gfx->flush();
    frame++;
}


SharePartyScreen::SharePartyScreen() { }
SharePartyScreen::~SharePartyScreen() {}

bool SharePartyScreen::Run() {
    leds[IR_LED] = CRGB::Blue;
    FastLED.show();
    keyboard.ClearEvents();
    uint32_t last_send=0;
    while (true) {
        uint32_t now = millis();
        if (Update()) {
            leds[IR_LED] = CRGB::Black;
            FastLED.show();
            return false;
        }
        if ((hold_start != 0) && (now > (hold_start + 3000))) {
            RequestCancel();
            leds[IR_LED] = CRGB::Black;
            FastLED.show();
            return LeaveParty();
        }
        Draw();
        gfx->flush();
        if (now > last_send + 2000) {
            irsend.sendNEC(party_id);
            last_send = now + (rand() % 200);
        }
    }
}

void SharePartyScreen::Draw() {
    gfx->setTextSize(2);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(10, 40);
    gfx->print("Pair with others to join");
    ListPartyScreen::Draw();
}