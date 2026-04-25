//using WindowsPhoneSpeedyBlupi;

#include <iostream>

#include "Microsoft/Xna/Framework/Game.hpp"
#include "WindowsPhoneSpeedyBlupi/Game1.hpp"

int main(int argc, char *args[]) {
    CNA::Logger::SetMinimumLevel(CNA::LogLevel::TRACE);
    WindowsPhoneSpeedyBlupi::Game1 game;
    game.Run();
}
