// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <cstddef>
#include <cctype>
#include "System/Collections/Generic/IEqualityComparer.hpp"

namespace System::Collections::Generic {

/**
 * @brief An IEqualityComparer<string> that uses a deterministic (non-randomized) hash algorithm.
 *
 * C++ counterpart of .NET System.Collections.Generic.NonRandomizedStringEqualityComparer.
 * Unlike the default string comparer, this class uses a stable FNV-1a hash that is not
 * seeded with a random value, making it safe to use in serialized or cross-process scenarios
 * where hash reproducibility is required.
 *
 * Use getOrdinalInstance() for case-sensitive comparison and
 * getOrdinalIgnoreCaseInstance() for case-insensitive comparison.
 */
class NonRandomizedStringEqualityComparer : public IEqualityComparer<std::string> {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~NonRandomizedStringEqualityComparer() = default;

    /**
     * @brief Determines whether two strings are equal using ordinal (byte-by-byte) comparison.
     *
     * C++ counterpart of .NET NonRandomizedStringEqualityComparer.Equals(string, string).
     * @param x The first string to compare.
     * @param y The second string to compare.
     * @return true if the strings are equal; otherwise false.
     */
    [[nodiscard]] bool Equals(const std::string& x, const std::string& y) const override {
        return x == y;
    }

    /**
     * @brief Returns a stable, non-randomized hash code for the specified string.
     *
     * C++ counterpart of .NET NonRandomizedStringEqualityComparer.GetHashCode(string).
     * Uses FNV-1a with a fixed seed, guaranteeing reproducible hash values across runs.
     * @param obj The string for which to compute the hash code.
     * @return A deterministic hash code.
     */
    [[nodiscard]] intcs GetHashCode(const std::string& obj) const override {
        return static_cast<intcs>(fnv1a(obj));
    }

    /**
     * @brief Returns the singleton case-sensitive (ordinal) comparer instance.
     *
     * C++ counterpart of .NET NonRandomizedStringEqualityComparer.WrappedAroundStringComparerOrdinal.
     * @return A const reference to the shared ordinal comparer.
     */
    [[nodiscard]] static const NonRandomizedStringEqualityComparer& getOrdinalInstance() {
        static NonRandomizedStringEqualityComparer instance;
        return instance;
    }

    /**
     * @brief Returns the singleton case-insensitive (ordinal ignore-case) comparer instance.
     *
     * C++ counterpart of .NET NonRandomizedStringEqualityComparer.WrappedAroundStringComparerOrdinalIgnoreCase.
     * @return A const reference to the shared ordinal-ignore-case comparer.
     */
    [[nodiscard]] static const NonRandomizedStringEqualityComparer& getOrdinalIgnoreCaseInstance() {
        static struct IgnoreCaseComparer : NonRandomizedStringEqualityComparer {
            [[nodiscard]] bool Equals(const std::string& x, const std::string& y) const override {
                if (x.size() != y.size()) return false;
                for (std::size_t i = 0; i < x.size(); ++i)
                    if (std::tolower(static_cast<unsigned char>(x[i])) !=
                        std::tolower(static_cast<unsigned char>(y[i]))) return false;
                return true;
            }
            [[nodiscard]] intcs GetHashCode(const std::string& obj) const override {
                std::string lower;
                lower.reserve(obj.size());
                for (unsigned char c : obj)
                    lower.push_back(static_cast<char>(std::tolower(c)));
                return static_cast<intcs>(fnv1a(lower));
            }
        } instance;
        return instance;
    }

private:
    static std::size_t fnv1a(const std::string& s) {
        std::size_t hash = 14695981039346656037ULL;
        for (unsigned char c : s) {
            hash ^= static_cast<std::size_t>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }
};

} // namespace System::Collections::Generic
