// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading {

    /**
     * @brief Represents an object whose Execute method is invoked when a work item runs on the thread pool.
     * C++ counterpart of .NET System.Threading.IThreadPoolWorkItem.
     */
    class IThreadPoolWorkItem {
    public:
        virtual ~IThreadPoolWorkItem() = default;

        /** Executes the work item. */
        virtual void Execute() = 0;
    };

} // namespace System::Threading
