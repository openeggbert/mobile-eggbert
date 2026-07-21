// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Attribute.hpp"

namespace System::Diagnostics {
    /**
     * @brief Marks a type or method to be omitted from the stack trace text shown in
     *        StackTrace.ToString() and Exception.StackTrace.
     *
     * C++ counterpart of .NET System.Diagnostics.StackTraceHiddenAttribute. This port has no
     * reflection/runtime attribute metadata (see CLAUDE.md's documented permanent deviation), so
     * this attribute carries no runtime behavior here -- it exists purely as a marker type for
     * source-level parity with ported .NET code.
     */
    class StackTraceHiddenAttribute : public System::Attribute {};
} // namespace System::Diagnostics
