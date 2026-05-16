
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SecretPowerUnderlying = SharpRuntime::ubytecs;
    enum class SecretPower : SharpRuntime::ushortcs
    {
        None   = 0,
        Shield = 1,   // SEC_SHIELD
        Power  = 2,   // SEC_POWER
        Cloud  = 3,   // SEC_CLOUD
        Hide   = 4    // SEC_HIDE
    };

    static constexpr auto ToRaw(SecretPower SecretPower) -> SecretPowerUnderlying
    {
        return static_cast<SecretPowerUnderlying>(SecretPower);
    }

    static constexpr auto ToSecretPower(const int value) -> SecretPower
    {
        return static_cast<SecretPower>(
            static_cast<SecretPowerUnderlying>(value)
        );
    }
}

