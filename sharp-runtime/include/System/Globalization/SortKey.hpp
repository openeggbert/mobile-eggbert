// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Globalization {

using SharpRuntime::bytecs;
using SharpRuntime::intcs;

/**
 * @brief Represents the result of mapping a string to its sort key for culture-sensitive comparison.
 *
 * C++ counterpart of .NET System.Globalization.SortKey.
 * Sort keys allow efficient repeated comparisons. The key data is a byte array
 * derived from the original string according to the culture's sort rules.
 */
class SortKey {
public:
    /** @brief Constructs an empty SortKey. */
    SortKey() = default;

    /**
     * @brief Constructs a SortKey from an original string and its pre-computed byte key.
     *
     * @param originalString The original string used to create this sort key.
     * @param keyData        The byte array representing the sort key.
     */
    SortKey(const std::string& originalString, const std::vector<bytecs>& keyData)
        : string_(originalString), keyData_(keyData) {}

    /**
     * @brief Gets the original string used to create this SortKey.
     *
     * C++ counterpart of .NET SortKey.OriginalString.
     * @return A const reference to the original string.
     */
    [[nodiscard]] const std::string& getOriginalStringProperty() const { return string_; }

    /**
     * @brief Gets the byte array representing this SortKey.
     *
     * C++ counterpart of .NET SortKey.KeyData.
     * @return A copy of the key data byte vector.
     */
    [[nodiscard]] std::vector<bytecs> getKeyDataProperty() const { return keyData_; }

    /**
     * @brief Compares two SortKey objects using lexicographic byte order.
     *
     * C++ counterpart of .NET SortKey.Compare(SortKey, SortKey).
     * @param sortkey1 The first SortKey.
     * @param sortkey2 The second SortKey.
     * @return Negative if sortkey1 < sortkey2, zero if equal, positive if sortkey1 > sortkey2.
     */
    static intcs Compare(const SortKey& sortkey1, const SortKey& sortkey2) {
        size_t n = std::min(sortkey1.keyData_.size(), sortkey2.keyData_.size());
        for (size_t i = 0; i < n; ++i) {
            if (sortkey1.keyData_[i] < sortkey2.keyData_[i]) return -1;
            if (sortkey1.keyData_[i] > sortkey2.keyData_[i]) return  1;
        }
        if (sortkey1.keyData_.size() < sortkey2.keyData_.size()) return -1;
        if (sortkey1.keyData_.size() > sortkey2.keyData_.size()) return  1;
        return 0;
    }

    /**
     * @brief Returns true if both SortKey instances have identical key data.
     *
     * C++ counterpart of .NET SortKey.Equals(object). Matches .NET exactly: only the key
     * byte sequence is compared (SortKey.cs's Equals uses
     * `_keyData.SequenceEqual(other._keyData)`), not the original source string — two
     * different strings that produce the same sort key (e.g. "HELLO"/"hello" under an
     * ignore-case comparison) are equal SortKeys.
     * @param other The SortKey to compare.
     * @return true if equal; otherwise false.
     */
    bool operator==(const SortKey& other) const {
        return keyData_ == other.keyData_;
    }

    /**
     * @brief Returns a hash code for this SortKey based on its key data.
     *
     * C++ counterpart of .NET SortKey.GetHashCode().
     * @return A hash derived from the key data bytes.
     */
    [[nodiscard]] intcs GetHashCode() const {
        std::size_t h = 0;
        for (auto b : keyData_) h = h * 31 + b;
        return static_cast<intcs>(h);
    }

    /**
     * @brief Returns a string representation of this SortKey.
     *
     * C++ counterpart of .NET SortKey.ToString().
     * @return A string in the form "SortKey - <original>".
     */
    [[nodiscard]] std::string ToString() const { return "SortKey - " + string_; }

private:
    std::string string_;
    std::vector<bytecs> keyData_;
};

} // namespace System::Globalization
