// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {
    /**
     * @brief Indicates that the value of a static field is unique for each thread.
     *
     * C++ counterpart of .NET System.ThreadStaticAttribute.
     */
    class ThreadStaticAttribute : public Attribute {};
} // namespace System
