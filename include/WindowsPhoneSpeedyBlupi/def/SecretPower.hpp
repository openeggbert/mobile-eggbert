/**
 * @file SecretPower.hpp
 * @brief Defines the SecretPower enumeration representing the hidden power-up bonus active for Blupi.
 *
 * @details Only one SecretPower can be active at a time. The active value affects collision
 * handling and player capabilities. Values correspond to the SEC_* constants from the original game.
 */

#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SecretPowerUnderlying = SharpRuntime::ubytecs;

    /**
     * @brief Special power-up bonus currently active for the player (Blupi).
     *
     * @details Represents the secret/hidden power-up state obtained by collecting specific
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
        None   = 0,      ///< @brief No special power active.
        Shield = 1,      ///< @brief Temporary invincibility shield (SEC_SHIELD).
        Power  = 2,      ///< @brief Enhanced strength/power bonus (SEC_POWER).
        Cloud  = 3,      ///< @brief Cloud/floating bonus (SEC_CLOUD).
        Hide   = 4       ///< @brief Invisibility bonus (SEC_HIDE).
    };

    /**
     * @brief Returns the raw underlying byte value of a SecretPower.
     * @param[in] power The SecretPower value to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(SecretPower power) -> SecretPowerUnderlying
    {
        return static_cast<SecretPowerUnderlying>(power);
    }

    /**
     * @brief Converts an integer to a SecretPower enum value.
     *
     * @details Used when loading power-up state from data tables or save files.
     * The caller is responsible for ensuring @p value is a valid SecretPower (0..4).
     *
     * @param[in] value Raw integer from original game data.
     * @return Corresponding SecretPower enum value.
     */
    static constexpr auto ToSecretPower(const int value) -> SecretPower
    {
        return static_cast<SecretPower>(
            static_cast<SecretPowerUnderlying>(value)
        );
    }
}
