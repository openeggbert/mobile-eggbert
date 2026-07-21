// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>

namespace System {

    /**
     * @brief Represents the size and fragmentation of a GC generation on entry and
     * exit of a garbage collection.
     *
     * C++ counterpart of .NET System.GCGenerationInfo.
     * All properties return zero in this stub implementation.
     */
    struct GCGenerationInfo {
        /** @brief Size in bytes on entry to the reported collection. */
        [[nodiscard]] long long getSizeBeforeBytesProperty()          const noexcept { return 0LL; }
        /** @brief Fragmentation in bytes on entry to the reported collection. */
        [[nodiscard]] long long getFragmentationBeforeBytesProperty() const noexcept { return 0LL; }
        /** @brief Size in bytes on exit from the reported collection. */
        [[nodiscard]] long long getSizeAfterBytesProperty()           const noexcept { return 0LL; }
        /** @brief Fragmentation in bytes on exit from the reported collection. */
        [[nodiscard]] long long getFragmentationAfterBytesProperty()  const noexcept { return 0LL; }
    };

} // namespace System
