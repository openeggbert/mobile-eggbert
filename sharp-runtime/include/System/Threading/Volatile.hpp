// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>

namespace System::Threading {

    /** Provides methods for reading and writing variables with acquire/release memory ordering. */
    class Volatile {
    public:
        /** Prevents instantiation — all members are static. */
        Volatile() = delete;

        /** Reads the value of location with acquire semantics. */
        template<typename T>
        static T Read(const T& location) {
            return __atomic_load_n(&location, __ATOMIC_ACQUIRE);
        }

        /** Writes value to location with release semantics. */
        template<typename T>
        static void Write(T& location, T value) {
            __atomic_store_n(&location, value, __ATOMIC_RELEASE);
        }
    };

} // namespace System::Threading
