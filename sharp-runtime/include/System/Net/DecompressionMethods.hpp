// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Net {

    /**
     * @brief Represents the file compression and decompression encoding format to use to
     * compress the data received in response to an HTTP request. [Flags]
     *
     * C++ counterpart of .NET System.Net.DecompressionMethods.
     */
    enum class DecompressionMethods {
        /** No compression. */
        None = 0x0,
        /** Gzip compression. */
        GZip = 0x1,
        /** Deflate compression. */
        Deflate = 0x2,
        /** Brotli compression. */
        Brotli = 0x4,
        /** Zstandard compression. */
        Zstandard = 0x8,
        /** All compression methods supported by this runtime. */
        All = ~0,
    };

    /** @brief Bitwise OR for the [Flags] DecompressionMethods enum. */
    constexpr DecompressionMethods operator|(DecompressionMethods a, DecompressionMethods b) {
        return static_cast<DecompressionMethods>(static_cast<int>(a) | static_cast<int>(b));
    }

    /** @brief Bitwise AND for the [Flags] DecompressionMethods enum. */
    constexpr DecompressionMethods operator&(DecompressionMethods a, DecompressionMethods b) {
        return static_cast<DecompressionMethods>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Net
