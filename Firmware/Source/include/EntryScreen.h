#ifndef ENTRYSCREEN_H
#define ENTRYSCREEN_H

#include <vector>
#include <Arduino.h>

class EntryScreen {
public:
    EntryScreen(String Title, int charcount, String value) : text(value), title(Title), maxChars(charcount), selx(0), sely(0), fsize(2) {};
    ~EntryScreen() {};


    virtual String Run(int wifiEntry = 0);

    bool Update(int wifiEntry = 0);
    void Draw(int keyBoardType = 0, int wifiEntry = 0);
    String GetInput() {return text;};

protected:
    int fsize;
    int selx, sely, frame;
    int maxChars;
    String text;
    String title;
};

class EntryScreenIR : public EntryScreen {
public:
    EntryScreenIR(String Title, int charcount, String value) : EntryScreen(Title, charcount, value) {};
    ~EntryScreenIR() {};

    String Run();
};

#endif