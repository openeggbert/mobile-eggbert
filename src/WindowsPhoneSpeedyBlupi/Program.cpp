//using WindowsPhoneSpeedyBlupi;

#include <iostream>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "WindowsPhoneSpeedyBlupi/Game1.hpp"

int main(int argc, char *args[]) {
    WindowsPhoneSpeedyBlupi::Game1 *game = new WindowsPhoneSpeedyBlupi::Game1();
    game->Run();

    delete game;
}
