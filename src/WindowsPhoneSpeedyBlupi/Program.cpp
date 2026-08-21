/**
 * @file Program.cpp
 * @brief Application entry point for the SpeedyBlupi / mobile-eggbert game.
 * @details Constructs the top-level Game1 object, invokes its Run() loop, and
 *          handles any unhandled C++ exceptions before the process exits.
 *
 * ### Exception handling and logging strategy
 * The entire game construction and run loop is wrapped in two catch blocks:
 * 1. @c catch(const std::exception&) — catches all standard library exceptions
 *    and exceptions derived from @c std::exception.  The @c what() message is
 *    logged at ERROR level and the process exits with code 1.
 * 2. @c catch(...) — catches all remaining exceptions (e.g. non-standard throws).
 *    A generic fatal message is logged at ERROR level and the process exits
 *    with code 1.
 *
 * Logging uses CNA::Logger throughout.  The minimum log level is raised to
 * @c ERROR immediately after entering @c main() so that only error messages
 * are emitted in production builds; lower-severity messages around construction
 * and Run() are therefore silenced at runtime but remain available when the
 * log level is lowered for debugging.
 *
 * On Android, CNA/Platform/Entrypoint.hpp renames @c main to @c SDL_main so that the
 * SDL Java bridge (SDLActivity.nativeRunMain) can locate the entry point.
 * Platform-specific entry-point plumbing is fully encapsulated in that header;
 * game code must never include @c <SDL3/SDL_main.h> directly.
 */

//using WindowsPhoneSpeedyBlupi;

#include <iostream>

// CNA/Platform/Entrypoint.hpp handles the SDL_main renaming required on Android so that
// SDL's Java bridge (SDLActivity.nativeRunMain) can locate main() as SDL_main.
// Game code must never include <SDL3/SDL_main.h> directly.
#include "CNA/Platform/Entrypoint.hpp"

#include "CNA/Logger.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "WindowsPhoneSpeedyBlupi/Game1.hpp"

/**
 * @brief Application entry point — creates Game1 and runs the game loop.
 *
 * @details Constructs a WindowsPhoneSpeedyBlupi::Game1 instance, calls its
 *          Run() method which blocks until the window is closed, then returns
 *          0. Under Emscripten the instance has static storage because CNA's
 *          browser main-loop callback outlives the stack frame that registered
 *          it. Any unhandled exception terminates the process with exit code 1
 *          after logging the error through CNA::Logger.
 *
 * @param[in] argc Number of command-line arguments (passed through by SDL on
 *                 all platforms; not currently used by the game).
 * @param[in] args Array of command-line argument strings.
 * @return 0 on normal exit; 1 if an unhandled exception was caught.
 *
 * @note On Android this function is renamed to SDL_main by CNA/Platform/Entrypoint.hpp.
 *
 * @throws Nothing — all exceptions are caught internally and converted to a
 *         non-zero return value.
 */
int main(int argc, char* args[])
{
    CNA::Logger::Info("SpeedyBlupi: main entered");
    CNA::Logger::SetMinimumLevel(CNA::LogLevel::ERROR);
    CNA::Logger::Info("SpeedyBlupi: before Game1 construction");
    try
    {
#if defined(__EMSCRIPTEN__)
        // emscripten_set_main_loop(..., simulateInfiniteLoop=1) unwinds the
        // current Wasm stack after registering CNA's browser callback. Keep the
        // game alive in static storage so the callback never retains a pointer
        // to a reclaimed stack object.
        static WindowsPhoneSpeedyBlupi::Game1 game;
#else
        WindowsPhoneSpeedyBlupi::Game1 game;
#endif
        CNA::Logger::Info("SpeedyBlupi: Game1 constructed, entering Run()");
        game.Run();
        CNA::Logger::Info("SpeedyBlupi: Run() returned normally");
    }
    catch (const std::exception& e)
    {
        CNA::Logger::Error(std::string("SpeedyBlupi: fatal exception in main: ") + e.what());
        return 1;
    }
    catch (...)
    {
        CNA::Logger::Error("SpeedyBlupi: unknown fatal exception in main");
        return 1;
    }
    CNA::Logger::Info("SpeedyBlupi: exiting normally");
    return 0;
}
