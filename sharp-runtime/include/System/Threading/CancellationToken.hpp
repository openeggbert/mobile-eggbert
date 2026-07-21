// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Threading {

    using SharpRuntime::intcs;

    /** @brief Internal shared cancellation state; not part of the public API. */
    namespace Detail {
        struct CancellationState {
            std::atomic<bool> cancelled{false};
            std::mutex mutex;
            // Ordered (not unordered_map) so Cancel() can walk callbacks in reverse key order.
            // nextId is monotonically increasing per Register() call, so key order == registration
            // order and reverse key order == LIFO (most-recently-registered fires first), matching
            // CancellationTokenSource.cs's ExecuteCallbackHandlers.
            std::map<intcs, std::function<void()>> callbacks;
            intcs nextId = 0;
            // Verified against CancellationTokenSource.cs's ExecutingCallbackId/
            // ThreadIDExecutingCallbacks and CancellationTokenRegistration.cs's
            // WaitForCallbackIfNecessary: while Cancel() is invoking a callback, its id and
            // executing thread are recorded here (under `mutex`) so a concurrent
            // CancellationTokenRegistration::Dispose() for that same id can wait for it to
            // finish instead of racing a caller's post-Dispose() teardown against the
            // still-running callback (see CancellationTokenRegistration.hpp's Dispose()).
            intcs executingId = -1;
            std::thread::id executingThreadId{};
            std::condition_variable callbackFinished;
        };
    }

    class CancellationTokenRegistration;

    /**
     * @brief Propagates notification that operations should be cancelled.
     *
     * Partial C++ counterpart of .NET System.Threading.CancellationToken.
     *
     * @note Status: Partial — implements IsCancellationRequested, ThrowIfCancellationRequested,
     * Register(callback), and the static None token. Missing relative to real .NET:
     * CanBeCanceled, WaitHandle, the `bool canceled` constructor, the state-object/
     * useSynchronizationContext Register overloads, UnsafeRegister, and value-equality
     * (Equals/GetHashCode/operator==/!=) — this port's CancellationToken is copyable but not
     * value-comparable. Covers the common "pass a token, check it, register one callback"
     * pattern; the missing surface is for less-common interop/equality scenarios.
     */
    class CancellationToken {
        std::shared_ptr<Detail::CancellationState> state_;
    public:
        /** Constructs a non-cancelled CancellationToken. */
        CancellationToken() : state_(std::make_shared<Detail::CancellationState>()) {}
        /** Constructs a CancellationToken backed by the given shared state. */
        explicit CancellationToken(std::shared_ptr<Detail::CancellationState> state) : state_(std::move(state)) {}

        /** Returns true if cancellation has been requested. */
        [[nodiscard]] bool getIsCancellationRequestedProperty() const { return state_->cancelled.load(); }

        /** Throws OperationCanceledException if cancellation has been requested. */
        void ThrowIfCancellationRequested() const;

        /**
         * @brief Registers a callback invoked when this token is cancelled.
         *
         * If the token is already cancelled, @p callback runs synchronously before this method
         * returns. Otherwise it runs synchronously on whichever thread calls
         * CancellationTokenSource::Cancel().
         */
        CancellationTokenRegistration Register(std::function<void()> callback);

        /** Returns a CancellationToken that is never in the cancelled state. */
        static const CancellationToken& None() {
            static CancellationToken none;
            return none;
        }

        friend class CancellationTokenSource;
    };

} // namespace System::Threading
