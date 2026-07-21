// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <optional>
#include <stdexcept>
#include <variant>
#include "System/Threading/Tasks/Task.hpp"

namespace System::Threading::Tasks {

    /**
     * @brief Represents an asynchronous operation that either completes synchronously or wraps a Task.
     *
     * @note When constructed from a Task, this port keeps the Task alive and delegates completion
     * queries/awaiting to it live, rather than snapshotting a single bool at construction time.
     * The previous implementation captured only `t.getIsCompletedProperty()` at construction and
     * then discarded the Task entirely -- a still-running task's later completion/fault was never
     * observed (getIsCompletedProperty() stayed stuck at its construction-time snapshot forever),
     * and even an already-faulted task's exception was silently dropped (only the completed-ness
     * bool was captured, never the exception), so GetAwaiter() could never rethrow it.
     */
    class ValueTask {
        bool completed_;
        std::exception_ptr exception_;
        std::optional<Task> task_;

    public:
        /** Constructs a successfully completed ValueTask. */
        ValueTask() : completed_(true) {}
        /** Constructs a ValueTask that wraps a Task, delegating completion/result/exception to it live. */
        explicit ValueTask(Task t) : completed_(false), task_(std::move(t)) {}
        /** Constructs a faulted ValueTask from the given exception pointer. */
        explicit ValueTask(std::exception_ptr ex) : completed_(true), exception_(ex) {}

        /** Returns true if the operation has completed. */
        [[nodiscard]] bool getIsCompletedProperty() const {
            return task_ ? task_->getIsCompletedProperty() : completed_;
        }
        /** Returns true if the operation completed successfully. */
        [[nodiscard]] bool getIsCompletedSuccessfullyProperty() const {
            return task_ ? task_->getIsCompletedSuccessfullyProperty() : (completed_ && !exception_);
        }
        /** Returns true if the operation faulted. */
        [[nodiscard]] bool getIsFaultedProperty() const {
            return task_ ? task_->getIsFaultedProperty() : (exception_ != nullptr);
        }

        /** Blocks until completion (if wrapping a Task) and rethrows the stored/underlying exception, if any. */
        void GetAwaiter() {
            if (task_) { task_->Wait(); return; }
            if (exception_) std::rethrow_exception(exception_);
        }

        /** Returns a completed ValueTask. */
        static ValueTask CompletedTask() { return ValueTask(); }

        /** Returns a ValueTask that faulted with the given exception. */
        static ValueTask FromException(std::exception_ptr ex) { return ValueTask(ex); }
    };

    /**
     * @brief Represents an asynchronous operation that produces a result of type TResult.
     *
     * @note Two real bugs found and fixed during a later audit pass, both mirroring issues
     * already fixed for the non-generic ValueTask above:
     * (1) The default constructor previously set completed_ = false, permanently "incomplete"
     * with no wrapped task and no way to ever complete it later. Verified against
     * ValueTask<TResult>.IsCompleted (`if (obj == null) return true;`): real .NET's "no backing
     * Task/IValueTaskSource" state (which this default constructor represents) is always
     * considered completed, holding default(TResult) -- not permanently pending.
     * (2) There was no TaskT<TResult>-wrapping constructor at all, unlike the sibling ValueTask's
     * Task-wrapping constructor -- meaning this type could only ever represent an
     * already-known synchronous result or exception, never a still-running async operation
     * (defeating half of ValueTask's purpose). Added, following the exact same
     * keep-the-task-alive-and-delegate-live pattern already established for ValueTask.
     */
    template<typename TResult>
    class ValueTaskT {
        bool completed_;
        TResult result_{};
        std::exception_ptr exception_;
        std::optional<TaskT<TResult>> task_;

    public:
        /** Constructs a successfully completed ValueTaskT holding default(TResult). */
        ValueTaskT() : completed_(true) {}
        /** Constructs a successfully completed ValueTaskT with the given result. */
        explicit ValueTaskT(const TResult& result) : completed_(true), result_(result) {}
        /** Constructs a faulted ValueTaskT from the given exception pointer. */
        explicit ValueTaskT(std::exception_ptr ex) : completed_(true), exception_(ex) {}
        /** Constructs a ValueTaskT that wraps a TaskT<TResult>, delegating completion/result/exception to it live. */
        explicit ValueTaskT(TaskT<TResult> t) : completed_(false), task_(std::move(t)) {}

        /** Returns true if the operation has completed. */
        [[nodiscard]] bool getIsCompletedProperty() const {
            return task_ ? task_->getIsCompletedProperty() : completed_;
        }
        /** Returns true if the operation completed successfully. */
        [[nodiscard]] bool getIsCompletedSuccessfullyProperty() const {
            return task_ ? task_->getIsCompletedSuccessfullyProperty() : (completed_ && !exception_);
        }
        /** Returns true if the operation faulted. */
        [[nodiscard]] bool getIsFaultedProperty() const {
            return task_ ? task_->getIsFaultedProperty() : (exception_ != nullptr);
        }

        /** Blocks until completion (if wrapping a TaskT) and returns the result, or rethrows the stored exception if faulted. */
        TResult getResultProperty() {
            if (task_) return task_->getResultProperty();
            if (exception_) std::rethrow_exception(exception_);
            return result_;
        }

        /** Returns a successfully completed ValueTaskT holding v. */
        static ValueTaskT<TResult> FromResult(const TResult& v) { return ValueTaskT<TResult>(v); }
        /** Returns a faulted ValueTaskT wrapping the given exception. */
        static ValueTaskT<TResult> FromException(std::exception_ptr ex) { return ValueTaskT<TResult>(ex); }
    };

} // namespace System::Threading::Tasks
