// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading::Tasks {

    /**
     * @brief Represents the current stage in the lifecycle of a Task.
     *
     * C++ counterpart of .NET System.Threading.Tasks.TaskStatus. This runtime's Task always
     * executes eagerly via std::async as soon as it is constructed (see Task.hpp), so the
     * Created/WaitingForActivation/WaitingToRun states are not observable in practice — Task
     * transitions straight from Running to a terminal state. The enum is complete for API
     * name/value compatibility with ported code that switches on TaskStatus.
     */
    enum class TaskStatus {
        /** The task has been initialized but has not yet been scheduled. */
        Created = 0,
        /** The task is waiting to be activated and scheduled internally by the runtime. */
        WaitingForActivation = 1,
        /** The task has been scheduled for execution but has not yet begun executing. */
        WaitingToRun = 2,
        /** The task is running but has not yet completed. */
        Running = 3,
        /** The task has finished executing and is implicitly waiting for attached child tasks to complete. */
        WaitingForChildrenToComplete = 4,
        /** The task completed execution successfully. */
        RanToCompletion = 5,
        /** The task acknowledged cancellation by throwing an OperationCanceledException with its own CancellationToken. */
        Canceled = 6,
        /** The task completed due to an unhandled exception. */
        Faulted = 7,
    };

} // namespace System::Threading::Tasks
