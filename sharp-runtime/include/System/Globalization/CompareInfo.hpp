// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Globalization/CompareOptions.hpp"
#include "System/Globalization/SortKey.hpp"

namespace System::Globalization {

using SharpRuntime::charcs;
using SharpRuntime::intcs;

/**
 * @brief Implements a set of methods for culture-sensitive string comparisons.
 *
 * C++ counterpart of .NET System.Globalization.CompareInfo.
 * Instances are obtained via the GetCompareInfo factory methods.
 * This implementation performs locale-insensitive comparisons with optional
 * case folding; full ICU-based locale comparison is not provided.
 */
class CompareInfo {
public:
    /**
     * @brief Constructs a CompareInfo for the given culture name.
     *
     * C++ counterpart of .NET CompareInfo obtained via CultureInfo.CompareInfo.
     * @param name The culture name (e.g. "en-US"); defaults to "en-US".
     */
    explicit CompareInfo(const std::string& name = "en-US") : name_(name) {}

    /**
     * @brief Returns a CompareInfo for the given culture name.
     *
     * C++ counterpart of .NET CompareInfo.GetCompareInfo(string).
     * @param name The culture name.
     * @return A CompareInfo instance for @p name.
     */
    static CompareInfo GetCompareInfo(const std::string& name) { return CompareInfo(name); }

    /**
     * @brief Returns a CompareInfo for the given LCID.
     *
     * C++ counterpart of .NET CompareInfo.GetCompareInfo(int).
     * @param culture The locale identifier; ignored — returns the default "en-US" instance.
     * @return A default CompareInfo instance.
     */
    static CompareInfo GetCompareInfo(intcs /*culture*/) { return CompareInfo("en-US"); }

    /**
     * @brief Gets the culture name for this CompareInfo.
     *
     * C++ counterpart of .NET CompareInfo.Name.
     * @return The culture name string.
     */
    [[nodiscard]] const std::string& getNameProperty() const { return name_; }

    /**
     * @brief Gets the locale identifier (LCID) for this CompareInfo.
     *
     * C++ counterpart of .NET CompareInfo.LCID.
     * @return Always 0 in this stub implementation.
     */
    [[nodiscard]] intcs getLCIDProperty() const { return 0; }

    /**
     * @brief Compares two strings using the specified options.
     *
     * C++ counterpart of .NET CompareInfo.Compare(string, string, CompareOptions).
     * @param s1      The first string.
     * @param s2      The second string.
     * @param options Comparison options (default None).
     * @return Negative if s1 < s2, zero if equal, positive if s1 > s2.
     */
    intcs Compare(const std::string& s1, const std::string& s2,
                  CompareOptions options = CompareOptions::None) const {
        if (isCaseInsensitive(options))
            return compareIgnoreCase(s1, s2);
        if (s1 < s2) return -1;
        if (s1 > s2) return  1;
        return 0;
    }

    /**
     * @brief Compares substrings of two strings using the specified options.
     *
     * C++ counterpart of .NET CompareInfo.Compare(string, int, int, string, int, int, CompareOptions).
     * @param s1      The first string.
     * @param off1    Starting offset in @p s1.
     * @param len1    Number of characters from @p s1.
     * @param s2      The second string.
     * @param off2    Starting offset in @p s2.
     * @param len2    Number of characters from @p s2.
     * @param options Comparison options (default None).
     * @return Negative, zero, or positive.
     */
    intcs Compare(const std::string& s1, intcs off1, intcs len1,
                  const std::string& s2, intcs off2, intcs len2,
                  CompareOptions options = CompareOptions::None) const {
        CheckRange(s1, off1, len1, "offset1");
        CheckRange(s2, off2, len2, "offset2");
        return Compare(s1.substr(static_cast<size_t>(off1), static_cast<size_t>(len1)),
                       s2.substr(static_cast<size_t>(off2), static_cast<size_t>(len2)), options);
    }

