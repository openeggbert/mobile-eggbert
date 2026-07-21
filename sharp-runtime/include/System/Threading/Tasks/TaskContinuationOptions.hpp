// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading::Tasks {

    /**
     * @brief Specifies flags that control optional behavior for the creation and execution of continuation tasks.
     *
     * C++ counterpart of .NET System.Threading.Tasks.TaskContinuationOptions. Provided for API
     * name/value compatibility; this runtime's Task does not yet implement ContinueWith, so these
     * flags are not currently consumed anywhere.
     */
    enum class TaskContinuationOptions {
        /** Default: continuation runs regardless of the antecedent's final TaskStatus. */
        None = 0,
        /** A hint to schedule the continuation in as fair a manner as possible. */
        PreferFairness = 0x01,
        /** Specifies that the continuation will be a long-running, coarse-grained operation. */
        LongRunning = 0x02,
        /** Specifies that the continuation is attached to a parent in the task hierarchy. */
        AttachedToParent = 0x04,
        /** Specifies that an InvalidOperationException is thrown if a child task is attached to the continuation. */
        DenyChildAttach = 0x08,
        /** Prevents the ambient scheduler from being seen as the current scheduler in the continuation. */
        HideScheduler = 0x10,
        /** In the case of continuation cancellation, prevents completion until the antecedent has completed. */
        LazyCancellation = 0x20,
        /** Forces the continuation to be executed asynchronously. */
        RunContinuationsAsynchronously = 0x40,
        /** The continuation is not scheduled if the antecedent ran to completion. */
        NotOnRanToCompletion = 0x10000,
        /** The continuation is not scheduled if the antecedent threw an unhandled exception. */
        NotOnFaulted = 0x20000,
        /** The continuation is not scheduled if the antecedent was canceled. */
        NotOnCanceled = 0x40000,
        /** The continuation is scheduled only if the antecedent ran to completion. */
        OnlyOnRanToCompletion = NotOnFaulted | NotOnCanceled,
        /** The continuation is scheduled only if the antecedent threw an unhandled exception. */
        OnlyOnFaulted = NotOnRanToCompletion | NotOnCanceled,
        /** The continuation is scheduled only if the antecedent was canceled. */
        OnlyOnCanceled = NotOnRanToCompletion | NotOnFaulted,
        /** The continuation is run synchronously on the thread that completes the antecedent. */
        ExecuteSynchronously = 0x80000,
    };

    /** Returns the bitwise OR of two TaskContinuationOptions values. */
    inline TaskContinuationOptions operator|(TaskContinuationOptions a, TaskContinuationOptions b) {
        return static_cast<TaskContinuationOptions>(static_cast<int>(a) | static_cast<int>(b));
    }
    /** Returns the bitwise AND of two TaskContinuationOptions values. */
    inline TaskContinuationOptions operator&(TaskContinuationOptions a, TaskContinuationOptions b) {
        return static_cast<TaskContinuationOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Threading::Tasks
