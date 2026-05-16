
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using DirectionUnderlying = SharpRuntime::ubytecs;
    enum class Direction : SharpRuntime::ushortcs
    {
        None = 0,
        Left  = 1,   // DIR_LEFT
        Right = 2    // DIR_RIGHT
    };

    static constexpr auto ToRaw(Direction direction) -> DirectionUnderlying
    {
        return static_cast<DirectionUnderlying>(direction);
    }

    static constexpr auto ToDirection(const int value) -> Direction
    {
        return static_cast<Direction>(
            static_cast<DirectionUnderlying>(value)
        );
    }
}

