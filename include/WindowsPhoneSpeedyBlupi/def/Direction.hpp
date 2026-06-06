/**
 * @file Direction.hpp
 * @brief Defines the Direction enumeration representing the horizontal facing direction of a game entity.
 *
 * @details Used throughout the gameplay state machine and animation tables to determine
 * sprite flipping and movement direction for Blupi and enemies.
 */

#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using DirectionUnderlying = SharpRuntime::ubytecs;

    /**
     * @brief Horizontal facing direction of the player (Blupi) or an enemy.
     *
     * Used throughout the gameplay state machine and animation tables to determine
     * which way Blupi is facing. Affects sprite flipping and movement direction.
     * Stored as an unsigned byte (ushortcs underlying) to match the original C# layout.
     *
     * @note This is gameplay/logic state, not a rendering flag. Do not use it as
     *       a direct sprite index or SpriteEffects value without the animation table lookup.
     */
    enum class Direction : DirectionUnderlying
    {
        None  = 0,   ///< @brief No direction / uninitialised state.
        Left  = 1,   ///< @brief Facing left (DIR_LEFT).
        Right = 2    ///< @brief Facing right (DIR_RIGHT).
    };

    /**
     * @brief Returns the raw underlying byte value of a Direction.
     * @param[in] direction The direction to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(Direction direction) -> DirectionUnderlying
    {
        return static_cast<DirectionUnderlying>(direction);
    }

    /**
     * @brief Converts an integer to a Direction enum value.
     *
     * Used when loading direction values from data tables or save files.
     * The caller is responsible for ensuring @p value is a valid Direction.
     *
     * @param[in] value Raw integer from original game data (0 = None, 1 = Left, 2 = Right).
     * @return Corresponding Direction enum value.
     */
    static constexpr auto ToDirection(const int value) -> Direction
    {
        return static_cast<Direction>(
            static_cast<DirectionUnderlying>(value)
        );
    }
}

