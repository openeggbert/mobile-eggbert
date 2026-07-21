// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <array>
#include <cstdint>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Globalization {

using SharpRuntime::intcs;

/**
 * @brief Provides information about the version of Unicode used to compare and order strings.
 *
 * C++ counterpart of .NET System.Globalization.SortVersion.
 * Used to ensure that string comparisons are consistent across versions of the runtime
 * and Unicode tables.
 */
class SortVersion {
    intcs fullVersion_;
    std::array<uint8_t, 16> sortId_ = {};

public:
    /**
     * @brief Constructs a SortVersion with the given full version and a zeroed sort ID.
     *
     * C++ counterpart of .NET SortVersion(int, Guid) — Guid defaults to all zeros.
     * @param fullVersion The full numeric version of the sort tables.
     */
    explicit SortVersion(intcs fullVersion)
        : fullVersion_(fullVersion) {}

    /**
     * @brief Constructs a SortVersion with the given full version and sort ID.
     *
     * C++ counterpart of .NET SortVersion(int, Guid).
     * @param fullVersion The full numeric version of the sort tables.
     * @param sortId      A 16-byte identifier for the sort tables.
     */
    SortVersion(intcs fullVersion, const std::array<uint8_t, 16>& sortId)
        : fullVersion_(fullVersion), sortId_(sortId) {}

    /**
     * @brief Gets the full version number of the sort tables.
     *
     * C++ counterpart of .NET SortVersion.FullVersion.
     * @return The full version integer.
     */
    [[nodiscard]] intcs getFullVersionProperty() const { return fullVersion_; }

    /**
     * @brief Gets the globally unique identifier for the sort tables.
     *
     * C++ counterpart of .NET SortVersion.SortId (mapped from Guid to 16-byte array).
     * @return A const reference to the 16-byte sort ID.
     */
    [[nodiscard]] const std::array<uint8_t, 16>& getSortIdProperty() const { return sortId_; }

    /**
     * @brief Determines whether this SortVersion equals another.
     *
     * C++ counterpart of .NET SortVersion.Equals(SortVersion).
     * @param other The SortVersion to compare.
     * @return true if both fullVersion and sortId are equal.
     */
    [[nodiscard]] bool Equals(const SortVersion& other) const {
        return fullVersion_ == other.fullVersion_ && sortId_ == other.sortId_;
    }

    /**
     * @brief Returns a hash code for this SortVersion.
     *
     * C++ counterpart of .NET SortVersion.GetHashCode().
     * @return A hash derived from fullVersion and the first 4 bytes of sortId.
     */
    [[nodiscard]] intcs GetHashCode() const {
        intcs h = fullVersion_;
        for (int i = 0; i < 4 && i < static_cast<int>(sortId_.size()); ++i)
            h = h * 31 + sortId_[i];
        return h;
    }

    /**
     * @brief Returns true if both SortVersion instances have equal version and sort ID.
     *
     * C++ counterpart of .NET SortVersion equality operator.
     * @param o The SortVersion to compare.
     * @return true if both fullVersion and sortId are equal.
     */
    bool operator==(const SortVersion& o) const { return Equals(o); }

    /**
     * @brief Returns true if either the version or sort ID differs.
     *
     * C++ counterpart of .NET SortVersion inequality operator.
     * @param o The SortVersion to compare.
     * @return true if the instances differ.
     */
    bool operator!=(const SortVersion& o) const { return !Equals(o); }
};

} // namespace System::Globalization