    /**
     * @brief Determines whether @p source starts with @p prefix using the specified options.
     *
     * C++ counterpart of .NET CompareInfo.IsPrefix(string, string, CompareOptions).
     * @param source  The string to search.
     * @param prefix  The prefix to test.
     * @param options Comparison options (default None).
     * @return true if @p source starts with @p prefix.
     */
    [[nodiscard]] bool IsPrefix(const std::string& source, const std::string& prefix,
                                CompareOptions options = CompareOptions::None) const {
        if (prefix.size() > source.size()) return false;
        return Compare(source, 0, static_cast<intcs>(prefix.size()),
                       prefix, 0, static_cast<intcs>(prefix.size()), options) == 0;
    }

    /**
     * @brief Determines whether @p source ends with @p suffix using the specified options.
     *
     * C++ counterpart of .NET CompareInfo.IsSuffix(string, string, CompareOptions).
     * @param source  The string to search.
     * @param suffix  The suffix to test.
     * @param options Comparison options (default None).
     * @return true if @p source ends with @p suffix.
     */
    [[nodiscard]] bool IsSuffix(const std::string& source, const std::string& suffix,
                                CompareOptions options = CompareOptions::None) const {
        if (suffix.size() > source.size()) return false;
        intcs off = static_cast<intcs>(source.size() - suffix.size());
        return Compare(source, off, static_cast<intcs>(suffix.size()),
                       suffix, 0, static_cast<intcs>(suffix.size()), options) == 0;
    }

    /**
     * @brief Searches for @p value in @p source and returns its zero-based index, or -1.
     *
     * C++ counterpart of .NET CompareInfo.IndexOf(string, string, CompareOptions).
     * @param source  The string to search within.
     * @param value   The string to locate.
     * @param options Comparison options (default None).
     * @return The zero-based index of the first occurrence, or -1 if not found.
     */
    [[nodiscard]] intcs IndexOf(const std::string& source, const std::string& value,
                                CompareOptions options = CompareOptions::None) const {
        if (isCaseInsensitive(options)) {
            std::string sl = toLower(source), vl = toLower(value);
            auto pos = sl.find(vl);
            return pos == std::string::npos ? -1 : static_cast<intcs>(pos);
        }
        auto pos = source.find(value);
        return pos == std::string::npos ? -1 : static_cast<intcs>(pos);
    }

    /**
     * @brief Searches for the last occurrence of @p value in @p source, or -1.
     *
     * C++ counterpart of .NET CompareInfo.LastIndexOf(string, string, CompareOptions).
     * @param source  The string to search within.
     * @param value   The string to locate.
     * @param options Comparison options (default None).
     * @return The zero-based index of the last occurrence, or -1 if not found.
     */
    [[nodiscard]] intcs LastIndexOf(const std::string& source, const std::string& value,
                                    CompareOptions options = CompareOptions::None) const {
        if (isCaseInsensitive(options)) {
            std::string sl = toLower(source), vl = toLower(value);
            auto pos = sl.rfind(vl);
            return pos == std::string::npos ? -1 : static_cast<intcs>(pos);
        }
        auto pos = source.rfind(value);
        return pos == std::string::npos ? -1 : static_cast<intcs>(pos);
    }

    /**
     * @brief Determines whether a character can be sorted.
     *
     * C++ counterpart of .NET CompareInfo.IsSortable(char).
     * @return Always true in this implementation.
     */
    static bool IsSortable(charcs /*ch*/) { return true; }

    /**
     * @brief Determines whether a string can be sorted.
     *
     * C++ counterpart of .NET CompareInfo.IsSortable(string).
     * @return Always true in this implementation.
     */
    static bool IsSortable(const std::string& /*text*/) { return true; }

