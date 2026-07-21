// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System::Diagnostics {

/**
 * @brief Indicates the code following the attribute is to be executed in run, not step, mode.
 *
 * C++ counterpart of .NET System.Diagnostics.DebuggerStepperBoundaryAttribute.
 * A pure marker attribute — carries no state and has no runtime behavior.
 */
class DebuggerStepperBoundaryAttribute : public System::Attribute {};

} // namespace System::Diagnostics
