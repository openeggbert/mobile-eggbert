// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/StringComparison.hpp"
#include "System/StringSplitOptions.hpp"

namespace System
{
    /**
     * @brief Provides utility methods similar to selected members of .NET System.String.
     *
     * This class is not a wrapper around std::string. Instead, it offers
     * static helper methods useful for source-porting C# code to C++.
     * All methods mirror .NET String members with the same name and semantics.
     */
    class String
    {
    public:
        /** @brief Deleted constructor — all members are static. */
        String() = delete;
        /** @brief Deleted destructor — class is not instantiable. */
        ~String() = delete;

        // -----------------------------------------------------------------------
        // Testing / classification
        // -----------------------------------------------------------------------

        /**
         * @brief Returns true if @p value is empty.
         *
         * C++ counterpart of .NET String.IsNullOrEmpty where null maps to empty.
         */
        static bool IsEmpty(const std::string& value);

        /**
         * @brief Returns true if @p value is empty.
         *
         * C++ counterpart of .NET String.IsNullOrEmpty — null maps to empty string.
         */
        static bool IsNullOrEmpty(const std::string& value);

        /**
         * @brief Returns true if @p value consists only of whitespace characters (or is empty).
         *
         * C++ counterpart of .NET String.IsNullOrWhiteSpace.
         */
        static bool IsNullOrWhiteSpace(const std::string& value);

        // -----------------------------------------------------------------------
        // Search / comparison
        // -----------------------------------------------------------------------

        /**
         * @brief Returns true if @p value starts with @p prefix.
         *
         * C++ counterpart of .NET String.StartsWith(string).
         */
        static bool StartsWith(const std::string& value, const std::string& prefix);

        /**
         * @brief Returns true if @p value starts with the character @p ch.
         *
         * C++ counterpart of .NET String.StartsWith(char).
         */
        static bool StartsWith(const std::string& value, char ch);

        /**
         * @brief Returns true if @p value ends with @p suffix.
         *
         * C++ counterpart of .NET String.EndsWith(string).
         */
        static bool EndsWith(const std::string& value, const std::string& suffix);

        /**
         * @brief Returns true if @p value ends with the character @p ch.
         *
         * C++ counterpart of .NET String.EndsWith(char).
         */
        static bool EndsWith(const std::string& value, char ch);

        /**
         * @brief Returns true if @p value starts with @p prefix using the specified comparison rules.
         *
         * C++ counterpart of .NET String.StartsWith(string, StringComparison).
         */
        static bool StartsWith(const std::string& value, const std::string& prefix, StringComparison comparisonType);

        /**
         * @brief Returns true if @p value ends with @p suffix using the specified comparison rules.
         *
         * C++ counterpart of .NET String.EndsWith(string, StringComparison).
         */
        static bool EndsWith(const std::string& value, const std::string& suffix, StringComparison comparisonType);

        /**
         * @brief Returns true if @p value contains @p substr.
         *
         * C++ counterpart of .NET String.Contains(string).
         */
        static bool Contains(const std::string& value, const std::string& substr);

        /**
         * @brief Returns true if @p value contains the character @p ch.
         *
         * C++ counterpart of .NET String.Contains(char).
         */
        static bool Contains(const std::string& value, char ch);

        /**
         * @brief Returns true if @p value contains @p substr using the specified comparison rules.
         *
         * C++ counterpart of .NET String.Contains(string, StringComparison).
         */
        static bool Contains(const std::string& value, const std::string& substr, StringComparison comparisonType);

        /**
         * @brief Compares @p a and @p b ordinally; returns negative/zero/positive.
         *
         * C++ counterpart of .NET String.Compare(string, string).
         */
        static SharpRuntime::intcs Compare(const std::string& a, const std::string& b);

        /**
         * @brief Compares @p a and @p b with optional case-insensitivity.
         *
         * C++ counterpart of .NET String.Compare(string, string, bool).
         */
        static SharpRuntime::intcs Compare(const std::string& a, const std::string& b, bool ignoreCase);

