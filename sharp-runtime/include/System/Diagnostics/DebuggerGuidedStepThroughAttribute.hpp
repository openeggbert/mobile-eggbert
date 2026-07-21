// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System::Diagnostics {

/**
 * @brief Marks a method, property, constructor, or assembly for guided step-through debugging.
 *
 * C++ counterpart of .NET System.Diagnostics.DebuggerGuidedStepThroughAttribute.
 * A pure marker attribute — carries no state and has no runtime behavior.
 */
class DebuggerGuidedStepThroughAttribute : public System::Attribute {};

} // namespace System::Diagnostics
