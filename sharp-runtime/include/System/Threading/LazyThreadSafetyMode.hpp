// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Threading {

    /** Specifies how a Lazy<T> instance synchronises access among multiple threads. */
    enum class LazyThreadSafetyMode {
        /** The instance provides no thread safety guarantees. */
        None                   = 0,
        /** Multiple threads may concurrently call the value factory; only one result is published. */
        PublicationOnly        = 1,
        /** Only one thread runs the value factory; all other threads wait. */
        ExecutionAndPublication = 2,
    };

} // namespace System::Threading
