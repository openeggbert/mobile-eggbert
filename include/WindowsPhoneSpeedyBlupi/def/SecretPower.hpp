
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SecretPowerUnderlying = SharpRuntime::ubytecs;

    /**
     * @brief Special power-up bonus currently active for the player (Blupi).
     *
     * Represents the secret/hidden power-up state obtained by collecting specific
     * items in the level. Only one SecretPower can be active at a time; None
     * indicates no bonus is currently in effect.
     *
     * These values correspond to the SEC_* constants from the original game.
     * The active SecretPower affects collision handling and Blupi's capabilities
     * (e.g., Shield makes Blupi temporarily invincible).
     *
     * @note This is gameplay state. Do not use these values as sprite indices.
     * @note Stored as an unsigned byte (ubytecs) to match the original C# enum layout.
     */
    enum class SecretPower : SharpRuntime::ushortcs
    {
        None   = 0,      ///< No special power active.
        Shield = 1,      ///< Temporary invincibility shield (SEC_SHIELD).
        Power  = 2,      ///< Enhanced strength/power bonus (SEC_POWER).
        Cloud  = 3,      ///< Cloud/floating bonus (SEC_CLOUD).
        Hide   = 4       ///< Invisibility bonus (SEC_HIDE).
    };

    /**
     * @brief Returns the raw underlying byte value of a SecretPower.
     * @param SecretPower The power to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(SecretPower SecretPower) -> SecretPowerUnderlying
    {
        return static_cast<SecretPowerUnderlying>(SecretPower);
    }

    /**
     * @brief Converts an integer to a SecretPower enum value.
     *
     * Used when loading power-up state from data tables or save files.
     * The caller is responsible for ensuring @p value is a valid SecretPower.
     *
     * @param value Raw integer from original game data.
     * @return Corresponding SecretPower enum value.
     */
    static constexpr auto ToSecretPower(const int value) -> SecretPower
    {
        return static_cast<SecretPower>(
            static_cast<SecretPowerUnderlying>(value)
        );
    }
}

