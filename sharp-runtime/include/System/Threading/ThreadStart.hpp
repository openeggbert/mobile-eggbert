// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>

namespace System::Threading {

    /** @brief Represents a method that executes on a Thread with no parameters. */
    using ThreadStart              = std::function<void()>;
    /** @brief Represents a method that executes on a Thread and receives a single void* parameter. */
    using ParameterizedThreadStart = std::function<void(void*)>;

    // Forward declaration for ThreadExceptionEventHandler
    class ThreadExceptionEventArgs;

    /**
     * @brief Represents the method that will handle the event raised when a thread
     * encounters an unhandled exception.
     *
     * C++ counterpart of .NET System.Threading.ThreadExceptionEventHandler delegate.
     * @param sender The source of the event.
     * @param e      A ThreadExceptionEventArgs containing the unhandled exception.
     */
    using ThreadExceptionEventHandler = std::function<void(void* sender, ThreadExceptionEventArgs& e)>;

} // namespace System::Threading
