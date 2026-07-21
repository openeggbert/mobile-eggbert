// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Runtime/InteropServices/Architecture.hpp"
#include "System/Runtime/InteropServices/OSPlatform.hpp"

namespace System::Runtime::InteropServices {

    /**
     * @brief Provides information about the .NET runtime installation.
     *
     * C++ counterpart of .NET System.Runtime.InteropServices.RuntimeInformation.
     *
     * @note `RuntimeIdentifier`/`FrameworkDescription` are not reproduced — there is no .NET
     * runtime/assembly identity in a native C++ build for these to describe.
     */
    class RuntimeInformation {
    public:
        RuntimeInformation() = delete;

        /** @return A human-readable description of the operating system. */
        [[nodiscard]] static std::string getOSDescriptionProperty();

        /** @return true if the current platform matches @p osPlatform. */
        [[nodiscard]] static bool IsOSPlatform(const OSPlatform& osPlatform);

        /** @return The process' architecture. */
        [[nodiscard]] static Architecture getProcessArchitectureProperty();

        /** @return The operating system's architecture. */
        [[nodiscard]] static Architecture getOSArchitectureProperty();
    };

} // namespace System::Runtime::InteropServices
