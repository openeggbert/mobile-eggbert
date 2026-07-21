// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>

namespace System::Threading {

    /** @brief Represents a callback method executed by the thread pool, receiving a single state argument. */
    using WaitCallback = std::function<void(void* /*state*/)>;

} // namespace System::Threading
