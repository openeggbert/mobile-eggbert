// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <exception>
#include <future>
#include <memory>
#include <stdexcept>
#include "System/InvalidOperationException.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/TaskCanceledException.hpp"

namespace System::Threading::Tasks {

    namespace detail {
        // Verified against TaskT_TransitionToFinal_AlreadyCompleted in Strings.resx.
        inline const char* taskCompletionSourceAlreadyCompletedMessage() {
            return "An attempt was made to transition a task to a final state when it had already completed.";
        }
    }

    /**
     * Provides producer-side control over a Task's completion for a result type TResult.
     *
     * Wraps std::promise/std::shared_future. Partial C++ counterpart of .NET TaskCompletionSource<TResult>.
     *
     * @note Matches TaskCompletionSource_T.cs's structure: the Try* methods are the atomic
     * primitives (an std::atomic compare-exchange claims the single completing thread), and
     * the non-Try Set* methods call them and throw if the claim fails -- not the reverse. The
     * previous implementation had non-atomic check-then-set logic in Set*, so two threads
     * racing a TrySet* call could both observe "not yet completed" and both proceed to call
     * promise_.set_value()/set_exception(), and the loser would throw an uncaught
     * std::future_error instead of TrySet* returning false as .NET guarantees.
     *
     * @note Exposes a `getTaskProperty()` bridging this source's completion onto the ordinary
     * TaskT<TResult> API (2026-07-14) -- real .NET's `Task<TResult> Task { get; }` property.
     * This port's Task/TaskT always launches an async lambda immediately on construction (see
     * Task.hpp) with no "pending, externally completed later" mode of its own, unlike real
     * .NET's Task; TaskT<TResult>::FromExternalFuture() bridges the two by wrapping this
     * source's own future and spawning one watcher thread that mirrors its eventual outcome
     * into the returned TaskT's state -- see that method's own doc-comment for the full
     * rationale. The bridging TaskT is created once in the constructor (matching real .NET's
     * own stable-identity Task property) and returned by value on every getTaskProperty() call
     * (cheap: TaskT is just two shared_ptrs + a CancellationToken).
     *
     * @note Declares an explicit destructor that force-completes an unfulfilled promise_ before
     * any member's implicit destructor runs (2026-07-14) -- required to avoid a genuine deadlock,
     * confirmed via a standalone repro on this toolchain: destroying the LAST shared_future
     * reference to an std::async-launched task blocks until that task's callable returns (this
     * holds for shared_future here, not just plain future -- an implementation behavior, not
     * something the standard requires, but real on GCC/libstdc++). task_'s implicit member
     * destructor runs before promise_'s (reverse declaration order), so without this, task_'s
     * internal watcher thread (blocked on this source's own future) would still be waiting for
     * promise_ to resolve while the destructor destroying task_ is itself blocked waiting for
     * that same watcher thread -- a cycle with no way out. Resolving promise_ first breaks it.
     *
     * @warning **Lifetime requirement, unlike real .NET**: real .NET's TaskCompletionSource is a
     * GC-tracked reference type, so the common "spin up a producer thread that completes the
     * source, hand out .Task, let the local reference go" idiom is automatically safe -- the GC
     * keeps the object alive as long as anything (including the producer thread's closure) still
     * references it. This C++ port has ordinary RAII lifetime: **a TaskCompletionSource must
     * remain alive for as long as any thread might still call one of its completion methods
     * (SetResult/SetException/SetCanceled, or their Try-prefixed equivalents).** Destroying it
     * while another thread is genuinely still inside one of those calls is
     * undefined behavior (confirmed via a standalone ThreadSanitizer repro, 2026-07-14) -- the
     * same as destroying any C++ object while a method call on it is concurrently in flight
     * elsewhere, not something specific to this class's internals. When completing from a
     * background thread, hold the source via `std::shared_ptr<TaskCompletionSource<TResult>>`
     * and capture that shared_ptr **by value** in the worker thread's closure (not a reference or
     * raw pointer to a stack-local instance) -- the shared_ptr copy keeps the source alive for as
     * long as the worker needs it, independent of when the original local goes out of scope. The
     * returned `getTaskProperty()` Task/TaskT is always safe to read independently (it owns its
     * own state via shared_ptr) -- only the *source* itself has this constraint.
     */
    template<typename TResult>
    class TaskCompletionSource {
        std::promise<TResult> promise_;
        std::shared_future<TResult> future_;
        std::atomic<bool> completed_{false}; ///< True once the source has been completed.
        // Producer-set, out-of-band signal so the bridging TaskT (via FromExternalFuture) can
        // distinguish genuine cancellation (TrySetCanceled()/the destructor's own abandonment
        // path) from a caller-supplied TaskCanceledException passed to SetException() -- see
        // TaskT::FromExternalFuture's doc-comment for why exception-type sniffing alone can't
        // tell the two apart. Declared before task_ so it's already constructed by the time
        // task_'s initializer runs (member init order follows declaration order).
        std::shared_ptr<std::atomic<bool>> canceledFlag_ = std::make_shared<std::atomic<bool>>(false);
        TaskT<TResult> task_;

