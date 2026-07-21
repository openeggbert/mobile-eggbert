// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <iostream>
#include <string>

namespace System::Diagnostics {

    /**
     * @brief Provides a set of methods that help you trace code execution.
     *
     * Unlike Debug, Trace output is NOT stripped in release builds.
     * Partial C++ counterpart of .NET System.Diagnostics.Trace.
     *
     * @note Status: Partial — writes to std::cerr; no TraceListeners/TraceListenerCollection,
     * no CorrelationManager, no Refreshing event, no WriteIf/WriteLineIf family, no
     * category-suffixed Write/WriteLine overloads, and no object-typed Write/WriteLine
     * overloads. Real .NET's Trace and Debug share the same underlying static IndentLevel
     * state (both delegate to an internal TraceInternal class); this port keeps them as
     * separate, independently-tracked indent levels since Debug's provider-hook design isn't
     * shared here -- a documented, intentional simplification rather than a bug.
     */
    class Trace {
        static int& indentSizeStorage() {
            static int size = 4;
            return size;
        }

        static int& indentLevelStorage() {
            static thread_local int level = 0;
            return level;
        }

    public:
        /** @brief Not instantiable — all members are static. */
        Trace() = delete;

        /**
         * @brief Gets the amount by which the indent is increased for each IndentLevel step.
         * C++ counterpart of .NET Trace.IndentSize.
         */
        static int getIndentSizeProperty() { return indentSizeStorage(); }

        /**
         * @brief Sets the amount by which the indent is increased for each IndentLevel step.
         * C++ counterpart of .NET Trace.IndentSize.
         * @param value The new indent size; negative values are clamped to 0.
         */
        static void setIndentSizeProperty(int value) {
            indentSizeStorage() = value < 0 ? 0 : value;
        }

        /**
         * @brief Gets the current indent level (per-thread), matching .NET's ThreadStatic IndentLevel.
         * C++ counterpart of .NET Trace.IndentLevel.
         */
        static int getIndentLevelProperty() { return indentLevelStorage(); }

        /**
         * @brief Sets the current indent level (per-thread).
         * C++ counterpart of .NET Trace.IndentLevel.
         * @param value The new indent level; negative values are clamped to 0.
         */
        static void setIndentLevelProperty(int value) {
            indentLevelStorage() = value < 0 ? 0 : value;
        }

        /** @brief Increases the current indent level by one. C++ counterpart of .NET Trace.Indent(). */
        static void Indent() { setIndentLevelProperty(getIndentLevelProperty() + 1); }

        /** @brief Decreases the current indent level by one. C++ counterpart of .NET Trace.Unindent(). */
        static void Unindent() { setIndentLevelProperty(getIndentLevelProperty() - 1); }

        /** @brief Writes @p message to stderr without a trailing newline. */
        static void Write(const std::string& message)   { std::cerr << message; }
        /** @brief Writes @p message to stderr followed by a newline. */
        static void WriteLine(const std::string& message) { std::cerr << message << '\n'; }
        /** @brief Writes a blank line to stderr. */
        static void WriteLine()                         { std::cerr << '\n'; }

        /** @brief Writes an informational @p message prefixed with [Info]. */
        static void TraceInformation(const std::string& message) {
            std::cerr << "[Info]  " << message << '\n';
        }
        /** @brief Writes a warning @p message prefixed with [Warn]. */
        static void TraceWarning(const std::string& message) {
            std::cerr << "[Warn]  " << message << '\n';
        }
        /** @brief Writes an error @p message prefixed with [Error]. */
        static void TraceError(const std::string& message) {
            std::cerr << "[Error] " << message << '\n';
        }

        /**
         * @brief Writes to stderr if @p condition is false.
         * @param condition The expression that is expected to be true.
         * @param message   Optional message appended to the failure line.
         */
        static void Assert(bool condition, const std::string& message = "") {
            if (!condition) {
                std::cerr << "[Trace Assert Failed]";
                if (!message.empty()) std::cerr << ' ' << message;
                std::cerr << '\n';
            }
        }

        /** @brief Writes a failure @p message prefixed with [Trace Fail]. */
        static void Fail(const std::string& message) {
            std::cerr << "[Trace Fail] " << message << '\n';
        }

        /** @brief Flushes the stderr buffer. */
        static void Flush() { std::cerr.flush(); }
    };

} // namespace System::Diagnostics
