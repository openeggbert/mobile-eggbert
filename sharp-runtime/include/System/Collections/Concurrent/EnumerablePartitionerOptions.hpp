// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Collections::Concurrent {

/**
 * @brief Specifies options to control the buffering behavior of a custom partitioner.
 *
 * C++ counterpart of .NET System.Collections.Concurrent.EnumerablePartitionerOptions.
 * This is a flags enum — values may be combined with bitwise OR.
 */
enum class EnumerablePartitionerOptions : int {
    /**
     * @brief Use the default behavior (buffering enabled for optimal throughput).
     *
     * C++ counterpart of .NET EnumerablePartitionerOptions.None.
     */
    None = 0x0,

    /**
     * @brief Creates a partitioner that takes items one at a time without intermediate buffering.
     *
     * C++ counterpart of .NET EnumerablePartitionerOptions.NoBuffering.
     * Provides lower latency and partial support for inter-item dependencies.
     */
    NoBuffering = 0x1
};

/** @brief Bitwise OR for EnumerablePartitionerOptions flags. */
inline EnumerablePartitionerOptions operator|(EnumerablePartitionerOptions a, EnumerablePartitionerOptions b) {
    return static_cast<EnumerablePartitionerOptions>(static_cast<int>(a) | static_cast<int>(b));
}

/** @brief Bitwise AND for EnumerablePartitionerOptions flags. */
inline EnumerablePartitionerOptions operator&(EnumerablePartitionerOptions a, EnumerablePartitionerOptions b) {
    return static_cast<EnumerablePartitionerOptions>(static_cast<int>(a) & static_cast<int>(b));
}

} // namespace System::Collections::Concurrent
