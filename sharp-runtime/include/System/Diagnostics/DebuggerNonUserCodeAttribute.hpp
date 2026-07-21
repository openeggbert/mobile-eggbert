// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System::Diagnostics {

/**
 * @brief Marks a class, struct, method, property, or constructor as not user code, so a debugger skips it when stepping.
 *
 * C++ counterpart of .NET System.Diagnostics.DebuggerNonUserCodeAttribute.
 * A pure marker attribute — carries no state and has no runtime behavior.
 */
class DebuggerNonUserCodeAttribute : public System::Attribute {};

} // namespace System::Diagnostics
