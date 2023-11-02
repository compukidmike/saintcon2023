#ifndef REQUESTS_H
#define REQUESTS_H

#include <stdbool.h>
#include <ArduinoJson.h>    

bool RequestIsComplete();
bool RequestGetResponse(JsonDocument &doc);
String RequestGetError();
void RequestCancel();

void RequestWaitingScreen(String message);
void RequestWaitNoScreen();
bool RequestOnlyValidate();


bool RequestLoadBadge();

bool RequestRegister(int pclass, int pchar, char* name, char* email);
bool RequestBattleAction(String action);
bool RequestBattleUpdate();

bool RequestListDungeons();
bool RequestEnterDungeon(int id);
bool RequestLeaveDungeon(int id);
bool RequestDungeonUpdate(bool confirm);

bool RequestGetParty();
bool RequestStartParty();
bool RequestJoinParty(int party);
bool RequestLeaveParty(int party, bool destroy);

bool RequestSendMinibadges();

bool RequestCheckBank();
bool RequestCheckVend();
bool RequestGetVendCode(uint lifespan);
bool RequestFirmwareVersion();


bool RequestSendLevelUpCode(int code);
bool RequestGetLevelUpCode(uint lifespan);

#endif