        /**
         * @brief Compares @p a and @p b using the specified StringComparison rules.
         *
         * C++ counterpart of .NET String.Compare(string, string, StringComparison).
         */
        static SharpRuntime::intcs Compare(const std::string& a, const std::string& b, StringComparison comparisonType);

        /**
         * @brief Performs a byte-by-byte ordinal comparison of @p a and @p b.
         *
         * C++ counterpart of .NET String.CompareOrdinal(string, string).
         */
        static SharpRuntime::intcs CompareOrdinal(const std::string& a, const std::string& b);

        /**
         * @brief Returns true if @p a and @p b are equal (case-sensitive).
         *
         * C++ counterpart of .NET String.Equals(string, string).
         */
        static bool Equals(const std::string& a, const std::string& b);

        /**
         * @brief Returns true if @p a and @p b are equal with optional case-insensitivity.
         *
         * C++ counterpart of .NET String.Equals(string, string, bool).
         */
        static bool Equals(const std::string& a, const std::string& b, bool ignoreCase);

        /**
         * @brief Returns true if @p a and @p b are equal under the specified StringComparison rules.
         *
         * C++ counterpart of .NET String.Equals(string, string, StringComparison).
         */
        static bool Equals(const std::string& a, const std::string& b, StringComparison comparisonType);

        // -----------------------------------------------------------------------
        // IndexOf / LastIndexOf
        // -----------------------------------------------------------------------

        /**
         * @brief Returns the zero-based index of the first occurrence of @p substr in @p value, or -1.
         *
         * C++ counterpart of .NET String.IndexOf(string).
         */
        static SharpRuntime::intcs IndexOf(const std::string& value, const std::string& substr);

        /**
         * @brief Returns the zero-based index of the first occurrence of @p ch in @p value, or -1.
         *
         * C++ counterpart of .NET String.IndexOf(char).
         */
        static SharpRuntime::intcs IndexOf(const std::string& value, char ch);

        /**
         * @brief Returns the first occurrence of @p substr starting at @p startIndex, or -1.
         *
         * C++ counterpart of .NET String.IndexOf(string, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of bounds.
         */
        static SharpRuntime::intcs IndexOf(const std::string& value, const std::string& substr, SharpRuntime::intcs startIndex);

        /**
         * @brief Returns the first occurrence of @p ch starting at @p startIndex, or -1.
         *
         * C++ counterpart of .NET String.IndexOf(char, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of bounds.
         */
        static SharpRuntime::intcs IndexOf(const std::string& value, char ch, SharpRuntime::intcs startIndex);

        /**
         * @brief Returns the first occurrence of @p ch in the range [@p startIndex, @p startIndex+@p count), or -1.
         *
         * C++ counterpart of .NET String.IndexOf(char, int, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count are out of bounds.
         */
        static SharpRuntime::intcs IndexOf(const std::string& value, char ch, SharpRuntime::intcs startIndex, SharpRuntime::intcs count);

        /**
         * @brief Returns the first occurrence of @p substr in the range [@p startIndex, @p startIndex+@p count), or -1.
         *
         * C++ counterpart of .NET String.IndexOf(string, int, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count are out of bounds.
         */
        static SharpRuntime::intcs IndexOf(const std::string& value, const std::string& substr, SharpRuntime::intcs startIndex, SharpRuntime::intcs count);

        /**
         * @brief Returns the zero-based index of the first occurrence of any character in @p anyOf, or -1.
         *
         * C++ counterpart of .NET String.IndexOfAny(char[]).
         */
        static SharpRuntime::intcs IndexOfAny(const std::string& value, const std::vector<char>& anyOf);

