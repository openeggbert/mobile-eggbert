#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using DoorKeyFlagsUnderlying = SharpRuntime::ubytecs;
    enum class DoorKeyFlags : DoorKeyFlagsUnderlying
    {
        None = 0,
        Key1 = 1 << 0,
        Key2 = 1 << 1,
        Key3 = 1 << 2,
        All  = Key1 | Key2 | Key3,
    };

    static constexpr auto ToRaw(DoorKeyFlags type) -> DoorKeyFlagsUnderlying
    {
        return static_cast<DoorKeyFlagsUnderlying>(type);
    }

    static constexpr auto ToDoorKeyFlags(const int value) -> DoorKeyFlags
    {
        return static_cast<DoorKeyFlags>(
            static_cast<DoorKeyFlagsUnderlying>(value)
        );
    }

    constexpr bool operator<(const DoorKeyFlags lhs, const DoorKeyFlags rhs) noexcept
    {
        return ToRaw(lhs) < ToRaw(rhs);
    }

    constexpr bool operator>(const DoorKeyFlags lhs, const DoorKeyFlags rhs) noexcept
    {
        return ToRaw(lhs) > ToRaw(rhs);
    }

    constexpr bool operator<=(const DoorKeyFlags lhs, const DoorKeyFlags rhs) noexcept
    {
        return ToRaw(lhs) <= ToRaw(rhs);
    }

    constexpr bool operator>=(const DoorKeyFlags lhs, const DoorKeyFlags rhs) noexcept
    {
        return ToRaw(lhs) >= ToRaw(rhs);
    }

    constexpr DoorKeyFlags operator|(DoorKeyFlags a, DoorKeyFlags b)
    {
        return static_cast<DoorKeyFlags>(ToRaw(a) | ToRaw(b));
    }

    constexpr DoorKeyFlags operator&(DoorKeyFlags a, DoorKeyFlags b)
    {
        return static_cast<DoorKeyFlags>(ToRaw(a) & ToRaw(b));
    }
}
