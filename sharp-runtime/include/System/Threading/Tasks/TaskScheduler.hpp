// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <limits>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Threading::Tasks {

    /**
     * @brief Represents an object that handles the low-level work of queuing tasks onto threads.
     *
     * C++ counterpart of .NET System.Threading.Tasks.TaskScheduler. In .NET, Task consults a
     * pluggable TaskScheduler to decide how/where queued work actually runs. This runtime's Task
     * (see Task.hpp) always executes eagerly on a dedicated std::async thread instead of consulting
     * a scheduler, so TaskScheduler here is a practical-subset stub: it exists so that ported code
     * referencing `TaskScheduler.Default`/`TaskScheduler.Current`/`.Id` compiles and behaves
     * sensibly, but subclassing it and expecting Task to route work through QueueTask/TryExecuteTask
     * will not work — there is no queuing engine to plug into.
     */
    class TaskScheduler {
        SharpRuntime::intcs id_;

    public:
        TaskScheduler();
        virtual ~TaskScheduler() = default;

        /** @brief Gets the maximum concurrency level this scheduler is able to support. */
        [[nodiscard]] virtual SharpRuntime::intcs getMaximumConcurrencyLevelProperty() const
        {
            return std::numeric_limits<SharpRuntime::intcs>::max();
        }

        /** @brief Gets the unique ID for this TaskScheduler. */
        [[nodiscard]] SharpRuntime::intcs getIdProperty() const { return id_; }

        /** @brief Gets the default TaskScheduler instance provided by the runtime. */
        static TaskScheduler& Default();

        /**
         * @brief Gets the TaskScheduler associated with the currently executing task, or Default
         * if there is none.
         *
         * @note Always returns Default(): this runtime does not track an ambient "currently
         * executing task" scheduler context, since Task does not route execution through a scheduler.
         */
        static TaskScheduler& Current();
    };

} // namespace System::Threading::Tasks
