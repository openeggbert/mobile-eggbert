// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System::Diagnostics {

/**
 * @brief Marks a class, struct, method, or constructor that a debugger should step through rather than into.
 *
 * C++ counterpart of .NET System.Diagnostics.DebuggerStepThroughAttribute.
 * A pure marker attribute — carries no state and has no runtime behavior.
 */
class DebuggerStepThroughAttribute : public System::Attribute {};

} // namespace System::Diagnostics
