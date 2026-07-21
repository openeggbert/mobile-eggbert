// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>

namespace System::Threading {

    /**
     * @brief Represents a callback method invoked when a WaitHandle registered via
     * ThreadPool::RegisterWaitForSingleObject is signalled or times out.
     * @param state Object passed when the callback was registered.
     * @param timedOut True if the callback was invoked because the wait timed out.
     */
    using WaitOrTimerCallback = std::function<void(void* /*state*/, bool /*timedOut*/)>;

} // namespace System::Threading
