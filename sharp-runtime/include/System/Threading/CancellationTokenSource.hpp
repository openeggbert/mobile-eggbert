// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <exception>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/AggregateException.hpp"
#include "System/ObjectDisposedException.hpp"
#include "System/Threading/CancellationToken.hpp"

namespace System::Threading {

    /**
     * @brief Signals to a CancellationToken that it should be cancelled.
     *
     * Partial C++ counterpart of .NET System.Threading.CancellationTokenSource.
     *
     * @note Status: Partial — implements the default constructor, Token, IsCancellationRequested,
     * Cancel() (with the documented LIFO callback-ordering and AggregateException-on-callback-
     * throw semantics verified against real .NET), and Dispose(). Missing relative to real .NET:
     * the TimeSpan/int-delay constructors and CancelAfter (timer-based delayed cancellation),
     * CreateLinkedTokenSource (linked token sources), the Cancel(bool throwOnFirstException)
     * overload, CancelAsync(), and TryReset(). Covers the common
     * "create a source, hand out its token, cancel it later" pattern.
     */
    class CancellationTokenSource {
        std::shared_ptr<Detail::CancellationState> state_ = std::make_shared<Detail::CancellationState>();
        // Verified data race: disposed_ was a plain bool read by getTokenProperty()/Cancel()'s
        // ThrowIfDisposed() check and written by Dispose() with no synchronization -- a genuine
        // data race (undefined behavior per the C++ memory model) if Dispose() is called
        // concurrently with any other method from a different thread, a realistic pattern for
        // a cancellation primitive (e.g. one thread disposing while another checks/cancels).
        std::atomic<bool> disposed_{false};

        void ThrowIfDisposed() const {
            if (disposed_) throw System::ObjectDisposedException("CancellationTokenSource", "The CancellationTokenSource has been disposed.");
        }

    public:
        /** Initializes a new CancellationTokenSource. */
        CancellationTokenSource() = default;

        /**
         * @brief Returns the CancellationToken associated with this source.
         * @throws System::ObjectDisposedException if this source has been disposed.
         */
        [[nodiscard]] CancellationToken getTokenProperty() const {
            ThrowIfDisposed();
            return CancellationToken(state_);
        }

        /** Returns true if cancellation has been requested. */
        [[nodiscard]] bool getIsCancellationRequestedProperty() const { return state_->cancelled.load(); }

        /**
         * @brief Signals cancellation to all linked CancellationToken holders and runs their registered callbacks.
         * @throws System::ObjectDisposedException if this source has been disposed.
         * @throws System::AggregateException if one or more callbacks threw; every callback still
         * runs regardless (verified against CancellationTokenSource.cs's ExecuteCallbackHandlers).
         */
        void Cancel() {
            ThrowIfDisposed();
            std::vector<std::pair<SharpRuntime::intcs, std::function<void()>>> callbacksToRun;
            {
                // Checking-and-setting `cancelled` under the same lock used by Register() closes
                // the TOCTOU window where a registration could otherwise be added after Cancel()
                // already decided there was nothing to run.
                std::lock_guard<std::mutex> lock(state_->mutex);
                if (state_->cancelled.exchange(true)) return;
                callbacksToRun.reserve(state_->callbacks.size());
                // Verified against CancellationTokenSource.cs's ExecuteCallbackHandlers: real .NET
                // invokes callbacks in LIFO order (most-recently-registered first, "deepest first")
                // so nested/child registrations fire before their parents'. state_->callbacks is a
                // std::map keyed by a monotonically increasing registration id, so a reverse walk
                // yields exactly that order.
                for (auto it = state_->callbacks.rbegin(); it != state_->callbacks.rend(); ++it)
                    callbacksToRun.emplace_back(it->first, it->second);
                state_->callbacks.clear();
            }
            std::vector<std::exception_ptr> exceptions;
            for (auto& [id, callback] : callbacksToRun) {
                if (!callback) continue;
                // Record which callback is executing (and on which thread) so a concurrent
                // CancellationTokenRegistration::Dispose() for this id can wait for it to finish
                // rather than returning while it's still running -- verified against
                // CancellationTokenSource.cs's ExecutingCallbackId/ThreadIDExecutingCallbacks.
                {
                    std::lock_guard<std::mutex> lock(state_->mutex);
                    state_->executingId = id;
                    state_->executingThreadId = std::this_thread::get_id();
                }
                try {
                    callback();
                } catch (...) {
                    exceptions.push_back(std::current_exception());
                }
                {
                    std::lock_guard<std::mutex> lock(state_->mutex);
                    state_->executingId = -1;
                }
                state_->callbackFinished.notify_all();
            }
            if (!exceptions.empty()) throw System::AggregateException(std::move(exceptions));
        }

        /** Releases resources used by this CancellationTokenSource. */
        void Dispose() { disposed_ = true; }
    };

} // namespace System::Threading