        /**
         * @brief Returns the first occurrence of @p substr using the specified comparison rules, or -1.
         *
         * C++ counterpart of .NET String.IndexOf(string, StringComparison).
         */
        static SharpRuntime::intcs IndexOf(const std::string& value, const std::string& substr, StringComparison comparisonType);

        /**
         * @brief Returns the last occurrence of @p substr using the specified comparison rules, or -1.
         *
         * C++ counterpart of .NET String.LastIndexOf(string, StringComparison).
         */
        static SharpRuntime::intcs LastIndexOf(const std::string& value, const std::string& substr, StringComparison comparisonType);

        /**
         * @brief Returns the zero-based index of the last occurrence of @p substr in @p value, or -1.
         *
         * C++ counterpart of .NET String.LastIndexOf(string).
         */
        static SharpRuntime::intcs LastIndexOf(const std::string& value, const std::string& substr);

        /**
         * @brief Returns the zero-based index of the last occurrence of @p ch in @p value, or -1.
         *
         * C++ counterpart of .NET String.LastIndexOf(char).
         */
        static SharpRuntime::intcs LastIndexOf(const std::string& value, char ch);

        /**
         * @brief Returns the last occurrence of @p substr searching backward from @p startIndex, or -1.
         *
         * C++ counterpart of .NET String.LastIndexOf(string, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of bounds.
         */
        static SharpRuntime::intcs LastIndexOf(const std::string& value, const std::string& substr, SharpRuntime::intcs startIndex);

        /**
         * @brief Returns the last occurrence of @p ch searching backward from @p startIndex, or -1.
         *
         * C++ counterpart of .NET String.LastIndexOf(char, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of bounds.
         */
        static SharpRuntime::intcs LastIndexOf(const std::string& value, char ch, SharpRuntime::intcs startIndex);

        /**
         * @brief Returns the last occurrence of @p ch in @p count characters ending at @p startIndex, or -1.
         *
         * C++ counterpart of .NET String.LastIndexOf(char, int, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count are out of bounds.
         */
        static SharpRuntime::intcs LastIndexOf(const std::string& value, char ch, SharpRuntime::intcs startIndex, SharpRuntime::intcs count);

        /**
         * @brief Returns the last occurrence of @p substr in @p count characters ending at @p startIndex, or -1.
         *
         * C++ counterpart of .NET String.LastIndexOf(string, int, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count are out of bounds.
         */
        static SharpRuntime::intcs LastIndexOf(const std::string& value, const std::string& substr, SharpRuntime::intcs startIndex, SharpRuntime::intcs count);

        /**
         * @brief Returns the zero-based index of the last occurrence of any character in @p anyOf, or -1.
         *
         * C++ counterpart of .NET String.LastIndexOfAny(char[]).
         */
        static SharpRuntime::intcs LastIndexOfAny(const std::string& value, const std::vector<char>& anyOf);

        // -----------------------------------------------------------------------
        // Splitting
        // -----------------------------------------------------------------------

        /**
         * @brief Splits @p value on a single character delimiter.
         *
         * C++ counterpart of .NET String.Split(char). An empty @p value returns a single-element
         * result containing that empty string (`{""}`), matching real .NET's
         * CreateSplitArrayOfThisAsSoleValue short-circuit for a zero-length input -- verified
         * against String.Manipulation.cs.
         */
        static std::vector<std::string> Split(const std::string& value, char delimiter);

        /**
         * @brief Splits @p value on any character in @p delimiters.
         *
         * C++ counterpart of .NET String.Split(char[]).
         */
        static std::vector<std::string> Split(const std::string& value, const std::vector<char>& delimiters);

        /**
         * @brief Splits @p value on a string delimiter.
         *
         * C++ counterpart of .NET String.Split(string).
         */
        static std::vector<std::string> Split(const std::string& value, const std::string& delimiter);

        /**
         * @brief Splits @p value on @p delimiter with the specified @p options.
         *
         * C++ counterpart of .NET String.Split(char, StringSplitOptions).
         */
        static std::vector<std::string> Split(const std::string& value, char delimiter, StringSplitOptions options);

