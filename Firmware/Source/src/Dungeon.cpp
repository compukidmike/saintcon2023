#include "Dungeon.h"
#include "request.h"
#include <Arduino_GFX_Library.h>
#include "Battle.h"
#include "sclogo.h"
#include "Menu.h"
#include <map>
#include "assets/ui/committee.h"
#include "LootScreen.hpp"

extern Arduino_Canvas *gfx;

int DungeonSelect () {
    DynamicJsonDocument doc(2048);
    std::map<String,int> dd;
    RequestListDungeons();
    RequestWaitingScreen("Getting Incident List");
    if (!RequestGetResponse(doc)) {
        ErrorScreen("Get Incident list failed", RequestGetError()).Run();
        return -1; 
    }

    Menu dselect;
    dselect.SetTitle("Choose Incident");
    dselect.AllowEscape(true);
    for(const auto& kv : doc["response"].as<JsonObject>()) {
        if (kv.value()["available"]) {
            String dname = kv.value()["name"].as<String>();
            dselect.AddOption(dname);
            dd[dname] = String(kv.key().c_str()).toInt();
        }
    }

    String selected = dselect.Run();
    if (selected.isEmpty())
        return -1;
    return dd[selected];
}

Dungeon::Dungeon(int ID) : id(ID) { }

Dungeon::~Dungeon() { }

void Dungeon::Run() {
    DynamicJsonDocument doc(2048);
    bool confirm=false;
    
    RequestEnterDungeon(id);
    RequestWaitingScreen("Entering Incident");
    if (!RequestOnlyValidate()) {
        ErrorScreen("Enter Incident failed", RequestGetError()).Run();
        return; 
    }

    int fail_count = 0;

    String message("Starting Incident");
    
    while(true) {
        RequestDungeonUpdate(confirm);
        confirm=false;
        RequestWaitingScreen(message);
        if (!RequestGetResponse(doc)) {
            if (fail_count++ > 10) {
                ErrorScreen("Incident Update failed", RequestGetError()).Run();
                return; 
            } else
                continue;
        }
        
        JsonObject data = doc["response"]["data"];
        String mtype = doc["response"]["type"].as<String>();
        if (mtype.equals("admin")) {
            message = doc["response"]["message"].as<String>();
            if (message.equals("dungeon_complete")) {
                RequestLeaveDungeon(id);
                RequestWaitingScreen("Leaving Incident");
                if (!RequestGetResponse(doc)) {
                    ErrorScreen("Leave Incident failed", RequestGetError()).Run();
                    return; 
                }
                data = doc["response"]["data"];
                LootScreen().Run(data);
                RequestLoadBadge();
                return;
            }
            else if (message.equals("enemy_defeated")) {
                confirm = true;
            }
            gfx->setTextSize(1);
            gfx->setCursor(10, 230);
            gfx->setTextColor(0xFFFF);
            gfx->draw16bitRGBBitmap(0,0,(uint16_t*)sclogo,320,240);
            gfx->print(message);
            gfx->flush();
            delay(500);
        }
        else if (mtype.equals("dialogue")) {
            DialogWindow wind(data["avatar"].as<String>(), data["text"].as<String>());
            wind.Run();
            message = "Continuing Incident";
            confirm = true;
        }
        else if (mtype.equals("battle")) {
            Battle b(data, id);
            b.Run();
        }

    }

}