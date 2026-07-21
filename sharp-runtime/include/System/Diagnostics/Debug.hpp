// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/Diagnostics/DebugProvider.hpp"

namespace System::Diagnostics {

    using SharpRuntime::intcs;

    /**
     * @brief Provides a set of methods and properties that help debug code.
     *
     * Partial C++ counterpart of .NET System.Diagnostics.Debug.
     * In Release builds the Assert/Write/WriteLine methods are compiled out.
     *
     * @note Status: Partial. Output is routed through a pluggable DebugProvider
     * (matching .NET's Debug.SetProvider extensibility), and IndentLevel/IndentSize
     * are tracked and reported to the provider. Unlike .NET, Write/WriteLine do not
     * automatically prefix output with the current indent — callers that need indented
     * output should build the prefix themselves from getIndentLevelProperty()/getIndentSizeProperty().
     */
    class Debug {
        static std::shared_ptr<DebugProvider>& providerStorage() {
            static std::shared_ptr<DebugProvider> provider = std::make_shared<DebugProvider>();
            return provider;
        }

        static intcs& indentSizeStorage() {
            static intcs size = 4;
            return size;
        }

        static intcs& indentLevelStorage() {
            static thread_local intcs level = 0;
            return level;
        }

    public:
        /** @brief Not instantiable — all members are static. */
        Debug() = delete;

        /**
         * @brief Gets the provider currently used to route Write/WriteLine/Fail output.
         *
         * C++ counterpart of .NET Debug.GetProvider().
         * @return The active DebugProvider.
         */
        static std::shared_ptr<DebugProvider> GetProvider() { return providerStorage(); }

        /**
         * @brief Installs a new provider to route Write/WriteLine/Fail output, returning the previous one.
         *
         * C++ counterpart of .NET Debug.SetProvider(DebugProvider).
         * @param provider The new provider to install.
         * @return The previously installed provider.
         * @throws System::ArgumentNullException if @p provider is null.
         */
        static std::shared_ptr<DebugProvider> SetProvider(std::shared_ptr<DebugProvider> provider) {
            if (!provider)
                throw System::ArgumentNullException("provider");
            std::shared_ptr<DebugProvider> previous = providerStorage();
            providerStorage() = std::move(provider);
            return previous;
        }

        /**
         * @brief Gets the amount by which the indent is increased for each IndentLevel step.
         *
         * C++ counterpart of .NET Debug.IndentSize.
         * @return The current indent size, in characters.
         */
        static intcs getIndentSizeProperty() { return indentSizeStorage(); }

        /**
         * @brief Sets the amount by which the indent is increased for each IndentLevel step.
         *
         * C++ counterpart of .NET Debug.IndentSize.
         * @param value The new indent size; negative values are clamped to 0.
         */
        static void setIndentSizeProperty(intcs value) {
            indentSizeStorage() = value < 0 ? 0 : value;
            providerStorage()->OnIndentSizeChanged(indentSizeStorage());
        }

        /**
         * @brief Gets the current indent level (per-thread), matching .NET's ThreadStatic IndentLevel.
         *
         * C++ counterpart of .NET Debug.IndentLevel.
         * @return The current indent level.
         */
        static intcs getIndentLevelProperty() { return indentLevelStorage(); }

        /**
         * @brief Sets the current indent level (per-thread).
         *
         * C++ counterpart of .NET Debug.IndentLevel.
         * @param value The new indent level; negative values are clamped to 0.
         */
        static void setIndentLevelProperty(intcs value) {
            indentLevelStorage() = value < 0 ? 0 : value;
            providerStorage()->OnIndentLevelChanged(indentLevelStorage());
        }

        /**
         * @brief Increases the current indent level by one.
         *
         * C++ counterpart of .NET Debug.Indent().
         */
        static void Indent() { setIndentLevelProperty(getIndentLevelProperty() + 1); }

        /**
         * @brief Decreases the current indent level by one.
         *
         * C++ counterpart of .NET Debug.Unindent().
         */
        static void Unindent() { setIndentLevelProperty(getIndentLevelProperty() - 1); }

#ifdef NDEBUG
        /** @brief No-op in release builds. */
        static void Assert(bool condition)                            {}
        /** @brief No-op in release builds. */
        static void Assert(bool condition, const char*)               {}
        /** @brief No-op in release builds. */
        static void Assert(bool condition, const std::string&)        {}
        /** @brief No-op in release builds. */
        static void Write(const char*)                                {}
        /** @brief No-op in release builds. */
        static void Write(const std::string&)                         {}
        /** @brief No-op in release builds. */
        static void WriteLine(const char*)                            {}
        /** @brief No-op in release builds. */
        static void WriteLine(const std::string&)                     {}
        /** @brief No-op in release builds. */
        static void WriteLine()                                       {}
        /** @brief No-op in release builds. */
        static void Fail(const char*)                                 {}
        /** @brief No-op in release builds. */
        static void Fail(const std::string&)                          {}
#else
        /**
         * @brief Checks @p condition; reports failure via the active DebugProvider if false.
         *
         * Routes through the same DebugProvider mechanism as the message-carrying overloads
         * below, instead of calling the raw assert() macro directly -- a custom provider
         * installed via SetProvider() must be invoked for this overload too, matching this
         * class's own doc comment ("output is routed through a pluggable DebugProvider").
         * @param condition The expression that is expected to be true.
         */
        static void Assert(bool condition) {
            Assert(condition, "");
        }
        /**
         * @brief Checks @p condition; reports failure via the active DebugProvider if false.
         * @param condition The expression that is expected to be true.
         * @param message   Diagnostic message emitted on failure.
         */
        static void Assert(bool condition, const char* message) {
            if (!condition) {
                Fail(message);
            }
        }
        /**
         * @brief Checks @p condition; reports failure via the active DebugProvider if false.
         * @param condition The expression that is expected to be true.
         * @param message   Diagnostic message emitted on failure.
         */
        static void Assert(bool condition, const std::string& message) {
            Assert(condition, message.c_str());
        }

        /** @brief Writes @p message via the active DebugProvider, without a trailing newline. */
        static void Write(const char* message)        { providerStorage()->Write(message); }
        /** @brief Writes @p message via the active DebugProvider, without a trailing newline. */
        static void Write(const std::string& message) { providerStorage()->Write(message); }

        /** @brief Writes a blank line via the active DebugProvider. */
        static void WriteLine()                        { providerStorage()->WriteLine(""); }
        /** @brief Writes @p message followed by a newline via the active DebugProvider. */
        static void WriteLine(const char* message)    { providerStorage()->WriteLine(message); }
        /** @brief Writes @p msg followed by a newline via the active DebugProvider. */
        static void WriteLine(const std::string& msg) { providerStorage()->WriteLine(msg); }

        /**
         * @brief Reports an assertion failure via the active DebugProvider (aborts by default).
         * @param message Failure description.
         */
        static void Fail(const char* message) {
            providerStorage()->Fail(message, "");
        }
        /** @brief Reports an assertion failure via the active DebugProvider (aborts by default). */
        static void Fail(const std::string& message) { Fail(message.c_str()); }
#endif
    };

} // namespace System::Diagnostics
