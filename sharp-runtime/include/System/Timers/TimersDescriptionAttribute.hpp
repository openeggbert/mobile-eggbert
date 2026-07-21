// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/ComponentModel/DescriptionAttribute.hpp"

namespace System::Timers {

    /**
     * @brief Marks a property, event, or extender of System.Timers.Timer with a description for
     * visual designers.
     *
     * C++ counterpart of .NET System.Timers.TimersDescriptionAttribute.
     *
     * @note The internal `TimersDescriptionAttribute(TimersDescriptionStringId)` constructor
     * (resource-string lookup for the built-in Timer properties) is not reproduced — it was
     * `internal` in .NET (not part of the public API surface) and only used by attributes on
     * Timer's own members, which have no reflection-driven consumer in this runtime anyway.
     */
    struct TimersDescriptionAttribute : System::ComponentModel::DescriptionAttribute {
        explicit TimersDescriptionAttribute(const std::string& description) : DescriptionAttribute(description) {}
    };

} // namespace System::Timers
