//using WindowsPhoneSpeedyBlupi;

#include <iostream>

#include "CNA/Game.h"
#include "WindowsPhoneSpeedyBlupi/Game1.h"

int main(int argc, char *args[]) {
    WindowsPhoneSpeedyBlupi::Game1 *game = new WindowsPhoneSpeedyBlupi::Game1();
    game->Run();


    delete game;
}
