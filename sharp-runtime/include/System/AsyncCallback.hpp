// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/IAsyncResult.hpp"

namespace System {

    /**
     * @brief References a method to be called when a corresponding asynchronous
     * operation completes.
     *
     * C++ counterpart of .NET System.AsyncCallback delegate.
     * Implemented as a std::function alias — any callable with signature
     * @c void(IAsyncResult&) is compatible.
     */
    using AsyncCallback = std::function<void(IAsyncResult&)>;

} // namespace System
