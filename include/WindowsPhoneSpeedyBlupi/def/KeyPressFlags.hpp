
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using KeyPressFlagsUnderlying = SharpRuntime::ubytecs;

    /**
     * @brief Bitmask of currently pressed virtual game buttons.
     *
     * Represents the set of logical game actions that are active during a given
     * update frame. Values are powers of two so they can be combined with bitwise OR.
     *
     * These flags are produced by InputPad from touch/keyboard/accelerometer input
     * and consumed by the gameplay logic in Decor to drive Blupi's state machine.
     *
     * @note This is logical input state, not raw keyboard/touch state. The mapping
     *       from physical input to KeyPressFlags is handled in InputPad.
     * @note Stored as an unsigned byte (ubytecs) to match the original C# enum layout.
     */
    enum class KeyPressFlags : SharpRuntime::ushortcs
    {
        None  = 0,      ///< No buttons pressed.
        Jump  = 1,      ///< Jump button active (KEY_JUMP).
        Fire  = 2,      ///< Fire/action button active (KEY_FIRE).
        Down  = 4       ///< Down button active (KEY_DOWN).
    };

    /**
     * @brief Returns the raw underlying byte value of a KeyPressFlags value.
     * @param KeyPressFlags The flags value to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(KeyPressFlags KeyPressFlags) -> KeyPressFlagsUnderlying
    {
        return static_cast<KeyPressFlagsUnderlying>(KeyPressFlags);
    }

    /**
     * @brief Converts an integer to a KeyPressFlags enum value.
     *
     * Used when reconstructing input state from raw data. The caller is responsible
     * for ensuring @p value only contains valid flag bits.
     *
     * @param value Raw integer bitmask.
     * @return Corresponding KeyPressFlags enum value.
     */
    static constexpr auto ToKeyPressFlags(const int value) -> KeyPressFlags
    {
        return static_cast<KeyPressFlags>(
            static_cast<KeyPressFlagsUnderlying>(value)
        );
    }
}

