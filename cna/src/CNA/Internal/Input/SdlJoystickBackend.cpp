// SPDX-License-Identifier: MS-PL
#include "CNA/Internal/Input/SdlJoystickBackend.hpp"

namespace CNA::Internal::Input
{
    namespace
    {
        // Real SDL-backed implementation: every method forwards 1:1 to the matching SDL2 function.
        class RealSdlJoystickBackend final : public ISdlJoystickBackend
        {
        public:
            SDL_Joystick* OpenJoystick(int deviceIndex) override
            {
                return SDL_JoystickOpen(deviceIndex);
            }
            void CloseJoystick(SDL_Joystick* joystick) override
            {
                SDL_JoystickClose(joystick);
            }
            std::string GetJoystickName(SDL_Joystick* joystick) override
            {
                const char* s = SDL_JoystickName(joystick);
                return s ? s : "";
            }
            SDL_JoystickType GetJoystickType(SDL_Joystick* joystick) override
            {
                return SDL_JoystickGetType(joystick);
            }
            std::string GetJoystickGUID(SDL_Joystick* joystick) override
            {
                char buffer[33] = {};
                SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), buffer, sizeof(buffer));
                return buffer;
            }
            int GetNumJoystickAxes(SDL_Joystick* joystick) override
            {
                return SDL_JoystickNumAxes(joystick);
            }
            int GetNumJoystickButtons(SDL_Joystick* joystick) override
            {
                return SDL_JoystickNumButtons(joystick);
            }
            int GetNumJoystickHats(SDL_Joystick* joystick) override
            {
                return SDL_JoystickNumHats(joystick);
            }
            int GetNumJoystickBalls(SDL_Joystick* joystick) override
            {
                return SDL_JoystickNumBalls(joystick);
            }
            Sint16 GetJoystickAxis(SDL_Joystick* joystick, int axis) override
            {
                return SDL_JoystickGetAxis(joystick, axis);
            }
            bool GetJoystickButton(SDL_Joystick* joystick, int button) override
            {
                return SDL_JoystickGetButton(joystick, button) != 0;
            }
            Uint8 GetJoystickHat(SDL_Joystick* joystick, int hat) override
            {
                return SDL_JoystickGetHat(joystick, hat);
            }
            bool GetJoystickBall(SDL_Joystick* joystick, int ball, int* dx, int* dy) override
            {
                return SDL_JoystickGetBall(joystick, ball, dx, dy) == 0;
            }
            // SDL2's joystick power query (unlike SDL3's SDL_GetJoystickPowerInfo) reports only a
            // coarse SDL_JoystickPowerLevel, never a percentage -- mapped onto the CNA-level
            // SDL_PowerState/percent shape this interface already declares (see the header's
            // updated doc), *percent always -1 ("unknown"), matching XNA's own
            // GamePadState.Battery "not always available" contract.
            SDL_PowerState GetJoystickPowerInfo(SDL_Joystick* joystick, int* percent) override
            {
                if (percent) *percent = -1;
                switch (SDL_JoystickCurrentPowerLevel(joystick))
                {
                    case SDL_JOYSTICK_POWER_WIRED:
                        return SDL_POWERSTATE_NO_BATTERY;
                    case SDL_JOYSTICK_POWER_EMPTY:
                    case SDL_JOYSTICK_POWER_LOW:
                    case SDL_JOYSTICK_POWER_MEDIUM:
                        return SDL_POWERSTATE_ON_BATTERY;
                    case SDL_JOYSTICK_POWER_FULL:
                        return SDL_POWERSTATE_CHARGED;
                    case SDL_JOYSTICK_POWER_UNKNOWN:
                    default:
                        return SDL_POWERSTATE_UNKNOWN;
                }
            }
        };

        RealSdlJoystickBackend g_realBackend;
        ISdlJoystickBackend*   g_currentBackend = &g_realBackend;
    }

    ISdlJoystickBackend& sdl_joystick_backend()
    {
        return *g_currentBackend;
    }
}