        /**
         * @brief Splits @p value on @p delimiter string with the specified @p options.
         *
         * C++ counterpart of .NET String.Split(string, StringSplitOptions).
         */
        static std::vector<std::string> Split(const std::string& value, const std::string& delimiter, StringSplitOptions options);

        // -----------------------------------------------------------------------
        // Substring / modification
        // -----------------------------------------------------------------------

        /**
         * @brief Returns the substring of @p value starting at @p startIndex to the end.
         *
         * C++ counterpart of .NET String.Substring(int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of bounds.
         */
        static std::string Substring(const std::string& value, SharpRuntime::intcs startIndex);

        /**
         * @brief Returns the substring of @p value starting at @p startIndex with the given @p length.
         *
         * C++ counterpart of .NET String.Substring(int, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p length are out of bounds.
         */
        static std::string Substring(const std::string& value, SharpRuntime::intcs startIndex, SharpRuntime::intcs length);

        /**
         * @brief Returns a new string with all characters from @p startIndex to the end removed.
         *
         * C++ counterpart of .NET String.Remove(int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of bounds.
         */
        static std::string Remove(const std::string& value, SharpRuntime::intcs startIndex);

        /**
         * @brief Returns a new string with @p count characters removed starting at @p startIndex.
         *
         * C++ counterpart of .NET String.Remove(int, int).
         * @throws System::ArgumentOutOfRangeException if @p startIndex or @p count are out of bounds.
         */
        static std::string Remove(const std::string& value, SharpRuntime::intcs startIndex, SharpRuntime::intcs count);

        /**
         * @brief Inserts @p insertValue into @p value at @p startIndex.
         *
         * C++ counterpart of .NET String.Insert(int, string).
         * @throws System::ArgumentOutOfRangeException if @p startIndex is out of bounds.
         */
        static std::string Insert(const std::string& value, SharpRuntime::intcs startIndex, const std::string& insertValue);

        /**
         * @brief Returns a copy of @p value with every occurrence of @p oldValue replaced by @p newValue.
         *
         * C++ counterpart of .NET String.Replace(string, string).
         */
        static std::string Replace(const std::string& value, const std::string& oldValue, const std::string& newValue);

        /**
         * @brief Returns a copy of @p value with every occurrence of @p oldChar replaced by @p newChar.
         *
         * C++ counterpart of .NET String.Replace(char, char).
         */
        static std::string Replace(const std::string& value, char oldChar, char newChar);

        // -----------------------------------------------------------------------
        // Trim / pad
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a copy of @p value with leading and trailing whitespace removed.
         *
         * C++ counterpart of .NET String.Trim().
         */
        static std::string Trim(const std::string& value);

        /**
         * @brief Returns a copy of @p value with leading and trailing characters in @p trimChars removed.
         *
         * C++ counterpart of .NET String.Trim(char[]).
         */
        static std::string Trim(const std::string& value, const std::vector<char>& trimChars);

        /**
         * @brief Returns a copy of @p value with leading whitespace removed.
         *
         * C++ counterpart of .NET String.TrimStart().
         */
        static std::string TrimStart(const std::string& value);

        /**
         * @brief Returns a copy of @p value with leading characters in @p trimChars removed.
         *
         * C++ counterpart of .NET String.TrimStart(char[]).
         */
        static std::string TrimStart(const std::string& value, const std::vector<char>& trimChars);

        /**
         * @brief Returns a copy of @p value with trailing whitespace removed.
         *
         * C++ counterpart of .NET String.TrimEnd().
         */
        static std::string TrimEnd(const std::string& value);

        /**
         * @brief Returns a copy of @p value with trailing characters in @p trimChars removed.
         *
         * C++ counterpart of .NET String.TrimEnd(char[]).
         */
        static std::string TrimEnd(const std::string& value, const std::vector<char>& trimChars);

