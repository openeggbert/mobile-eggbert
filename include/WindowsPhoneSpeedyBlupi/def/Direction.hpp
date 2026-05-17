
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
    enum class Direction : SharpRuntime::ushortcs
    {
        None  = 0,
        Left  = 1,   ///< Facing left (DIR_LEFT).
        Right = 2    ///< Facing right (DIR_RIGHT).
    };

    /**
     * @brief Returns the raw underlying byte value of a Direction.
     * @param direction The direction to convert.
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
     * @param value Raw integer from original game data.
     * @return Corresponding Direction enum value.
     */
    static constexpr auto ToDirection(const int value) -> Direction
    {
        return static_cast<Direction>(
            static_cast<DirectionUnderlying>(value)
        );
    }
}

