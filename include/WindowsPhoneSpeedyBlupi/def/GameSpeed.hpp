
#pragma once
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using GameSpeedUnderlying = SharpRuntime::ubytecs;

    enum class GameSpeed : GameSpeedUnderlying
    {
        Slow = 0,
        Normal = 1,
        Fast = 2,
        Faster = 4,
        Fastest = 8
    };

    /**
     * @brief Returns the raw underlying byte value of a GameSpeed.
     * @param GameSpeed The GameSpeed to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(GameSpeed GameSpeed) -> GameSpeedUnderlying
    {
        return static_cast<GameSpeedUnderlying>(GameSpeed);
    }


    constexpr bool operator<(GameSpeed lhs, GameSpeed rhs) noexcept
    {
        return static_cast<GameSpeedUnderlying>(lhs) <
               static_cast<GameSpeedUnderlying>(rhs);
    }

    constexpr bool operator<=(GameSpeed lhs, GameSpeed rhs) noexcept
    {
        return static_cast<GameSpeedUnderlying>(lhs) <=
               static_cast<GameSpeedUnderlying>(rhs);
    }

    constexpr bool operator>(GameSpeed lhs, GameSpeed rhs) noexcept
    {
        return static_cast<GameSpeedUnderlying>(lhs) >
               static_cast<GameSpeedUnderlying>(rhs);
    }

    constexpr bool operator>=(GameSpeed lhs, GameSpeed rhs) noexcept
    {
        return static_cast<GameSpeedUnderlying>(lhs) >=
               static_cast<GameSpeedUnderlying>(rhs);
    }

    /**
     * @brief Converts an integer to a GameSpeed enum value.
     *
     * Used when loading GameSpeed values from data tables or save files.
     * The caller is responsible for ensuring @p value is a valid GameSpeed.
     *
     * @param value Raw integer from original game data.
     * @return Corresponding GameSpeed enum value.
     */
    static constexpr auto ToGameSpeed(const int value) -> GameSpeed
    {
        return static_cast<GameSpeed>(
            static_cast<GameSpeedUnderlying>(value)
        );
    }
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

