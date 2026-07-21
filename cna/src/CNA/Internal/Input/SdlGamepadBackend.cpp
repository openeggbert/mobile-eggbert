// SPDX-License-Identifier: MS-PL
#include "CNA/Internal/Input/SdlGamepadBackend.hpp"

namespace CNA::Internal::Input
{
    namespace
    {
        // Real SDL-backed implementation: every method forwards 1:1 to the matching SDL3 function.
        class RealSdlGamepadBackend final : public ISdlGamepadBackend
        {
        public:
            bool IsGamepad(SDL_JoystickID instanceId) override
            {
                return SDL_IsGamepad(instanceId);
            }
            SDL_Gamepad* OpenGamepad(SDL_JoystickID instanceId) override
            {
                return SDL_OpenGamepad(instanceId);
            }
            void CloseGamepad(SDL_Gamepad* gamepad) override
            {
                SDL_CloseGamepad(gamepad);
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
