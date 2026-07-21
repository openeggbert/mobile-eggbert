// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>

#include "System/IDisposable.hpp"

namespace System::Threading {

    /**
     * @brief Manages the ambient context flowed across asynchronous control-flow boundaries.
     *
     * C++ counterpart of .NET System.Threading.ExecutionContext. In real .NET, this captures
     * AsyncLocal values, SynchronizationContext, and security state so they flow across
     * await/Task boundaries. This runtime has no async/await machinery to flow across (AsyncLocal
     * is backed directly by real thread-local storage — see AsyncLocal.hpp), so Capture() always
     * returns nullptr (mirroring .NET's own optimization of returning null when there is no
     * interesting ambient state to snapshot) and Run() simply invokes the callback synchronously
     * on the calling thread. This is a deliberate minimal stub, not a silently-wrong approximation:
     * there is no ambient state in this runtime for a non-null ExecutionContext to meaningfully
     * capture.
     */
    class ExecutionContext : public System::IDisposable {
        ExecutionContext() = default;

    public:
        /**
         * @brief RAII-style handle returned by SuppressFlow(), used to undo the suppression.
         *
         * C++ counterpart of .NET System.Threading.AsyncFlowControl. A no-op placeholder in this
         * runtime, consistent with SuppressFlow()/RestoreFlow()/IsFlowSuppressed() all being no-ops
         * here (see class doc comment) — there is no ambient flow state to actually suppress.
         */
        struct AsyncFlowControl {
            /** Undoes the flow suppression (no-op in this runtime). */
            void Undo() {}
        };

        /** Captures the current execution context; always returns nullptr in this runtime (see class doc). */
        [[nodiscard]] static ExecutionContext* Capture() { return nullptr; }

        /** Returns a copy of this execution context. */
        [[nodiscard]] ExecutionContext* CreateCopy() const { return new ExecutionContext(); }

        /** Releases resources used by this ExecutionContext (no-op). */
        void Dispose() override {}

        /** Returns true if ExecutionContext flow is currently suppressed (always false in this runtime). */
        [[nodiscard]] static bool IsFlowSuppressed() { return false; }

        /** Restores the given captured execution context (no-op in this runtime). */
        static void Restore(ExecutionContext* /*executionContext*/) {}

        /** Restores flowing of the execution context after a prior SuppressFlow (no-op in this runtime). */
        static void RestoreFlow() {}

        /** Suppresses execution context flow (no-op in this runtime; see class doc comment). */
        [[nodiscard]] static AsyncFlowControl SuppressFlow() { return AsyncFlowControl{}; }

        /** Runs @p callback with @p state under the given execution context (invoked synchronously in this runtime). */
        static void Run(ExecutionContext* /*executionContext*/, const std::function<void(void*)>& callback, void* state) {
            if (callback) callback(state);
        }
    };

} // namespace System::Threading
