// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>

namespace System::Runtime::CompilerServices {

    enum class MethodImplOptions : uint16_t {
        Unmanaged         = 0x0004,
        NoInlining        = 0x0008,
        ForwardRef        = 0x0010,
        Synchronized      = 0x0020,
        NoOptimization    = 0x0040,
        PreserveSig       = 0x0080,
        AggressiveInlining = 0x0100,
        AggressiveOptimization = 0x0200,
        InternalCall      = 0x1000,
    };

    inline MethodImplOptions operator|(MethodImplOptions a, MethodImplOptions b) {
        return static_cast<MethodImplOptions>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
    }

} // namespace System::Runtime::CompilerServices
