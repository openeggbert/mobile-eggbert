// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>

#include "System/Threading/CancellationToken.hpp"
#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/TaskContinuationOptions.hpp"
#include "System/Threading/Tasks/TaskCreationOptions.hpp"
#include "System/Threading/Tasks/TaskScheduler.hpp"

namespace System::Threading::Tasks {

    /**
     * @brief Provides support for creating and scheduling Task objects.
     *
     * C++ counterpart of .NET System.Threading.Tasks.TaskFactory. Practical subset: StartNew
     * overloads for actions and functions are implemented in terms of Task::Run/TaskT::Run; the
     * APM-pattern FromAsync overloads (Begin/End method pairs, IAsyncResult, AsyncCallback) have
     * no equivalent in this runtime (no IAsyncResult port) and are intentionally omitted.
     */
    class TaskFactory {
        System::Threading::CancellationToken defaultCancellationToken_ = System::Threading::CancellationToken::None();
        TaskCreationOptions creationOptions_ = TaskCreationOptions::None;
        TaskContinuationOptions continuationOptions_ = TaskContinuationOptions::None;
        TaskScheduler* scheduler_ = nullptr;

    public:
        /** @brief Initializes a TaskFactory with default configuration. */
        TaskFactory() = default;

        /** @brief Initializes a TaskFactory with the given default CancellationToken. */
        explicit TaskFactory(System::Threading::CancellationToken cancellationToken)
            : defaultCancellationToken_(std::move(cancellationToken))
        {
        }

        /** @brief Initializes a TaskFactory targeting the given TaskScheduler (nullptr means TaskScheduler::Current()). */
        explicit TaskFactory(TaskScheduler* scheduler) : scheduler_(scheduler) {}

        /** @brief Initializes a TaskFactory with the given default creation/continuation options. */
        TaskFactory(TaskCreationOptions creationOptions, TaskContinuationOptions continuationOptions)
            : creationOptions_(creationOptions), continuationOptions_(continuationOptions)
        {
        }

        /** @brief Initializes a TaskFactory with every default configurable option. */
        TaskFactory(System::Threading::CancellationToken cancellationToken,
                    TaskCreationOptions creationOptions,
                    TaskContinuationOptions continuationOptions,
                    TaskScheduler* scheduler)
            : defaultCancellationToken_(std::move(cancellationToken))
            , creationOptions_(creationOptions)
            , continuationOptions_(continuationOptions)
            , scheduler_(scheduler)
        {
        }

        /** @brief Gets the default CancellationToken used when creating tasks. */
        [[nodiscard]] const System::Threading::CancellationToken& getCancellationTokenProperty() const {
            return defaultCancellationToken_;
        }
        /** @brief Gets the TaskScheduler used when creating tasks, or nullptr if none was set (TaskScheduler::Current() applies). */
        [[nodiscard]] TaskScheduler* getSchedulerProperty() const { return scheduler_; }
        /** @brief Gets the default TaskCreationOptions used when creating tasks. */
        [[nodiscard]] TaskCreationOptions getCreationOptionsProperty() const { return creationOptions_; }
        /** @brief Gets the default TaskContinuationOptions used when creating continuations. */
        [[nodiscard]] TaskContinuationOptions getContinuationOptionsProperty() const { return continuationOptions_; }

        /** @brief Creates and starts a Task that runs @p action, observing this factory's default CancellationToken. */
        [[nodiscard]] Task StartNew(std::function<void()> action) const {
            return Task::Run(std::move(action), defaultCancellationToken_);
        }

        /** @brief Creates and starts a Task that runs @p action, observing @p cancellationToken. */
        [[nodiscard]] Task StartNew(std::function<void()> action,
                                     System::Threading::CancellationToken cancellationToken) const {
            return Task::Run(std::move(action), std::move(cancellationToken));
        }

        /**
         * @brief Creates and starts a TaskT<TResult> that runs @p function, observing this
         * factory's default CancellationToken, returning its result.
         *
         * @note Previously did not observe defaultCancellationToken_ at all -- an inconsistency
         * with the non-generic StartNew(action) overload above, which always has. Now that
         * TaskT<TResult> supports cancellation (see Task.hpp), this overload was fixed to match.
         */
        template<typename TResult>
        [[nodiscard]] TaskT<TResult> StartNew(std::function<TResult()> function) const {
            return TaskT<TResult>::Run(std::move(function), defaultCancellationToken_);
        }

        /** @brief Creates and starts a TaskT<TResult> that runs @p function, observing @p cancellationToken. */
        template<typename TResult>
        [[nodiscard]] TaskT<TResult> StartNew(std::function<TResult()> function,
                                               System::Threading::CancellationToken cancellationToken) const {
            return TaskT<TResult>::Run(std::move(function), std::move(cancellationToken));
        }
    };

    inline TaskFactory Task::Factory() { return TaskFactory(); }

} // namespace System::Threading::Tasks
