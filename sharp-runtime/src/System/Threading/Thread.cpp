// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Threading/Thread.hpp"

#if defined(_WIN32)
#  include <windows.h>
#elif defined(__EMSCRIPTEN__)
// No per-processor scheduling info available.
#elif defined(__linux__)
#  include <sched.h>
#endif

namespace System::Threading {

    intcs Thread::GetCurrentProcessorId() {
#if defined(_WIN32)
        return static_cast<intcs>(GetCurrentProcessorNumber());
#elif defined(__EMSCRIPTEN__)
        return 0;
#elif defined(__linux__)
        return static_cast<intcs>(sched_getcpu());
#else
        return 0;
#endif
    }

} // namespace System::Threading
