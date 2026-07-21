// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>

#include "System/OperationCanceledException.hpp"
#include "System/Threading/CancellationToken.hpp"

namespace System::Threading::Tasks {

    class Task;

    /**
     * @brief Represents an exception used to communicate task cancellation.
     *
     * C++ counterpart of .NET System.Threading.Tasks.TaskCanceledException.
     */
    class TaskCanceledException : public System::OperationCanceledException {
        const Task* canceledTask_ = nullptr;

    public:
        /** @brief Initializes a new instance with the default message ("A task was canceled."). */
        TaskCanceledException();

        /** @brief Initializes a new instance with the specified error message. */
        explicit TaskCanceledException(const std::string& message);

        /** @brief Initializes a new instance with a message and an inner exception. */
        TaskCanceledException(const std::string& message, std::exception_ptr innerException);

        /** @brief Initializes a new instance with a message, an inner exception, and the triggering token. */
        TaskCanceledException(const std::string& message, std::exception_ptr innerException,
                               const System::Threading::CancellationToken& token);

        /**
         * @brief Initializes a new instance referencing the Task that was canceled.
         *
         * @param task The task that has been canceled; may be nullptr.
         * @note Lifetime hazard: unlike real .NET's TaskCanceledException.Task (a GC-tracked
         * reference to a heap-allocated Task object, always safe to dereference for as long as
         * the exception itself is reachable), this stores a raw, non-owning pointer. If @p task
         * is destroyed (e.g. a local Task going out of scope) before this exception is caught and
         * getTaskProperty() is called, the pointer dangles. Callers must ensure the referenced
         * Task outlives this exception, or avoid getTaskProperty() and rely on the exception's own
         * message/CancellationToken instead.
         */
        explicit TaskCanceledException(const Task* task);

        /**
         * @brief Gets the task associated with this exception, or nullptr if none.
         *
         * C++ counterpart of .NET TaskCanceledException.Task. See this type's pointer
         * constructor for the lifetime contract this raw pointer carries.
         */
        [[nodiscard]] const Task* getTaskProperty() const { return canceledTask_; }
    };

} // namespace System::Threading::Tasks
