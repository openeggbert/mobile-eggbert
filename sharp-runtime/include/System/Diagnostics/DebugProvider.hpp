// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdlib>
#include <iostream>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Diagnostics {

    using SharpRuntime::intcs;

/**
 * @brief Provides the default implementation for the Write and Fail methods of Debug.
 *
 * C++ counterpart of .NET System.Diagnostics.DebugProvider.
 * Override Write/WriteLine/Fail/OnIndentLevelChanged/OnIndentSizeChanged and install the
 * override via Debug::SetProvider() to redirect where Debug output goes (e.g. to an
 * on-screen console instead of stdout), matching .NET's Debug.SetProvider extensibility hook.
 */
class DebugProvider {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~DebugProvider() = default;

    /**
     * @brief Writes @p message to stdout, with no trailing newline.
     *
     * C++ counterpart of .NET DebugProvider.Write(string).
     * @param message The text to write.
     */
    virtual void Write(const std::string& message) { std::cout << message; }

    /**
     * @brief Writes @p message followed by a newline.
     *
     * C++ counterpart of .NET DebugProvider.WriteLine(string).
     * @param message The text to write.
     */
    virtual void WriteLine(const std::string& message) { Write(message + "\n"); }

    /**
     * @brief Reports an assertion failure: emits @p message/@p detailMessage and aborts.
     *
     * C++ counterpart of .NET DebugProvider.Fail(string, string), which is marked
     * [DoesNotReturn]. Uses std::abort() unconditionally rather than assert(false) -- the
     * latter is a standard no-op when built with NDEBUG defined, which would silently let
     * this method return instead of terminating, breaking its documented never-returns
     * contract for any release build of this library.
     * @param message       The primary failure message.
     * @param detailMessage Additional detail about the failure.
     */
    virtual void Fail(const std::string& message, const std::string& detailMessage) {
        std::cerr << "Assertion failed: " << message;
        if (!detailMessage.empty()) std::cerr << "\n" << detailMessage;
        std::cerr << std::endl;
        std::abort();
    }

    /**
     * @brief Invoked when Debug::IndentLevel changes; override to react to indent changes.
     *
     * C++ counterpart of .NET DebugProvider.OnIndentLevelChanged(int).
     * @param indentLevel The new indent level.
     */
    virtual void OnIndentLevelChanged(intcs indentLevel) { (void)indentLevel; }

    /**
     * @brief Invoked when Debug::IndentSize changes; override to react to indent-size changes.
     *
     * C++ counterpart of .NET DebugProvider.OnIndentSizeChanged(int).
     * @param indentSize The new indent size.
     */
    virtual void OnIndentSizeChanged(intcs indentSize) { (void)indentSize; }
};

} // namespace System::Diagnostics
