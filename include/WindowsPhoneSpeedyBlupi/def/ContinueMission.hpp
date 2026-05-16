#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using ContinueMissionTypeUnderlying = SharpRuntime::ubytecs;
    enum class ContinueMissionType : SharpRuntime::ushortcs
    {
        None    = 0,
        Pending = 1,
        Active  = 2
    };

    static constexpr auto ToRaw(ContinueMissionType ContinueMissionType) -> ContinueMissionTypeUnderlying
    {
        return static_cast<ContinueMissionTypeUnderlying>(ContinueMissionType);
    }

    static constexpr auto ToContinueMissionType(const int value) -> ContinueMissionType
    {
        return static_cast<ContinueMissionType>(
            static_cast<ContinueMissionTypeUnderlying>(value)
        );
    }
}