    public:
        /** Default constructor — creates an unresolved source. */
        TaskCompletionSource()
            : future_(promise_.get_future().share()),
              task_(TaskT<TResult>::FromExternalFuture(std::make_shared<std::shared_future<TResult>>(future_), canceledFlag_)) {}

        /** See the class-level @note on why this can't be the implicitly-generated destructor. */
        ~TaskCompletionSource() {
            if (!completed_.exchange(true)) {
                canceledFlag_->store(true);
                promise_.set_exception(std::make_exception_ptr(System::Threading::Tasks::TaskCanceledException()));
            }
        }

        /** The TaskT<TResult> this source controls; completes when this source does. */
        [[nodiscard]] TaskT<TResult> getTaskProperty() const { return task_; }

        /**
         * Transitions the task to a successful result.
         * @throws System::InvalidOperationException if the source was already completed.
         */
        void SetResult(const TResult& result) {
            if (!TrySetResult(result))
                throw System::InvalidOperationException(detail::taskCompletionSourceAlreadyCompletedMessage());
        }

        /**
         * Transitions the task to a faulted state with the given exception.
         * @throws System::InvalidOperationException if the source was already completed.
         */
        void SetException(std::exception_ptr ex) {
            if (!TrySetException(ex))
                throw System::InvalidOperationException(detail::taskCompletionSourceAlreadyCompletedMessage());
        }

        /**
         * Transitions the task to a canceled state.
         * @throws System::InvalidOperationException if the source was already completed.
         */
        void SetCanceled() {
            if (!TrySetCanceled())
                throw System::InvalidOperationException(detail::taskCompletionSourceAlreadyCompletedMessage());
        }

        /**
         * Attempts to set a successful result; returns false if already completed.
         *
         * @note If TResult's copy constructor throws (audit finding A-04, 2026-07-14),
         * promise_.set_value(result) can fail AFTER completed_ has already been claimed true but
         * BEFORE the promise's shared state becomes ready. Without the try/catch below, that used
         * to strand the source permanently: no later TrySetResult/TrySetException/TrySetCanceled
         * call could re-claim completed_, and everything waiting on GetResult() or the bridging
         * Task (getTaskProperty()) blocked forever, confirmed via a standalone repro (a
         * throwing-copy result type left the task incomplete and TrySetCanceled() returning
         * false, with no way to unblock a waiter). The promise is now settled with the copy
         * failure itself so no watcher is left hanging, and the failure still propagates to this
         * immediate caller exactly as it did before (unchanged for callers who already wrap
         * TrySetResult/SetResult in their own try/catch).
         */
        bool TrySetResult(const TResult& result) {
            bool expected = false;
            if (!completed_.compare_exchange_strong(expected, true)) return false;
            try {
                promise_.set_value(result);
            } catch (...) {
                promise_.set_exception(std::current_exception());
                throw;
            }
            return true;
        }