        /**
         * @brief Returns @p value right-aligned in a field of @p totalWidth, padded with spaces on the left.
         *
         * C++ counterpart of .NET String.PadLeft(int).
         * @throws System::ArgumentOutOfRangeException if @p totalWidth is negative.
         */
        static std::string PadLeft(const std::string& value, SharpRuntime::intcs totalWidth);

        /**
         * @brief Returns @p value right-aligned in a field of @p totalWidth, padded with @p paddingChar on the left.
         *
         * C++ counterpart of .NET String.PadLeft(int, char).
         * @throws System::ArgumentOutOfRangeException if @p totalWidth is negative.
         */
        static std::string PadLeft(const std::string& value, SharpRuntime::intcs totalWidth, char paddingChar);

        /**
         * @brief Returns @p value left-aligned in a field of @p totalWidth, padded with spaces on the right.
         *
         * C++ counterpart of .NET String.PadRight(int).
         * @throws System::ArgumentOutOfRangeException if @p totalWidth is negative.
         */
        static std::string PadRight(const std::string& value, SharpRuntime::intcs totalWidth);

        /**
         * @brief Returns @p value left-aligned in a field of @p totalWidth, padded with @p paddingChar on the right.
         *
         * C++ counterpart of .NET String.PadRight(int, char).
         * @throws System::ArgumentOutOfRangeException if @p totalWidth is negative.
         */
        static std::string PadRight(const std::string& value, SharpRuntime::intcs totalWidth, char paddingChar);

        // -----------------------------------------------------------------------
        // Case conversion
        // -----------------------------------------------------------------------

        /**
         * @brief Returns a copy of @p value with all characters converted to uppercase.
         *
         * C++ counterpart of .NET String.ToUpper().
         */
        static std::string ToUpper(const std::string& value);

        /**
         * @brief Returns a locale-invariant uppercase copy of @p value.
         *
         * C++ counterpart of .NET String.ToUpperInvariant().
         */
        static std::string ToUpperInvariant(const std::string& value);

        /**
         * @brief Returns a copy of @p value with all characters converted to lowercase.
         *
         * C++ counterpart of .NET String.ToLower().
         */
        static std::string ToLower(const std::string& value);

        /**
         * @brief Returns a locale-invariant lowercase copy of @p value.
         *
         * C++ counterpart of .NET String.ToLowerInvariant().
         */
        static std::string ToLowerInvariant(const std::string& value);

        // -----------------------------------------------------------------------
        // Concatenation / joining
        // -----------------------------------------------------------------------

        /**
         * @brief Concatenates two strings.
         *
         * C++ counterpart of .NET String.Concat(string, string).
         */
        static std::string Concat(const std::string& a, const std::string& b);

        /**
         * @brief Concatenates three strings.
         *
         * C++ counterpart of .NET String.Concat(string, string, string).
         */
        static std::string Concat(const std::string& a, const std::string& b, const std::string& c);

        /**
         * @brief Concatenates four strings.
         *
         * C++ counterpart of .NET String.Concat(string, string, string, string).
         */
        static std::string Concat(const std::string& a, const std::string& b, const std::string& c, const std::string& d);

        /**
         * @brief Concatenates all strings in @p values with no separator.
         *
         * C++ counterpart of .NET String.Concat(IEnumerable&lt;string&gt;).
         */
        static std::string Concat(const std::vector<std::string>& values);

        /**
         * @brief Joins all elements of @p values with @p separator between them.
         *
         * C++ counterpart of .NET String.Join(string, IEnumerable&lt;string&gt;).
         */
        static std::string Join(const std::string& separator, const std::vector<std::string>& values);

        /**
         * @brief Joins all strings in @p values (initializer_list form) with @p separator.
         *
         * C++ counterpart of .NET String.Join(string, string[]).
         */
        static std::string Join(const std::string& separator, std::initializer_list<std::string> values)
        {
            return Join(separator, std::vector<std::string>(values));
        }

