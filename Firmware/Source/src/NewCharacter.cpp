#include "NewCharacter.hpp"

const char* class_names[] = {
    "Blue Team",
    "Red Team",
    "Social Engineer",
    "Physical Security",
};

bool UpdateCharacter() {
    Menu cpicker;

    int character = CharacterPicker(g_badgeinfo.character).Run();

    cpicker.SetTitle("Pick your class");
    for (int i=0; i<4; ++i)
        cpicker.AddOption(class_names[i]);
    cpicker.SetSelected(g_badgeinfo.pclass);
    cpicker.AllowEscape(false);
    cpicker.Run();
    int pclass = cpicker.GetSelectedValue();

    String handle = EntryScreen("Handle", 25, g_badgeinfo.handle).Run();

    String email = EntryScreen("Email", 50, g_badgeinfo.email).Run();

    RequestRegister(pclass, character, (char*)handle.c_str(), (char*)email.c_str());
    RequestWaitingScreen("Registering");
    if (RequestOnlyValidate()) {
        g_badgeinfo.character = character;
        g_badgeinfo.pclass = pclass;
        g_badgeinfo.handle = handle;
        g_badgeinfo.email = email;
        return true;
    }
    ErrorScreen("Registration Failed", RequestGetError()).Run();
    return false;
}