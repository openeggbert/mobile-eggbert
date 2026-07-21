// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/DateTime.hpp"
#include "System/EventArgs.hpp"

namespace System::Timers {

    /**
     * @brief Provides data for the Timer::Elapsed event.
     *
     * C++ counterpart of .NET System.Timers.ElapsedEventArgs.
     */
    class ElapsedEventArgs : public System::EventArgs {
        System::DateTime signalTime_;

    public:
        explicit ElapsedEventArgs(System::DateTime signalTime) : signalTime_(signalTime) {}

        /** @return The time when the timer elapsed. */
        [[nodiscard]] System::DateTime getSignalTimeProperty() const { return signalTime_; }
    };

} // namespace System::Timers
