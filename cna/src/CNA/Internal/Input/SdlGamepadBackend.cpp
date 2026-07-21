// SPDX-License-Identifier: MS-PL
#include "CNA/Internal/Input/SdlGamepadBackend.hpp"

namespace CNA::Internal::Input
{
    namespace
    {
        // Real SDL-backed implementation: every method forwards 1:1 to the matching SDL2 function.
        class RealSdlGamepadBackend final : public ISdlGamepadBackend
        {
        public:
            bool IsGamepad(int deviceIndex) override
            {
                return SDL_IsGameController(deviceIndex) == SDL_TRUE;
            }
            SDL_GameController* OpenGamepad(int deviceIndex) override
            {
                return SDL_GameControllerOpen(deviceIndex);
            }
            void CloseGamepad(SDL_GameController* gamepad) override
            {
                SDL_GameControllerClose(gamepad);
            }
        };

        RealSdlGamepadBackend g_realBackend;
        ISdlGamepadBackend*   g_currentBackend = &g_realBackend;
    }

    ISdlGamepadBackend& sdl_gamepad_backend()
    {
        return *g_currentBackend;
    }
}
