// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <cstring>
#include "System/Buffers/SequenceReader.hpp"

namespace System::Buffers {

/**
 * @brief Extension methods for SequenceReader&lt;byte&gt; that provide binary reading.
 *
 * C++ counterpart of .NET System.Buffers.SequenceReaderExtensions (binary overloads).
 * These are free functions (C++ has no extension methods).
 *
 * All methods read @p count bytes from @p reader in the specified byte order and
 * advance the reader's position on success.
 */

namespace SequenceReaderExtensions {

namespace detail {
    template<typename T>
    bool tryReadBytes(SequenceReader<uint8_t>& reader, T& value) {
        constexpr int N = static_cast<int>(sizeof(T));
        if (reader.getRemainingProperty() < N) {
            value = T{};
            return false;
        }
        uint8_t buf[sizeof(T)];
        for (int i = 0; i < N; ++i) {
            uint8_t b{};
            reader.TryRead(b);
            buf[i] = b;
        }
        std::memcpy(&value, buf, sizeof(T));
        return true;
    }

    template<typename T>
    T swapBytes(T v) {
        static_assert(sizeof(T) <= 8, "swapBytes supports up to 8 bytes");
        uint8_t buf[sizeof(T)];
        std::memcpy(buf, &v, sizeof(T));
        for (int i = 0, j = static_cast<int>(sizeof(T)) - 1; i < j; ++i, --j)
            std::swap(buf[i], buf[j]);
        T out{};
        std::memcpy(&out, buf, sizeof(T));
        return out;
    }

    constexpr bool isBigEndian() {
        // Compile-time endianness detection (C++20 std::endian equivalent)
        union { uint16_t u; uint8_t b[2]; } e{ 1 };
        return e.b[0] == 0;
    }
} // namespace detail

/**
 * @brief Tries to read a little-endian int16 from @p reader.
 * @return true if successful; false if not enough data remains.
 */
inline bool TryReadLittleEndian(SequenceReader<uint8_t>& reader, int16_t& value) {
    uint16_t raw{};
    if (!detail::tryReadBytes(reader, raw)) { value = 0; return false; }
    value = static_cast<int16_t>(detail::isBigEndian() ? detail::swapBytes(raw) : raw);
    return true;
}

/**
 * @brief Tries to read a big-endian int16 from @p reader.
 */
inline bool TryReadBigEndian(SequenceReader<uint8_t>& reader, int16_t& value) {
    uint16_t raw{};
    if (!detail::tryReadBytes(reader, raw)) { value = 0; return false; }
    value = static_cast<int16_t>(detail::isBigEndian() ? raw : detail::swapBytes(raw));
    return true;
}

/**
 * @brief Tries to read a little-endian int32 from @p reader.
 */
inline bool TryReadLittleEndian(SequenceReader<uint8_t>& reader, int32_t& value) {
    uint32_t raw{};
    if (!detail::tryReadBytes(reader, raw)) { value = 0; return false; }
    value = static_cast<int32_t>(detail::isBigEndian() ? detail::swapBytes(raw) : raw);
    return true;
}

/**
 * @brief Tries to read a big-endian int32 from @p reader.
 */
inline bool TryReadBigEndian(SequenceReader<uint8_t>& reader, int32_t& value) {
    uint32_t raw{};
    if (!detail::tryReadBytes(reader, raw)) { value = 0; return false; }
    value = static_cast<int32_t>(detail::isBigEndian() ? raw : detail::swapBytes(raw));
    return true;
}

/**
 * @brief Tries to read a little-endian int64 from @p reader.
 */
inline bool TryReadLittleEndian(SequenceReader<uint8_t>& reader, int64_t& value) {
    uint64_t raw{};
    if (!detail::tryReadBytes(reader, raw)) { value = 0; return false; }
    value = static_cast<int64_t>(detail::isBigEndian() ? detail::swapBytes(raw) : raw);
    return true;
}

/**
 * @brief Tries to read a big-endian int64 from @p reader.
 */
inline bool TryReadBigEndian(SequenceReader<uint8_t>& reader, int64_t& value) {
    uint64_t raw{};
    if (!detail::tryReadBytes(reader, raw)) { value = 0; return false; }
    value = static_cast<int64_t>(detail::isBigEndian() ? raw : detail::swapBytes(raw));
    return true;
}

} // namespace SequenceReaderExtensions
} // namespace System::Buffers
