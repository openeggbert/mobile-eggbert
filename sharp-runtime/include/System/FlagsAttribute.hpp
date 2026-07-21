// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System {
    /**
     * @brief Indicates that an enumeration can be treated as a bit field; that is,
     * a set of flags.
     *
     * C++ counterpart of .NET System.FlagsAttribute.
     */
    class FlagsAttribute : public Attribute {};
} // namespace System
