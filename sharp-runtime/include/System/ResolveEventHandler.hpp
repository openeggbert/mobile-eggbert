// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <string>
#include "System/ResolveEventArgs.hpp"

namespace System {
    /**
     * @brief Represents a method that handles the event for resolving assemblies.
     *
     * C++ counterpart of the .NET System.ResolveEventHandler delegate type.
     * The function signature is: `std::string(void* sender, ResolveEventArgs& args)`.
     */
    using ResolveEventHandler = std::function<std::string(void*, ResolveEventArgs&)>;
} // namespace System
