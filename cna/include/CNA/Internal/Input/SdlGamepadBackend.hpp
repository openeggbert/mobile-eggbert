// SPDX-License-Identifier: MS-PL
#pragma once

#include <SDL2/SDL.h>

// Internal (CNA) seam over SDL2's game-controller C API used by SdlInputBridge.
//
// This exists ONLY so gamepad runtime behavior (hot-plug, slot assignment) can be unit-tested
// without real hardware: tests inject a fake implementation. It is NOT part of the XNA public
// API and must never be exposed there. Production uses the real SDL-backed implementation by
// default.
namespace CNA::Internal::Input
{
    /**
     * @brief Abstract seam over the SDL2 game-controller hot-plug operations SdlInputBridge depends on.
     *
     * Each method forwards 1:1 to the corresponding `SDL_GameController*` function in the real
     * implementation. The `SDL_GameController*` handle is opaque and only ever passed back to
     * this seam, so a fake may return any non-null sentinel it recognizes.
     *
     * @note SDL2's `SDL_IsGameController`/`SDL_GameControllerOpen` take a device index (which
     * changes as controllers connect/disconnect), not the stable instance id SDL3's equivalent
     * functions took -- the only caller (SdlInputBridge's `SDL_CONTROLLERDEVICEADDED` handler)
     * already has a device index on hand for that specific event (SDL2 documents
     * `SDL_ControllerDeviceEvent::which` as a device index on ADDED, unlike REMOVED/REMAPPED
     * where it's the instance id), so no conversion is needed at the one real call site.
     */
    class ISdlGamepadBackend
    {
    public:
        virtual ~ISdlGamepadBackend() = default;

        /** @brief Whether the joystick device index is a recognized gamepad (SDL_IsGameController). */
        virtual bool IsGamepad(int deviceIndex) = 0;
        /** @brief Opens the gamepad for the device index, or nullptr on failure (SDL_GameControllerOpen). */
        virtual SDL_GameController* OpenGamepad(int deviceIndex) = 0;
        /** @brief Closes an opened gamepad handle (SDL_GameControllerClose). */
        virtual void CloseGamepad(SDL_GameController* gamepad) = 0;
    };

    /**
     * @brief Returns the currently active SDL gamepad backend (the real SDL one unless overridden).
     */
    ISdlGamepadBackend& sdl_gamepad_backend();
}
