
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using KeyPressFlagsUnderlying = SharpRuntime::ubytecs;
    enum class KeyPressFlags : SharpRuntime::ushortcs
    {
        None  = 0,
        Jump  = 1,   // KEY_JUMP
        Fire  = 2,   // KEY_FIRE
        Down  = 4    // KEY_DOWN
    };

    static constexpr auto ToRaw(KeyPressFlags KeyPressFlags) -> KeyPressFlagsUnderlying
    {
        return static_cast<KeyPressFlagsUnderlying>(KeyPressFlags);
    }

    static constexpr auto ToKeyPressFlags(const int value) -> KeyPressFlags
    {
        return static_cast<KeyPressFlags>(
            static_cast<KeyPressFlagsUnderlying>(value)
        );
    }
}

