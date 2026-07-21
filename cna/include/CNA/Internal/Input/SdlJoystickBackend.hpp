// SPDX-License-Identifier: MS-PL
#pragma once

#include <SDL2/SDL.h>

#include <string>

// Internal (CNA) seam over SDL2's raw joystick C API used by SdlInputBridge.
//
// Deliberately SEPARATE from ISdlGamepadBackend: a gamepad is a *mapped* view of a joystick device
// (button/axis semantics assigned by SDL's gamepad database), while this seam is the *raw* device —
// arbitrary axis/button/hat/trackball counts with no semantic assignment. The same physical device
// may be opened independently through both seams.
//
// This exists ONLY so joystick runtime behavior (hot-plug, capabilities, raw state) can be
// unit-tested without real hardware: tests inject a fake implementation. It is NOT part of the XNA
// public API and must never be exposed there. Production uses the real SDL-backed implementation.
namespace CNA::Internal::Input
{
    /**
     * @brief Abstract seam over the SDL2 raw joystick operations SdlInputBridge depends on.
     *
     * Each method forwards 1:1 to the corresponding `SDL_Joystick*` function in the real
     * implementation. The `SDL_Joystick*` handle is opaque and only ever passed back to this
     * seam, so a fake may return any non-null sentinel it recognizes.
     *
     * @note SDL2's `SDL_JoystickOpen` takes a device index (which changes as devices connect/
     * disconnect), not the stable instance id SDL3's `SDL_OpenJoystick` took -- the only caller
     * (SdlInputBridge's `SDL_JOYDEVICEADDED` handler) already has a device index on hand for that
     * specific event (SDL2 documents `SDL_JoyDeviceEvent::which` as a device index on ADDED,
     * unlike REMOVED where it's the instance id), so no conversion is needed at the one real call
     * site.
     */
    class ISdlJoystickBackend
    {
    public:
        virtual ~ISdlJoystickBackend() = default;

        /** @brief Opens the joystick for the device index, or nullptr on failure (SDL_JoystickOpen). */
        virtual SDL_Joystick* OpenJoystick(int deviceIndex) = 0;
        /** @brief Closes an opened joystick handle (SDL_JoystickClose). */
        virtual void CloseJoystick(SDL_Joystick* joystick) = 0;

        /** @brief Human-readable device name, or "" if unknown (SDL_JoystickName). */
        virtual std::string GetJoystickName(SDL_Joystick* joystick) = 0;
        /** @brief The device's physical category (SDL_JoystickGetType). */
        virtual SDL_JoystickType GetJoystickType(SDL_Joystick* joystick) = 0;
        /** @brief The device's GUID, formatted as a lowercase hex string (SDL_JoystickGetGUID + SDL_JoystickGetGUIDString). */
        virtual std::string GetJoystickGUID(SDL_Joystick* joystick) = 0;

        /** @brief Number of axes the device reports (SDL_JoystickNumAxes). */
        virtual int GetNumJoystickAxes(SDL_Joystick* joystick) = 0;
        /** @brief Number of buttons the device reports (SDL_JoystickNumButtons). */
        virtual int GetNumJoystickButtons(SDL_Joystick* joystick) = 0;
        /** @brief Number of POV hats the device reports (SDL_JoystickNumHats). */
        virtual int GetNumJoystickHats(SDL_Joystick* joystick) = 0;
        /** @brief Number of trackballs the device reports (SDL_JoystickNumBalls). */
        virtual int GetNumJoystickBalls(SDL_Joystick* joystick) = 0;

        /** @brief Raw value of the given axis, range -32768..32767 (SDL_JoystickGetAxis). */
        virtual Sint16 GetJoystickAxis(SDL_Joystick* joystick, int axis) = 0;
        /** @brief Down/up state of the given button (SDL_JoystickGetButton). */
        virtual bool GetJoystickButton(SDL_Joystick* joystick, int button) = 0;
        /** @brief Raw SDL_HAT_* bitmask position of the given hat (SDL_JoystickGetHat). */
        virtual Uint8 GetJoystickHat(SDL_Joystick* joystick, int hat) = 0;
        /** @brief Relative motion of the given trackball since the last read (SDL_JoystickGetBall). */
        virtual bool GetJoystickBall(SDL_Joystick* joystick, int ball, int* dx, int* dy) = 0;

        /**
         * @brief Battery/charge state; fills `*percent` (SDL_JoystickCurrentPowerLevel).
         *
         * @note SDL2's joystick power query (unlike SDL3's `SDL_GetJoystickPowerInfo`) reports
         * only a coarse `SDL_JoystickPowerLevel` (empty/low/medium/full/wired/unknown), never a
         * percentage -- `*percent` is always set to -1 ("unknown"), matching FNA/XNA's own
         * `GamePadState.Battery`'s existing "not always available" contract rather than
         * fabricating a number SDL2 cannot actually provide.
         */
        virtual SDL_PowerState GetJoystickPowerInfo(SDL_Joystick* joystick, int* percent) = 0;
    };

    /**
     * @brief Returns the currently active SDL joystick backend (the real SDL one unless overridden).
     */
    ISdlJoystickBackend& sdl_joystick_backend();
}
