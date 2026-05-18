#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using DecorActionUnderlying = SharpRuntime::ubytecs;
    enum class DecorAction : DecorActionUnderlying
    {
        None = 0,
        SmallShake = 1,
        BigShake = 2,
        ElectricShake = 5,
    };

    static constexpr auto ToRaw(DecorAction type) -> DecorActionUnderlying
    {
        return static_cast<DecorActionUnderlying>(type);
    }

    static constexpr auto ToDecorAction(const int value) -> DecorAction
    {
        return static_cast<DecorAction>(
            static_cast<DecorActionUnderlying>(value)
        );
    }

    constexpr bool operator<(const DecorAction lhs, const DecorAction rhs) noexcept
    {
        return ToRaw(lhs) < ToRaw(rhs);
    }

    constexpr bool operator>(const DecorAction lhs, const DecorAction rhs) noexcept
    {
        return ToRaw(lhs) > ToRaw(rhs);
    }

    constexpr bool operator<=(const DecorAction lhs, const DecorAction rhs) noexcept
    {
        return ToRaw(lhs) <= ToRaw(rhs);
    }

    constexpr bool operator>=(const DecorAction lhs, const DecorAction rhs) noexcept
    {
        return ToRaw(lhs) >= ToRaw(rhs);
    }
}