    /**
     * @brief Gets the SortKey for a string using the specified comparison options.
     *
     * C++ counterpart of .NET CompareInfo.GetSortKey(string, CompareOptions).
     * This stub returns a byte-level key from the raw string bytes; when @p options
     * includes IgnoreCase, the key is derived from the case-folded string so that two
     * strings comparing equal under IgnoreCase also produce equal sort keys, matching
     * the .NET contract that GetSortKey results are consistent with Compare.
     * @param source  The string to create a sort key for.
     * @param options Comparison options (default None).
     * @return A SortKey object for @p source.
     */
    [[nodiscard]] SortKey GetSortKey(const std::string& source,
                                     CompareOptions options = CompareOptions::None) const {
        const std::string& keySource = isCaseInsensitive(options) ? toLower(source) : source;
        std::vector<bytecs> key(keySource.begin(), keySource.end());
        return SortKey(source, key);
    }

    /**
     * @brief Gets a hash code for the given string under the specified comparison options.
     *
     * C++ counterpart of .NET CompareInfo.GetHashCode(string, CompareOptions).
     * When @p options includes IgnoreCase, the hash is computed from the case-folded
     * string so that strings equal under IgnoreCase comparison also hash equal.
     * @param source  The source string.
     * @param options Comparison options.
     * @return A hash code derived from the string value.
     */
    [[nodiscard]] intcs GetHashCode(const std::string& source, CompareOptions options) const {
        const std::string& hashSource = isCaseInsensitive(options) ? toLower(source) : source;
        return static_cast<intcs>(std::hash<std::string>{}(hashSource));
    }

    /**
     * @brief Returns true if both CompareInfo instances have the same culture name.
     *
     * C++ counterpart of .NET CompareInfo.Equals(object).
     * @param other The CompareInfo to compare.
     * @return true if the culture names are equal.
     */
    bool operator==(const CompareInfo& other) const { return name_ == other.name_; }

    /**
     * @brief Returns a string representation of this CompareInfo.
     *
     * C++ counterpart of .NET CompareInfo.ToString().
     * @return A string in the form "CompareInfo - <name>".
     */
    [[nodiscard]] std::string ToString() const { return "CompareInfo - " + name_; }

private:
    std::string name_;

    static void CheckRange(const std::string& s, intcs offset, intcs length, const char* paramName) {
        if (offset < 0) throw System::ArgumentOutOfRangeException(paramName);
        if (length < 0) throw System::ArgumentOutOfRangeException("length");
        if (static_cast<size_t>(offset) > s.size() ||
            static_cast<size_t>(length) > s.size() - static_cast<size_t>(offset)) {
            throw System::ArgumentOutOfRangeException(paramName);
        }
    }

    static bool hasFlag(CompareOptions options, CompareOptions flag) {
        return (static_cast<int>(options) & static_cast<int>(flag)) != 0;
    }

    // .NET's real invariant-mode implementation (CompareInfo.Invariant.cs) treats a
    // comparison as case-insensitive when EITHER CompareOptions::IgnoreCase OR
    // CompareOptions::OrdinalIgnoreCase is set: `(options & (CompareOptions.IgnoreCase |
    // CompareOptions.OrdinalIgnoreCase)) != 0`. IgnoreCase and OrdinalIgnoreCase are
    // distinct, non-overlapping bits (0x1 vs 0x10000000), so checking only IgnoreCase here
    // would silently treat OrdinalIgnoreCase-only callers as case-sensitive.
    static bool isCaseInsensitive(CompareOptions options) {
        return hasFlag(options, CompareOptions::IgnoreCase) || hasFlag(options, CompareOptions::OrdinalIgnoreCase);
    }

    static std::string toLower(const std::string& s) {
        std::string r = s;
        for (auto& c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return r;
    }

    static intcs compareIgnoreCase(const std::string& a, const std::string& b) {
        std::string al = toLower(a), bl = toLower(b);
        if (al < bl) return -1;
        if (al > bl) return  1;
        return 0;
    }
};

} // namespace System::Globalization
