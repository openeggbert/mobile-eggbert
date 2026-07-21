// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading::Tasks {

    /**
     * @brief Specifies flags that control optional behavior for the creation and execution of tasks.
     *
     * C++ counterpart of .NET System.Threading.Tasks.TaskCreationOptions. This runtime's Task
     * always executes eagerly on a dedicated std::async thread (see Task.hpp) rather than being
     * queued to a pluggable TaskScheduler, so PreferFairness/HideScheduler/RunContinuationsAsynchronously
     * have no observable effect; the enum exists for API name/value compatibility with ported code.
     */
    enum class TaskCreationOptions {
        /** Specifies that the default behavior should be used. */
        None = 0x0,
        /** A hint to schedule a task in as fair a manner as possible. */
        PreferFairness = 0x01,
        /** Specifies that a task will be a long-running, coarse-grained operation. */
        LongRunning = 0x02,
        /** Specifies that a task is attached to a parent in the task hierarchy. */
        AttachedToParent = 0x04,
        /** Specifies that an InvalidOperationException is thrown if a child task is attached to the created task. */
        DenyChildAttach = 0x08,
        /** Prevents the ambient scheduler from being seen as the current scheduler in the created task. */
        HideScheduler = 0x10,
        /** Forces continuations added to the current task to be executed asynchronously. */
        RunContinuationsAsynchronously = 0x40,
    };

    /** Returns the bitwise OR of two TaskCreationOptions values. */
    inline TaskCreationOptions operator|(TaskCreationOptions a, TaskCreationOptions b) {
        return static_cast<TaskCreationOptions>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** Returns the bitwise AND of two TaskCreationOptions values. */
    inline TaskCreationOptions operator&(TaskCreationOptions a, TaskCreationOptions b) {
        return static_cast<TaskCreationOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Threading::Tasks
