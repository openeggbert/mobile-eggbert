// SPDX-License-Identifier: MS-PL
#include "CNA/Internal/Input/SystemDeviceBackend.hpp"

#include <SDL2/SDL.h>

namespace CNA::Internal::Input
{
    namespace
    {
        struct RealSystemDeviceBackend final : ISystemDeviceBackend
        {
            // SDL2 has no multi-mouse/multi-keyboard device enumeration API at all (SDL_GetMice/
            // SDL_GetKeyboards/SDL_GetMouseNameForID/SDL_GetKeyboardNameForID are SDL3-only
            // additions) -- SDL2 treats every physical mouse as one combined logical pointer and
            // every physical keyboard as one combined logical keyboard, with no per-device
            // identity or name at all. This is a genuine SDL2 capability gap, not a naming
            // difference: both methods report exactly one synthetic "default device" entry
            // instead of a real per-device list. XNA's own input model is merged across devices
            // anyway (this is a NOXNA metadata-only extension, per InputDeviceInfoEXT's own doc),
            // and mobile-eggbert itself never calls either method -- see plan_lite.md's Phase 4
            // status for the full disclosure.
            std::vector<CNA::Input::InputDeviceInfoEXT> GetMice() override
            {
                return { CNA::Input::InputDeviceInfoEXT{0, "Mouse"} };
            }

            std::vector<CNA::Input::InputDeviceInfoEXT> GetKeyboards() override
            {
                return { CNA::Input::InputDeviceInfoEXT{0, "Keyboard"} };
            }

            std::vector<CNA::Input::InputDeviceInfoEXT> GetTouchDevices() override
            {
                // SDL2's touch API enumerates real per-device IDs (SDL_GetNumTouchDevices/
                // SDL_GetTouchDevice), unlike mice/keyboards above, but reports no device name
                // (SDL_GetTouchDeviceName is an SDL3-only addition) -- name is always empty here.
                std::vector<CNA::Input::InputDeviceInfoEXT> devices;
                const int count = SDL_GetNumTouchDevices();
                for (int i = 0; i < count; ++i)
                {
                    devices.push_back({static_cast<std::uint64_t>(SDL_GetTouchDevice(i)), ""});
                }
                return devices;
            }
        };

        RealSystemDeviceBackend g_realBackend;
        ISystemDeviceBackend*   g_currentBackend = &g_realBackend;
    }

    ISystemDeviceBackend& system_device_backend()
    {
        return *g_currentBackend;
    }
}
