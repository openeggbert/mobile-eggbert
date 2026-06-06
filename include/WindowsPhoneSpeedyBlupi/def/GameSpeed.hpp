/**
 * @file GameSpeed.hpp
 * @brief Defines the GameSpeed enumeration and associated helpers for controlling simulation speed.
 *
 * @details GameSpeed is a multiplier-style enum (values 0, 1, 2, 4, 8) that governs how many
 * simulation ticks are executed per rendered frame. Comparison operators allow speed levels to
 * be ordered. Keyboard shortcuts F5-F8 map directly to speed presets.
 */

#pragma once
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using GameSpeedUnderlying = SharpRuntime::ubytecs;

    /**
     * @brief Simulation speed multiplier for the game loop.
     *
     * @details Each value controls how many update ticks are run per rendered frame.
     * Values are powers of two (except Slow=0 and Normal=1) so that speed changes
     * are uniform multipliers. The active speed is set via IGame1::SetGameSpeed().
     *
     * @note Only available in MODERN builds (not LEGACY). Values must not be used as
     *       array indices without explicit mapping.
     * @note Stored as an unsigned byte (ubytecs) to match the original C# enum layout.
     */
    enum class GameSpeed : GameSpeedUnderlying
    {
        Slow    = 0,  ///< @brief Reduced speed (0 extra ticks per frame).
        Normal  = 1,  ///< @brief Standard game speed (1 tick per frame, default).
        Fast    = 2,  ///< @brief Double speed (2 ticks per frame).
        Faster  = 4,  ///< @brief Quadruple speed (4 ticks per frame).
        Fastest = 8   ///< @brief Maximum speed (8 ticks per frame).
    };

    /**
     * @brief Returns the raw underlying byte value of a GameSpeed.
     * @param[in] speed The GameSpeed value to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(GameSpeed speed) -> GameSpeedUnderlying
    {
        return static_cast<GameSpeedUnderlying>(speed);
    }

    /**
     * @brief Less-than comparison for GameSpeed values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return True if @p lhs is strictly slower than @p rhs.
     */
    constexpr bool operator<(GameSpeed lhs, GameSpeed rhs) noexcept
    {
        return static_cast<GameSpeedUnderlying>(lhs) <
               static_cast<GameSpeedUnderlying>(rhs);
    }

    /**
     * @brief Less-than-or-equal comparison for GameSpeed values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return True if @p lhs is slower than or equal to @p rhs.
     */
    constexpr bool operator<=(GameSpeed lhs, GameSpeed rhs) noexcept
    {
        return static_cast<GameSpeedUnderlying>(lhs) <=
               static_cast<GameSpeedUnderlying>(rhs);
    }

    /**
     * @brief Greater-than comparison for GameSpeed values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return True if @p lhs is strictly faster than @p rhs.
     */
    constexpr bool operator>(GameSpeed lhs, GameSpeed rhs) noexcept
    {
        return static_cast<GameSpeedUnderlying>(lhs) >
               static_cast<GameSpeedUnderlying>(rhs);
    }

    /**
     * @brief Greater-than-or-equal comparison for GameSpeed values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return True if @p lhs is faster than or equal to @p rhs.
     */
    constexpr bool operator>=(GameSpeed lhs, GameSpeed rhs) noexcept
    {
        return static_cast<GameSpeedUnderlying>(lhs) >=
               static_cast<GameSpeedUnderlying>(rhs);
    }

    /**
     * @brief Converts an integer to a GameSpeed enum value.
     *
     * @details Used when loading GameSpeed values from data tables or save files.
     * The caller is responsible for ensuring @p value is a valid GameSpeed (0, 1, 2, 4, or 8).
     *
     * @param[in] value Raw integer from original game data.
     * @return Corresponding GameSpeed enum value.
     */
    static constexpr auto ToGameSpeed(const int value) -> GameSpeed
    {
        return static_cast<GameSpeed>(
            static_cast<GameSpeedUnderlying>(value)
        );
    }

    /**
     * @brief Converts a function-key code to a GameSpeed preset.
     *
     * @details Maps the cheat/debug keyboard shortcuts to their corresponding speed levels:
     * F5 = Normal, F6 = Fast, F7 = Faster, F8 = Fastest. Any other key returns Normal.
     *
     * @param[in] key The XNA/CNA keyboard key code to map.
     * @return The GameSpeed preset associated with @p key, or GameSpeed::Normal if unknown.
     */
    static constexpr auto ToGameSpeed(const Microsoft::Xna::Framework::Input::Keys key) -> GameSpeed
    {
        switch (key)
        {
            case Microsoft::Xna::Framework::Input::Keys::F5:
                return GameSpeed::Normal;
            case Microsoft::Xna::Framework::Input::Keys::F6:
                return GameSpeed::Fast;
            case Microsoft::Xna::Framework::Input::Keys::F7:
                return GameSpeed::Faster;
            case Microsoft::Xna::Framework::Input::Keys::F8:
                return GameSpeed::Fastest;
            default:
                return GameSpeed::Normal;
        }
    }
}
