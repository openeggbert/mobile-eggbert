// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/IDisposable.hpp"
#include "System/TimeSpan.hpp"

namespace System::Threading {

    /**
     * @brief Provides a mechanism for changing the period and due time of a timer.
     * C++ counterpart of .NET System.Threading.ITimer.
     */
    class ITimer : public System::IDisposable {
    public:
        virtual ~ITimer() = default;
        /** Changes the start time and interval between callbacks. Returns false if the timer has been disposed. */
        virtual bool Change(TimeSpan dueTime, TimeSpan period) = 0;
    };

} // namespace System::Threading
