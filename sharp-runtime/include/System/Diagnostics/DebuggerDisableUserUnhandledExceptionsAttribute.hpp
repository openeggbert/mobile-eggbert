// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System::Diagnostics {

/**
 * @brief Prevents an attached debugger from breaking on user-unhandled exceptions caught by a method with this attribute.
 *
 * C++ counterpart of .NET System.Diagnostics.DebuggerDisableUserUnhandledExceptionsAttribute.
 * A pure marker attribute — carries no state and has no runtime behavior.
 */
class DebuggerDisableUserUnhandledExceptionsAttribute : public System::Attribute {};

} // namespace System::Diagnostics
