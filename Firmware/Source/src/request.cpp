#include "request.h"
#include <AsyncHTTPSRequest_Generic.h>
#include <Arduino_GFX_Library.h>
#include "sclogo.h"
#include "badge.h"
#include "Window.hpp"
#include "NewCharacter.hpp"
#include "utils.h"

#include <WiFiClientSecure.h>
#include <HTTPClient.h>

extern Arduino_Canvas *gfx;
extern float firmwareVersion;

#define API_BASE    "https://sc23-api.redactd.net:8443"   
String rq_last_error;


AsyncHTTPSRequest request;
bool RequestSendRaw(const char* endpoint, JsonDocument &data) {
    static bool requestOpenResult;
    String url(API_BASE);
    url += endpoint;
    String payload;
    bool ret = false;

    bool hasPayload = !data.isNull();
    printf(hasPayload?"POST ":"GET ");
    puts(endpoint);

    if (request.readyState() == readyStateUnsent || request.readyState() == readyStateDone) {
        requestOpenResult = request.open(hasPayload?"POST":"GET", url.c_str());
        if (requestOpenResult) {
            request.setReqHeader("x-auth-id", badge_id.c_str());
            if (hasPayload) {
                serializeJson(data, payload);
                puts(payload.c_str());
                request.setReqHeader("Content-Type", "application/json");
                ret = request.send(payload);
            }
            else {
                ret = request.send();
            }
        }
        else {
            rq_last_error = "invalid request";
        }
    }
    else {
        rq_last_error = "Can't send request, bad state";
    }
    if (ret) {
        leds[WIFI_LED] = CRGB::Green;
        FastLED.show();
    }
    else {
        puts(rq_last_error.c_str());
    }
    return ret;
}


void RequestCancel() {
    if (readyStateDone != request.readyState())
        request.abort();
    leds[WIFI_LED] = CRGB::Black;
    FastLED.show();
}

bool RequestIsComplete() {
    return readyStateDone == request.readyState();
}

String RequestGetError() {
    return rq_last_error;
}

bool RequestGetResponse(JsonDocument &doc) {
    String resp = request.responseLongText();
    leds[WIFI_LED] = CRGB::Black;
    FastLED.show();
    puts(resp.c_str());
    DeserializationError ret = deserializeJson(doc, resp);
    if (DeserializationError::Ok == ret) {
        if (doc["status"].as<bool>()) 
            return true;
        rq_last_error = doc["error"].as<String>();
        puts(rq_last_error.c_str());
    
    }
    else {
        rq_last_error = ret.c_str();
        puts("Error parsing response:");
        puts(rq_last_error.c_str());
    }
    return false;
}

bool RequestOnlyValidate() {
    DynamicJsonDocument doc(2048);
    return RequestGetResponse(doc);
}

void RequestWaitingScreen(String message) {
    gfx->setTextSize(1);
    gfx->setTextColor(0xFFFF);
    int frame=0;
    while (!RequestIsComplete()) {
        gfx->setCursor(10, 230);
        String line = message;
        for (int i=0; i<=(frame%3); ++i)
            line += ".";
        gfx->draw16bitRGBBitmap(0,0,(uint16_t*)sclogo,320,240);
        gfx->print(line);
        gfx->flush();
        frame++;
    }
}

void RequestWaitNoScreen() {
    int frame=0;
    while (!RequestIsComplete()) {
        uint16_t c = 0xFFFF;

        switch (++frame % 4) {
            case 0:
                c = 0xfb60;
                break;
            case 1:
            case 3:
                c = 0xfcc0;
                break;
            case 2:
                c = 0xfde0;
                break;
        }
            c = 0;
        gfx->fillCircle(4,4,4,c);
        gfx->flush();
    }
}

bool RequestLoadBadge() {

    for (int i=0; i<3; ++i) {
        StaticJsonDocument<512> doc;
        if (!RequestSendRaw("/badge/whois", doc)) { //No internet
            ErrorScreen("Unable to contact server", RequestGetError());
            return false;
        }
        RequestWaitingScreen("Signing in");
        if (RequestGetResponse(doc)) {
            JsonVariant resp = doc["response"];
            if (resp.is<const char*>() && resp.as<String>().equals("unregistered")) {
                if (UpdateCharacter())
                    return true;
            }
            else {
                g_badgeinfo.character = resp["character"];
                g_badgeinfo.email = resp["email"].as<String>();
                g_badgeinfo.eyecolor = resp["eyecolor"];
                g_badgeinfo.haircolor = resp["haircolor"];
                g_badgeinfo.handle = resp["handle"].as<String>();
                g_badgeinfo.pclass = resp["class"];
                g_badgeinfo.trained = resp["completed_training"] > 0;
                g_badgeinfo.isStaff = resp["staff"] > 0;
                g_badgeinfo.level = resp["level"];
                return true;
            }
        }
    }
    ErrorScreen("Internet connection down", "Unable to contact server");
    return false;
}