        /**
         * @brief Joins all strings in @p values with a single-character @p separator between them.
         *
         * C++ counterpart of .NET String.Join(char, string[]).
         */
        static std::string Join(char separator, const std::vector<std::string>& values);

        /**
         * @brief Joins all integers in @p values with @p separator between them.
         *
         * C++ counterpart of .NET String.Join(string, IEnumerable&lt;int&gt;).
         */
        static std::string Join(const std::string& separator, const std::vector<SharpRuntime::intcs>& values);

        /**
         * @brief Joins all doubles in @p values with @p separator between them.
         *
         * C++ counterpart of .NET String.Join(string, IEnumerable&lt;double&gt;).
         */
        static std::string Join(const std::string& separator, const std::vector<double>& values);

        // -----------------------------------------------------------------------
        // Char array / conversion
        // -----------------------------------------------------------------------

        /**
         * @brief Returns the characters of @p value as a vector of chars.
         *
         * C++ counterpart of .NET String.ToCharArray().
         */
        static std::vector<char> ToCharArray(const std::string& value);

        /**
         * @brief Returns @p length characters of @p value starting at @p startIndex.
         *
         * C++ counterpart of .NET String.ToCharArray(int, int).
         * @throws System::ArgumentOutOfRangeException if startIndex or length are out of bounds.
         */
        static std::vector<char> ToCharArray(const std::string& value, SharpRuntime::intcs startIndex, SharpRuntime::intcs length);

        /**
         * @brief Returns a new string of @p count copies of character @p ch.
         *
         * C++ counterpart of .NET new string(char, int).
         * @throws System::ArgumentOutOfRangeException if @p count is negative.
         */
        static std::string Create(SharpRuntime::intcs count, char ch);

        /**
         * @brief Converts an integer to a zero-padded string of at least @p width characters.
         *
         * @param value Integer value to convert.
         * @param width Minimum field width.
         * @param fill  Padding character (default '0').
         */
        static std::string ToString(SharpRuntime::intcs value, SharpRuntime::intcs width, char fill = '0');

        /**
         * @brief Returns a hash code for @p value using std::hash&lt;std::string&gt;.
         *
         * C++ counterpart of .NET String.GetHashCode().
         */
        static SharpRuntime::intcs GetHashCode(const std::string& value) noexcept;

        // -----------------------------------------------------------------------
        // Formatting
        //
        // Every Format() overload below throws System::FormatException if @p format references
        // an argument index at or beyond the number of arguments this overload accepts (e.g.
        // "{5}" when only arg0 was supplied), or contains a malformed placeholder (an unclosed
        // '{', or a '{' not followed by a digit) -- matching real .NET's String.Format, which
        // throws rather than silently leaving an unresolved placeholder in the output. Verified
        // against String.Manipulation.cs's FormatHelper, which validates every placeholder's
        // index against the supplied argument count before formatting anything.
        // -----------------------------------------------------------------------

