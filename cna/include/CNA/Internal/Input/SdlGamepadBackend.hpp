// SPDX-License-Identifier: MS-PL
#pragma once

#include <SDL3/SDL.h>

// Internal (CNA) seam over the SDL3 gamepad/joystick C API used by SdlInputBridge.
//
// This exists ONLY so gamepad runtime behavior (hot-plug, slot assignment) can be unit-tested
// without real hardware: tests inject a fake implementation. It is NOT part of the XNA public
// API and must never be exposed there. Production uses the real SDL-backed implementation by
// default.
namespace CNA::Internal::Input
{
    /**
     * @brief Abstract seam over the SDL3 gamepad hot-plug operations SdlInputBridge depends on.
     *
     * Each method forwards 1:1 to the corresponding `SDL_*` function in the real implementation.
     * The `SDL_Gamepad*` handle is opaque and only ever passed back to this seam, so a fake may
     * return any non-null sentinel it recognizes.
     */
    class ISdlGamepadBackend
    {
    public:
        virtual ~ISdlGamepadBackend() = default;

        /** @brief Whether the joystick instance id is a recognized gamepad (SDL_IsGamepad). */
        virtual bool IsGamepad(SDL_JoystickID instanceId) = 0;
        /** @brief Opens the gamepad for the instance id, or nullptr on failure (SDL_OpenGamepad). */
        virtual SDL_Gamepad* OpenGamepad(SDL_JoystickID instanceId) = 0;
        /** @brief Closes an opened gamepad handle (SDL_CloseGamepad). */
        virtual void CloseGamepad(SDL_Gamepad* gamepad) = 0;
    };

    /**
     * @brief Returns the currently active SDL gamepad backend (the real SDL one unless overridden).
     */
    ISdlGamepadBackend& sdl_gamepad_backend();
}
