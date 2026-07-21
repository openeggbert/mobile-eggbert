// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Specifies the range of legal key/block sizes for a symmetric algorithm.
     *
     * C++ counterpart of .NET System.Security.Cryptography.KeySizes.
     */
    class KeySizes {
        SharpRuntime::intcs minSize_;
        SharpRuntime::intcs maxSize_;
        SharpRuntime::intcs skipSize_;

    public:
        KeySizes(SharpRuntime::intcs minSize, SharpRuntime::intcs maxSize, SharpRuntime::intcs skipSize)
            : minSize_(minSize), maxSize_(maxSize), skipSize_(skipSize) {}

        /** @return The minimum valid size, in bits. */
        [[nodiscard]] SharpRuntime::intcs getMinSizeProperty() const { return minSize_; }
        /** @return The maximum valid size, in bits. */
        [[nodiscard]] SharpRuntime::intcs getMaxSizeProperty() const { return maxSize_; }
        /** @return The increment between valid sizes, in bits (0 if only MinSize==MaxSize is valid). */
        [[nodiscard]] SharpRuntime::intcs getSkipSizeProperty() const { return skipSize_; }
    };

} // namespace System::Security::Cryptography
