// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "System/MulticastAction.hpp"

#include <cstdint>

namespace CNA::Input
{
    /**
     * @brief NOXNA — raw joystick access (flight sticks, wheels, throttles, arbitrary HID
     *        controllers), backed by SDL3's joystick API.
     *
     * XNA 4.0 only ever modeled Xbox-style mapped gamepads (`Microsoft::Xna::Framework::Input::
     * GamePad`). SDL3's raw joystick API exposes arbitrary axes/buttons/hats/trackballs with no
     * semantic mapping — essential for flight sims, racing wheels, and HOTAS setups that `GamePad`
     * cannot represent. A device SDL also maps as a gamepad is visible here too (as
     * `JoystickTypeEXT::Gamepad`); this is an independent, unmapped view of the same hardware.
     */
    NOXNA class Joysticks
    {
    public:
        /** @brief Static-only utility; not instantiable. */
        Joysticks() = delete;

        /** @brief NOXNA/EXT: fires with the SDL joystick instance id when a joystick is connected. */
        NOXNA static System::MulticastAction<std::uint32_t> ConnectedEXT;

        /** @brief NOXNA/EXT: fires with the SDL joystick instance id when a joystick is disconnected. */
        NOXNA static System::MulticastAction<std::uint32_t> DisconnectedEXT;
    };
}