bool RequestRegister(int pclass, int pchar, char* name, char* email) {
    StaticJsonDocument<256> doc;
    doc["class"] = pclass;
    doc["character"] = pchar;
    doc["handle"] = name;
    doc["email"] = email;
    doc["haircolor"] = 1;
    doc["clothingcolor"] = 1;
    doc["eyecolor"] = 1;
    return RequestSendRaw("/badge/register", doc);
}

bool RequestBattleAction(String action) {
    StaticJsonDocument<256> doc;
    if (action.equals("Attack"))
        doc["type"] = "attack";
    else if (action.equals("Cower"))
        doc["type"] = "cower";
    else if (action.equals("Use Item"))
        doc["type"] = "use_item";
    return RequestSendRaw("/dungeon/battle", doc);
}

bool RequestBattleUpdate() {
   StaticJsonDocument<256> doc;
   return RequestSendRaw("/dungeon/battle/update", doc);
}

bool RequestListDungeons() {
    StaticJsonDocument<256> doc;
    return RequestSendRaw("/dungeon", doc);
}

bool RequestEnterDungeon(int id) {
    StaticJsonDocument<256> doc;
    doc["dummy"] = true;
    String uri("/dungeon/enter/");
    uri += String(id);
    return RequestSendRaw(uri.c_str(), doc);
}

bool RequestLeaveDungeon(int id) {
    StaticJsonDocument<256> doc;
    doc["dummy"] = true;
    String uri("/dungeon/exit/");
    uri += String(id);
    return RequestSendRaw(uri.c_str(), doc);
}

bool RequestDungeonUpdate(bool confirm) {
    StaticJsonDocument<256> doc;
    if (confirm) doc["action"] = "confirm_dialogue";
    return RequestSendRaw("/dungeon/update", doc);
}

bool RequestSendMinibadges() {
    StaticJsonDocument<512> doc;
    const uint8_t zeros[16] = {0};
    
    for (int i=0; i<4; ++i) {
        const char* slot_name[] = {"slot1", "slot2", "slot3", "slot4"};
        
        if (badgeLabels[i].isEmpty())
            doc[slot_name[i]]["name"] = nullptr;
        else
            doc[slot_name[i]]["name"] = badgeLabels[i];
        
        if (memcmp(badgeSerialNumbers[i], zeros, 16))
            doc[slot_name[i]]["serial"] = SerialString(badgeSerialNumbers[i]); 
        
    }   
    return RequestSendRaw("/badge/equip", doc);
}


bool RequestGetParty() {
    StaticJsonDocument<256> doc;
    return RequestSendRaw("/party", doc);
}

bool RequestStartParty() {
    StaticJsonDocument<256> doc;
    doc["dummy"] = true;
    return RequestSendRaw("/party/create", doc);
}

bool RequestJoinParty(int party) {
    StaticJsonDocument<256> doc;
    doc["party_code"]=party;
    return RequestSendRaw("/party/join", doc);
}

bool RequestLeaveParty(int party, bool destroy) {
    StaticJsonDocument<256> doc;
    doc["party_code"]=party;
    return RequestSendRaw(destroy?"/party/destroy":"/party/leave", doc);
}


bool RequestCheckBank() {
    StaticJsonDocument<256> doc;
    return RequestSendRaw("/bank/statement", doc);
}

bool RequestCheckVend() {
    StaticJsonDocument<256> doc;
    return RequestSendRaw("/vend", doc);
}

bool RequestGetVendCode(uint lifespan) {
    StaticJsonDocument<256> doc;
    doc["timeout_seconds"] = lifespan;
    return RequestSendRaw("/vend/unlock", doc);
}

bool RequestFirmwareVersion(){
    StaticJsonDocument<256> doc;
        if (!RequestSendRaw("/badge/firmware_version", doc)) { //No internet
            ErrorScreen("Unable to contact server", RequestGetError());
            return false;
        }
        RequestWaitingScreen("Checking for updates");
        if (RequestGetResponse(doc)) {
            JsonVariant resp = doc["response"];
            if(resp["version"].as<float>() > firmwareVersion){
                RequestWaitingScreen("Updating firmware. Please wait");
                return true;
            } else {
                return false;
            }
        }
    ErrorScreen("Internet connection down", "Unable to contact server");
    return false;
}


bool RequestSendLevelUpCode(int code) {
    StaticJsonDocument<256> doc;
    doc["levelup_code"]=code;
    return RequestSendRaw("/badge/levelup", doc);
}

bool RequestGetLevelUpCode(uint lifespan) {
    StaticJsonDocument<256> doc;
    doc["timeout_seconds"] = lifespan;
    return RequestSendRaw("/badge/generate_levelup", doc);
}