// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/Object.hpp"
#include "System/Timers/ElapsedEventArgs.hpp"

namespace System::Timers {

    /**
     * @brief Represents the method that handles the Timer::Elapsed event.
     *
     * C++ counterpart of .NET System.Timers.ElapsedEventHandler.
     */
    using ElapsedEventHandler = std::function<void(System::Object* sender, const ElapsedEventArgs& e)>;

} // namespace System::Timers