        /** @brief Formats a string replacing `{0}` with @p arg0 (supports `{0:X}`, `{0:D3}` specifiers). */
        static std::string Format(const std::string& format, SharpRuntime::intcs arg0);
        /** @brief Formats a string replacing `{0}` with @p arg0 (supports `{0:F2}`, `{0:G}` specifiers). */
        static std::string Format(const std::string& format, double arg0);
        /** @brief Formats a string replacing `{0}` with @p arg0 (plain string substitution). */
        static std::string Format(const std::string& format, const std::string& arg0);
        /** @brief Formats a string replacing `{0}` with @p arg0 (single-precision float). */
        static std::string Format(const std::string& format, float arg0);
        /** @brief Formats a string replacing `{0}` with @p arg0 as "True" or "False". */
        static std::string Format(const std::string& format, bool arg0);
        /** @brief Formats a string replacing `{0}` with @p arg0 as a single character. */
        static std::string Format(const std::string& format, char arg0);
        /** @brief Formats a string replacing `{0}` with @p arg0 (long integer). */
        static std::string Format(const std::string& format, SharpRuntime::longcs arg0);
        /** @brief Formats a string with two integer arguments (`{0}` and `{1}`). */
        static std::string Format(const std::string& format, SharpRuntime::intcs arg0, SharpRuntime::intcs arg1);
        /** @brief Formats a string with an integer arg0 and string arg1. */
        static std::string Format(const std::string& format, SharpRuntime::intcs arg0, const std::string& arg1);
        /** @brief Formats a string with a string arg0 and integer arg1. */
        static std::string Format(const std::string& format, const std::string& arg0, SharpRuntime::intcs arg1);
        /** @brief Formats a string with two string arguments. */
        static std::string Format(const std::string& format, const std::string& arg0, const std::string& arg1);
        /** @brief Formats a string with two double arguments (supports `{0:F2}` specifiers). */
        static std::string Format(const std::string& format, double arg0, double arg1);
        /** @brief Formats a string with a double arg0 and string arg1. */
        static std::string Format(const std::string& format, double arg0, const std::string& arg1);
        /** @brief Formats a string with a string arg0 and double arg1. */
        static std::string Format(const std::string& format, const std::string& arg0, double arg1);
        /** @brief Formats a string with a longcs arg0 and intcs arg1. */
        static std::string Format(const std::string& format, SharpRuntime::longcs arg0, SharpRuntime::intcs arg1);
        /** @brief Formats a string with an intcs arg0 and longcs arg1. */
        static std::string Format(const std::string& format, SharpRuntime::intcs arg0, SharpRuntime::longcs arg1);
        /** @brief Formats a string with an intcs arg0 and double arg1. */
        static std::string Format(const std::string& format, SharpRuntime::intcs arg0, double arg1);
        /** @brief Formats a string with a double arg0 and intcs arg1. */
        static std::string Format(const std::string& format, double arg0, SharpRuntime::intcs arg1);
        /** @brief Formats a string with three integer arguments. */
        static std::string Format(const std::string& format, SharpRuntime::intcs arg0, SharpRuntime::intcs arg1, SharpRuntime::intcs arg2);
        /** @brief Formats a string with three string arguments. */
        static std::string Format(const std::string& format, const std::string& arg0, const std::string& arg1, const std::string& arg2);
        /** @brief Formats a string with two long integer arguments. */
        static std::string Format(const std::string& format, SharpRuntime::longcs arg0, SharpRuntime::longcs arg1);
        /** @brief Formats a string with four string arguments. */
        static std::string Format(const std::string& format,
                                  const std::string& arg0, const std::string& arg1,
                                  const std::string& arg2, const std::string& arg3);

        // -----------------------------------------------------------------------
        // Interning (stub — C++ has no string intern pool)
        // -----------------------------------------------------------------------

        /**
         * @brief Retrieves the system's reference to the specified string.
         *
         * C++ counterpart of .NET String.Intern(string). C++ has no managed
         * string intern pool, so this stub returns @p str unchanged.
         * @param str The string to intern.
         * @return The same string @p str.
         */
        static std::string Intern(const std::string& str) { return str; }

        /**
         * @brief Returns a reference to a known interned string, if it exists.
         *
         * C++ counterpart of .NET String.IsInterned(string). C++ has no managed
         * string intern pool, so this stub always returns @p str (never null).
         * @param str The string to look up.
         * @return The same string @p str.
         */
        static std::string IsInterned(const std::string& str) { return str; }

        // -----------------------------------------------------------------------
        // Constants
        // -----------------------------------------------------------------------

        /**
         * @brief The empty string constant — equivalent to .NET String.Empty.
         */
        static const std::string Empty;
    };

} // namespace System
