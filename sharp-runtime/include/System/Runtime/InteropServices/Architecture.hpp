// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Runtime::InteropServices {

    /**
     * @brief Indicates the processor architecture.
     *
     * C++ counterpart of .NET System.Runtime.InteropServices.Architecture.
     */
    enum class Architecture {
        X86,
        X64,
        Arm,
        Arm64,
        Wasm,
        S390x,
        LoongArch64,
        Armv6,
        Ppc64le,
        RiscV64,
    };

} // namespace System::Runtime::InteropServices