        /** Attempts to fault the task; returns false if already completed. */
        bool TrySetException(std::exception_ptr ex) {
            bool expected = false;
            if (!completed_.compare_exchange_strong(expected, true)) return false;
            promise_.set_exception(ex);
            return true;
        }

        /** Attempts to cancel the task; returns false if already completed. */
        bool TrySetCanceled() {
            bool expected = false;
            if (!completed_.compare_exchange_strong(expected, true)) return false;
            canceledFlag_->store(true);
            promise_.set_exception(std::make_exception_ptr(System::Threading::Tasks::TaskCanceledException()));
            return true;
        }

        /** Blocks until the task completes and returns its result. */
        TResult GetResult() { return future_.get(); }
    };

    /**
     * Specialisation of TaskCompletionSource for void tasks (no result value).
     *
     * @note Declares an explicit destructor for the same reason as the primary template -- see
     * its class-level @note for the full deadlock rationale, the exception-fidelity rationale
     * behind canceledFlag_, and the @warning about this class's lifetime requirements (unlike
     * real .NET's GC-tracked TaskCompletionSource).
     */
    template<>
    class TaskCompletionSource<void> {
        std::promise<void> promise_;
        std::shared_future<void> future_;
        std::atomic<bool> completed_{false}; ///< True once the source has been completed.
        std::shared_ptr<std::atomic<bool>> canceledFlag_ = std::make_shared<std::atomic<bool>>(false);
        Task task_;

    public:
        /** Default constructor — creates an unresolved source. */
        TaskCompletionSource()
            : future_(promise_.get_future().share()),
              task_(Task::FromExternalFuture(std::make_shared<std::shared_future<void>>(future_), canceledFlag_)) {}

        /** See the class-level @note on why this can't be the implicitly-generated destructor. */
        ~TaskCompletionSource() {
            if (!completed_.exchange(true)) {
                canceledFlag_->store(true);
                promise_.set_exception(std::make_exception_ptr(System::Threading::Tasks::TaskCanceledException()));
            }
        }

        /** The Task this source controls; completes when this source does. */
        [[nodiscard]] Task getTaskProperty() const { return task_; }

        /**
         * Transitions the task to the completed state.
         * @throws System::InvalidOperationException if the source was already completed.
         */
        void SetResult() {
            if (!TrySetResult())
                throw System::InvalidOperationException(detail::taskCompletionSourceAlreadyCompletedMessage());
        }

        /**
         * Transitions the task to a faulted state with the given exception.
         * @throws System::InvalidOperationException if the source was already completed.
         */
        void SetException(std::exception_ptr ex) {
            if (!TrySetException(ex))
                throw System::InvalidOperationException(detail::taskCompletionSourceAlreadyCompletedMessage());
        }

        /**
         * Transitions the task to a canceled state.
         * @throws System::InvalidOperationException if the source was already completed.
         */
        void SetCanceled() {
            if (!TrySetCanceled())
                throw System::InvalidOperationException(detail::taskCompletionSourceAlreadyCompletedMessage());
        }

        /** Attempts to complete the task; returns false if already completed. */
        bool TrySetResult() {
            bool expected = false;
            if (!completed_.compare_exchange_strong(expected, true)) return false;
            promise_.set_value();
            return true;
        }

        /** Attempts to fault the task; returns false if already completed. */
        bool TrySetException(std::exception_ptr ex) {
            bool expected = false;
            if (!completed_.compare_exchange_strong(expected, true)) return false;
            promise_.set_exception(ex);
            return true;
        }

        /** Attempts to cancel the task; returns false if already completed. */
        bool TrySetCanceled() {
            bool expected = false;
            if (!completed_.compare_exchange_strong(expected, true)) return false;
            canceledFlag_->store(true);
            promise_.set_exception(std::make_exception_ptr(System::Threading::Tasks::TaskCanceledException()));
            return true;
        }

        /** Blocks until the task completes. */
        void Wait() { future_.get(); }
    };

} // namespace System::Threading::Tasks
