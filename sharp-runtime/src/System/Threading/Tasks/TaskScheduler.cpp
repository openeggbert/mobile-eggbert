// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Threading/Tasks/TaskScheduler.hpp"

#include <atomic>

namespace System::Threading::Tasks {

    namespace {
        std::atomic<SharpRuntime::intcs> g_nextId{0};
    }

    TaskScheduler::TaskScheduler() : id_(g_nextId.fetch_add(1)) {}

    TaskScheduler& TaskScheduler::Default()
    {
        static TaskScheduler instance;
        return instance;
    }

    TaskScheduler& TaskScheduler::Current()
    {
        return Default();
    }

} // namespace System::Threading::Tasks
