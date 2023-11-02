#include <ArduinoJson.h>
#include "Window.hpp"

int DungeonSelect();


class Dungeon {
  public:
    Dungeon(int ID);
    ~Dungeon();

    void Run();

  private:   
    int id;
};