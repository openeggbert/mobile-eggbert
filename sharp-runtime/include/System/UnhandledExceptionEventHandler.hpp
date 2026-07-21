// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/UnhandledExceptionEventArgs.hpp"

namespace System {

    /**
     * @brief Represents the method that will handle the event raised by an exception
     * that is not handled by the application domain.
     *
     * C++ counterpart of .NET System.UnhandledExceptionEventHandler delegate.
     * The sender is typed as void* (instead of System.Object) because C++ lacks a
     * universal object base class.
     *
     * Signature: @code void handler(void* sender, UnhandledExceptionEventArgs& e); @endcode
     */
    using UnhandledExceptionEventHandler =
        std::function<void(void*, UnhandledExceptionEventArgs&)>;

} // namespace System
