// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)

#include "System/DateTime.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include <chrono>

namespace System {

    // -------------------------------------------------------------------------
    // Constructors
    // -------------------------------------------------------------------------

    DateTime::DateTime()
        : ticks_(0) {}

    DateTime::DateTime(longcs ticks)
        : ticks_(ticks) {
        if (ticks < 0 || ticks > MaxTicks)
            throw System::ArgumentOutOfRangeException("ticks", "Ticks must be between DateTime.MinValue.Ticks and DateTime.MaxValue.Ticks.");
    }

    // -------------------------------------------------------------------------
    // Properties — tick-level decomposition
    // -------------------------------------------------------------------------

    longcs DateTime::getTicksProperty() const { return ticks_; }

    // -------------------------------------------------------------------------
    // Static factories
    // -------------------------------------------------------------------------

    DateTime DateTime::getNowProperty() {
        using namespace std::chrono;
        const auto now      = system_clock::now();
        const auto duration = now.time_since_epoch();
        const auto secs     = duration_cast<seconds>(duration);
        const auto sub      = duration - secs;
        const longcs ticks  = UnixEpochTicks
            + static_cast<longcs>(secs.count()) * TicksPerSecond
            + static_cast<longcs>(duration_cast<nanoseconds>(sub).count() / 100);
        return DateTime(ticks);
    }

    GetTypeNameCPP(DateTime, "System.DateTime")
}
