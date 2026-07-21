// SPDX-License-Identifier: MS-PL
#pragma once

#include <SDL2/SDL.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "CNA/Input/JoystickCapabilities.hpp"
#include "CNA/Input/JoystickInfo.hpp"
#include "CNA/Input/JoystickState.hpp"

namespace CNA::Internal::Input
{
    /**
     * @brief Bridge between SDL3 events and CNA internal input state.
     *
     * This bridge knows SDL types, but exposes them only internally.
     */
    class SdlInputBridge
    {
    public:
        /**
         * @brief Processes one SDL event and propagates relevant changes to InputManager.
         */
        static void ProcessEvent(const SDL_Event& event);

        /**
         * @brief Ensures the SDL gamepad subsystem is initialized so gamepad events are delivered.
         *
         * SDL_INIT_VIDEO does not imply SDL_INIT_GAMEPAD, so without this no
         * SDL_EVENT_GAMEPAD_ADDED/REMOVED/AXIS/BUTTON events are ever produced — gamepads would be
         * invisible to GamePad::GetState. Idempotent (ref-counted by SDL); safe to call repeatedly.
         * Initializing the subsystem makes SDL enumerate already-connected pads and queue
         * SDL_EVENT_GAMEPAD_ADDED for each, so pads connected before the first frame become visible.
         * ProcessEvent() calls this lazily on first use; startup code may also call it explicitly.
         */
        static void EnsureGamepadSubsystemInitialized();

        /**
         * @brief Quits the SDL gamepad subsystem if it was initialized (P8-002).
         *
         * Mirrors FNA's own shutdown symmetry: `SDL3_FNAPlatform.ProgramExit` quits
         * `SDL_INIT_VIDEO | SDL_INIT_GAMEPAD` together. CNA's `GraphicsDevice::Dispose` already
         * quits `SDL_INIT_VIDEO`; this is the `SDL_INIT_GAMEPAD` counterpart, called from
         * `Game::Dispose(bool)`. Safe to call even if the subsystem was never initialized
         * (`SDL_QuitSubSystem` is a documented no-op in that case) and safe to call more than once.
         */
        static void ShutdownGamepadSubsystem();

        /** @brief NOXNA/EXT: enumerates the connected raw joysticks (id/name/type). */
        static std::vector<CNA::Input::JoystickInfoEXT> GetJoysticks();

        /** @brief NOXNA/EXT: static hardware shape/identity of a raw joystick; default if not connected. */
        static CNA::Input::JoystickCapabilitiesEXT GetJoystickCapabilities(std::uint32_t id);

        /** @brief NOXNA/EXT: current axis/button/hat/trackball state of a raw joystick; all-empty if not connected. */
        static CNA::Input::JoystickStateEXT GetJoystickState(std::uint32_t id);

        /**
         * @brief Internal (not public CNA::Input API): the raw SDL handle for an opened joystick, so
         *        N-013 Haptics can call `SDL_OpenHapticFromJoystick` on it. nullptr if not connected.
         */
        static SDL_Joystick* GetOpenedJoystickHandle(std::uint32_t id);
    };
}
