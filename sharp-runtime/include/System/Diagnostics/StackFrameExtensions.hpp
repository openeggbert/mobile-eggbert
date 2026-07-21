// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Diagnostics/StackFrame.hpp"
#include "System/IntPtr.hpp"

namespace System::Diagnostics {

    /**
     * @brief Provides extension-like static helper methods for System::Diagnostics::StackFrame.
     *
     * C++ counterpart of .NET System.Diagnostics.StackFrameExtensions. C++ has no extension-method
     * syntax, so .NET's `stackFrame.HasSource()` becomes `StackFrameExtensions::HasSource(stackFrame)`.
     *
     * This runtime has no native image / JIT infrastructure and no runtime reflection, so the
     * native-image and method-reflection members are permanent simplifications rather than gaps:
     * @c GetNativeIP() and @c GetNativeImageBase() always return @c IntPtr::Zero (matching .NET
     * CoreLib's own base implementation), and @c HasMethod() reports whether a method name is known
     * instead of resolving a @c MethodBase via reflection.
     */
    struct StackFrameExtensions {
        StackFrameExtensions() = delete;

        /** @brief Returns true if the frame has a native image associated with it. */
        [[nodiscard]] static bool HasNativeImage(const StackFrame& stackFrame) {
            return GetNativeImageBase(stackFrame) != IntPtr::Zero;
        }

        /** @brief Returns true if method information is available for this frame. */
        [[nodiscard]] static bool HasMethod(const StackFrame& stackFrame) {
            return !stackFrame.getMethodNameProperty().empty();
        }

        /** @brief Returns true if the IL offset is known for this frame. */
        [[nodiscard]] static bool HasILOffset(const StackFrame& stackFrame) {
            return stackFrame.getILOffsetProperty() != StackFrame::OFFSET_UNKNOWN;
        }

        /** @brief Returns true if source file information is available for this frame. */
        [[nodiscard]] static bool HasSource(const StackFrame& stackFrame) {
            return !stackFrame.getFileNameProperty().empty();
        }

        /** @brief Returns the native instruction pointer for this frame. Always IntPtr::Zero in this runtime. */
        [[nodiscard]] static IntPtr GetNativeIP(const StackFrame& stackFrame) {
            (void)stackFrame;
            return IntPtr::Zero;
        }

        /** @brief Returns the native image base for this frame. Always IntPtr::Zero in this runtime. */
        [[nodiscard]] static IntPtr GetNativeImageBase(const StackFrame& stackFrame) {
            (void)stackFrame;
            return IntPtr::Zero;
        }
    };

} // namespace System::Diagnostics
