// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cctype>
#include <cwctype>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Globalization/CharUnicodeInfo.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"

namespace System {

/**
 * @brief Provides constants and static methods for working with Unicode characters.
 *
 * C++ counterpart of .NET System.Char.
 * Characters are represented as char16_t (SharpRuntime::charcs) code units.
 */
class Char {
public:
    /** @brief Deleted constructor — all members are static. */
    Char() = delete;

    /** @brief The maximum value of a Char (U+FFFF). C++ counterpart of .NET Char.MaxValue. */
    static constexpr SharpRuntime::charcs MaxValue = 0xFFFFu;

    /** @brief The minimum value of a Char (U+0000). C++ counterpart of .NET Char.MinValue. */
    static constexpr SharpRuntime::charcs MinValue = u'\0';

    // -----------------------------------------------------------------------
    // Range helper
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if @p c is within the inclusive range [@p minInclusive, @p maxInclusive].
     *
     * C++ counterpart of .NET Char.IsBetween(char, char, char).
     */
    static bool IsBetween(SharpRuntime::charcs c,
                          SharpRuntime::charcs minInclusive,
                          SharpRuntime::charcs maxInclusive) noexcept {
        return static_cast<uint16_t>(c - minInclusive) <=
               static_cast<uint16_t>(maxInclusive - minInclusive);
    }

    // -----------------------------------------------------------------------
    // Classification — single-character overloads
    // -----------------------------------------------------------------------

    /** @brief Returns true if @p c is a Unicode letter. */
    static bool IsLetter(SharpRuntime::charcs c) { return std::iswalpha(static_cast<wint_t>(c)) != 0; }

    /**
     * @brief Returns true if @p c is a decimal digit.
     *
     * C++ counterpart of .NET Char.IsDigit(char).
     */
    static bool IsDigit(SharpRuntime::charcs c) { return std::iswdigit(static_cast<wint_t>(c)) != 0; }

    /** @brief Returns true if @p c is a letter or decimal digit. */
    static bool IsLetterOrDigit(SharpRuntime::charcs c) { return IsLetter(c) || IsDigit(c); }

    /**
     * @brief Returns true if @p c is white space.
     *
     * C++ counterpart of .NET Char.IsWhiteSpace(char).
     */
    static bool IsWhiteSpace(SharpRuntime::charcs c) { return std::iswspace(static_cast<wint_t>(c)) != 0; }

    /**
     * @brief Returns true if @p c is an uppercase letter.
     *
     * C++ counterpart of .NET Char.IsUpper(char).
     */
    static bool IsUpper(SharpRuntime::charcs c) { return std::iswupper(static_cast<wint_t>(c)) != 0; }

    /**
     * @brief Returns true if @p c is a lowercase letter.
     *
     * C++ counterpart of .NET Char.IsLower(char).
     */
    static bool IsLower(SharpRuntime::charcs c) { return std::iswlower(static_cast<wint_t>(c)) != 0; }

    /**
     * @brief Returns true if @p c is a punctuation character.
     *
     * C++ counterpart of .NET Char.IsPunctuation(char).
     */
    static bool IsPunctuation(SharpRuntime::charcs c) { return std::iswpunct(static_cast<wint_t>(c)) != 0; }

    /**
     * @brief Returns true if @p c is a symbol character.
     *
     * C++ counterpart of .NET Char.IsSymbol(char).
     * Approximation: printable characters that are not letter/digit/space/punctuation/control.
     */
    static bool IsSymbol(SharpRuntime::charcs c) {
        return !IsLetter(c) && !IsDigit(c) && !IsWhiteSpace(c) &&
               !IsPunctuation(c) && !IsControl(c) &&
               std::iswprint(static_cast<wint_t>(c));
    }

    /**
     * @brief Returns true if @p c is a control character (U+0000–U+001F or U+007F–U+009F).
     *
     * C++ counterpart of .NET Char.IsControl(char).
     */
    static bool IsControl(SharpRuntime::charcs c) noexcept {
        return (((static_cast<uint32_t>(c) + 1u) & ~0x80u) <= 0x20u);
    }

    /**
     * @brief Returns true if @p c is a numeric character (decimal digit, letter number, or other number).
     *
     * C++ counterpart of .NET Char.IsNumber(char).
     * More inclusive than IsDigit: includes fractions and letter-number characters.
     */
    static bool IsNumber(SharpRuntime::charcs c) {
        using Globalization::UnicodeCategory;
        auto cat = Globalization::CharUnicodeInfo::GetUnicodeCategory(c);
        return cat == UnicodeCategory::DecimalDigitNumber
            || cat == UnicodeCategory::LetterNumber
            || cat == UnicodeCategory::OtherNumber;
    }

    /**
     * @brief Returns true if @p c is a Unicode separator (space, line, or paragraph separator).
     *
     * C++ counterpart of .NET Char.IsSeparator(char).
     * Note: tab and newline are control characters, not separators.
     */
    static bool IsSeparator(SharpRuntime::charcs c) {
        if (c <= 0x00FFu) {
            return c == u' ' || c == static_cast<SharpRuntime::charcs>(0x00A0u);
        }
        using Globalization::UnicodeCategory;
        auto cat = Globalization::CharUnicodeInfo::GetUnicodeCategory(c);
        return cat == UnicodeCategory::SpaceSeparator
            || cat == UnicodeCategory::LineSeparator
            || cat == UnicodeCategory::ParagraphSeparator;
    }

    // -----------------------------------------------------------------------
    // ASCII helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if @p c is in the ASCII range (U+0000–U+007F).
     *
     * C++ counterpart of .NET Char.IsAscii(char).
     */
    static bool IsAscii(SharpRuntime::charcs c) noexcept { return c < 0x80u; }

    /** @brief Returns true if @p c is an ASCII decimal digit ('0'–'9'). */
    static bool IsAsciiDigit(SharpRuntime::charcs c) noexcept { return c >= u'0' && c <= u'9'; }

    /** @brief Returns true if @p c is an ASCII uppercase letter ('A'–'Z'). Helper for IsAsciiLetterUpper. */
    static bool IsAsciiUpper(SharpRuntime::charcs c) noexcept { return c >= u'A' && c <= u'Z'; }

    /** @brief Returns true if @p c is an ASCII lowercase letter ('a'–'z'). Helper for IsAsciiLetterLower. */
    static bool IsAsciiLower(SharpRuntime::charcs c) noexcept { return c >= u'a' && c <= u'z'; }

    /**
     * @brief Returns true if @p c is an ASCII letter ('A'–'Z' or 'a'–'z').
     *
     * C++ counterpart of .NET Char.IsAsciiLetter(char).
     */
    static bool IsAsciiLetter(SharpRuntime::charcs c) noexcept { return IsAsciiUpper(c) || IsAsciiLower(c); }

    /**
     * @brief Returns true if @p c is an ASCII letter or decimal digit.
     *
     * C++ counterpart of .NET Char.IsAsciiLetterOrDigit(char).
     */
    static bool IsAsciiLetterOrDigit(SharpRuntime::charcs c) noexcept { return IsAsciiLetter(c) || IsAsciiDigit(c); }

    /**
     * @brief Returns true if @p c is an ASCII uppercase letter ('A'–'Z').
     *
     * C++ counterpart of .NET Char.IsAsciiLetterUpper(char).
     */
    static bool IsAsciiLetterUpper(SharpRuntime::charcs c) noexcept { return IsAsciiUpper(c); }

    /**
     * @brief Returns true if @p c is an ASCII lowercase letter ('a'–'z').
     *
     * C++ counterpart of .NET Char.IsAsciiLetterLower(char).
     */
    static bool IsAsciiLetterLower(SharpRuntime::charcs c) noexcept { return IsAsciiLower(c); }

    /**
     * @brief Returns true if @p c is a valid ASCII hexadecimal digit (0–9, A–F, a–f).
     *
     * C++ counterpart of .NET Char.IsAsciiHexDigit(char).
     */
    static bool IsAsciiHexDigit(SharpRuntime::charcs c) noexcept {
        return IsAsciiDigit(c) || (c >= u'A' && c <= u'F') || (c >= u'a' && c <= u'f');
    }

    /**
     * @brief Returns true if @p c is a lowercase ASCII hex digit (0–9, a–f).
     *
     * C++ counterpart of .NET Char.IsAsciiHexDigitLower(char).
     */
    static bool IsAsciiHexDigitLower(SharpRuntime::charcs c) noexcept {
        return IsAsciiDigit(c) || (c >= u'a' && c <= u'f');
    }

    /**
     * @brief Returns true if @p c is an uppercase ASCII hex digit (0–9, A–F).
     *
     * C++ counterpart of .NET Char.IsAsciiHexDigitUpper(char).
     */
    static bool IsAsciiHexDigitUpper(SharpRuntime::charcs c) noexcept {
        return IsAsciiDigit(c) || (c >= u'A' && c <= u'F');
    }

    // -----------------------------------------------------------------------
    // Surrogate helpers — single-character overloads
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if @p c is a Unicode high surrogate (U+D800–U+DBFF).
     *
     * C++ counterpart of .NET Char.IsHighSurrogate(char).
     */
    static bool IsHighSurrogate(SharpRuntime::charcs c) noexcept { return c >= 0xD800u && c <= 0xDBFFu; }

    /**
     * @brief Returns true if @p c is a Unicode low surrogate (U+DC00–U+DFFF).
     *
     * C++ counterpart of .NET Char.IsLowSurrogate(char).
     */
    static bool IsLowSurrogate(SharpRuntime::charcs c) noexcept { return c >= 0xDC00u && c <= 0xDFFFu; }

    /**
     * @brief Returns true if @p c is any surrogate (U+D800–U+DFFF).
     *
     * C++ counterpart of .NET Char.IsSurrogate(char).
     */
    static bool IsSurrogate(SharpRuntime::charcs c) noexcept { return c >= 0xD800u && c <= 0xDFFFu; }

    /**
     * @brief Returns true if @p high and @p low form a valid surrogate pair.
     *
     * C++ counterpart of .NET Char.IsSurrogatePair(char, char).
     */
    static bool IsSurrogatePair(SharpRuntime::charcs high, SharpRuntime::charcs low) noexcept {
        return IsHighSurrogate(high) && IsLowSurrogate(low);
    }

    // -----------------------------------------------------------------------
    // Case conversion
    // -----------------------------------------------------------------------

    /**
     * @brief Converts @p c to its uppercase equivalent using the current culture.
     *
     * C++ counterpart of .NET Char.ToUpper(char).
     */
    static SharpRuntime::charcs ToUpper(SharpRuntime::charcs c) {
        return static_cast<SharpRuntime::charcs>(std::towupper(static_cast<wint_t>(c)));
    }

    /**
     * @brief Converts @p c to its lowercase equivalent using the current culture.
     *
     * C++ counterpart of .NET Char.ToLower(char).
     */
    static SharpRuntime::charcs ToLower(SharpRuntime::charcs c) {
        return static_cast<SharpRuntime::charcs>(std::towlower(static_cast<wint_t>(c)));
    }

    /**
     * @brief Converts @p c to uppercase using the invariant culture.
     *
     * C++ counterpart of .NET Char.ToUpperInvariant(char).
     */
    static SharpRuntime::charcs ToUpperInvariant(SharpRuntime::charcs c) noexcept {
        if (c >= u'a' && c <= u'z') return static_cast<SharpRuntime::charcs>(c - (u'a' - u'A'));
        return c;
    }

    /**
     * @brief Converts @p c to lowercase using the invariant culture.
     *
     * C++ counterpart of .NET Char.ToLowerInvariant(char).
     */
    static SharpRuntime::charcs ToLowerInvariant(SharpRuntime::charcs c) noexcept {
        if (c >= u'A' && c <= u'Z') return static_cast<SharpRuntime::charcs>(c + (u'a' - u'A'));
        return c;
    }

    // -----------------------------------------------------------------------
    // Comparison
    // -----------------------------------------------------------------------

    /**
     * @brief Compares two Char values.
     *
     * C++ counterpart of .NET Char.CompareTo(char).
     * @return Negative if a < b, zero if equal, positive if a > b.
     */
    [[nodiscard]] static SharpRuntime::intcs CompareTo(SharpRuntime::charcs a,
                                                       SharpRuntime::charcs b) noexcept {
        return static_cast<SharpRuntime::intcs>(a) - static_cast<SharpRuntime::intcs>(b);
    }

    /**
     * @brief Returns true if @p a equals @p b.
     *
     * C++ counterpart of .NET Char.Equals(char).
     */
    [[nodiscard]] static bool Equals(SharpRuntime::charcs a, SharpRuntime::charcs b) noexcept {
        return a == b;
    }

    // -----------------------------------------------------------------------
    // Hash
    // -----------------------------------------------------------------------

    /**
     * @brief Returns a hash code for @p c.
     *
     * C++ counterpart of .NET Char.GetHashCode().
     * Uses the same formula as .NET: (int)c | ((int)c << 16).
     */
    [[nodiscard]] static SharpRuntime::intcs GetHashCode(SharpRuntime::charcs c) noexcept {
        auto v = static_cast<SharpRuntime::intcs>(c);
        return v | (v << 16);
    }

    // -----------------------------------------------------------------------
    // Parse / TryParse / ToString
    // -----------------------------------------------------------------------

    /**
     * @brief Parses a UTF-8 string that contains exactly one Unicode BMP code point.
     *
     * C++ counterpart of .NET Char.Parse(string).
     * @throws System::FormatException for empty, multi-char, invalid UTF-8, or non-BMP input.
     */
    static SharpRuntime::charcs Parse(const std::string& s) {
        if (s.empty()) throw System::FormatException("String must be exactly one character long.");
        auto b0 = static_cast<unsigned char>(s[0]);
        uint32_t cp;
        size_t   bytes;
        if      (b0 < 0x80u) { cp = b0;        bytes = 1; }
        else if (b0 < 0xE0u) { cp = b0 & 0x1F; bytes = 2; }
        else if (b0 < 0xF0u) { cp = b0 & 0x0F; bytes = 3; }
        else                  { cp = b0 & 0x07; bytes = 4; }
        if (s.size() != bytes) throw System::FormatException("String must be exactly one character long.");
        for (size_t i = 1; i < bytes; ++i) {
            auto bi = static_cast<unsigned char>(s[i]);
            if ((bi & 0xC0u) != 0x80u) throw System::FormatException("Invalid UTF-8 sequence.");
            cp = (cp << 6) | (bi & 0x3Fu);
        }
        if (cp > 0xFFFFu) throw System::FormatException("Character is outside BMP; cannot fit in char16_t.");
        return static_cast<SharpRuntime::charcs>(cp);
    }

    /**
     * @brief Attempts to parse a single-character UTF-8 string; returns false on failure.
     *
     * C++ counterpart of .NET Char.TryParse(string, out char).
     */
    static bool TryParse(const std::string& s, SharpRuntime::charcs& result) noexcept {
        try { result = Parse(s); return true; }
        catch (...) { result = u'\0'; return false; }
    }

    /**
     * @brief Converts @p c to a UTF-8 encoded std::string (1–3 bytes for BMP code points).
     *
     * C++ counterpart of .NET Char.ToString(char).
     */
    static std::string ToString(SharpRuntime::charcs c) {
        uint32_t cp = static_cast<uint32_t>(c);
        std::string r;
        if (cp < 0x80u) {
            r += static_cast<char>(cp);
        } else if (cp < 0x800u) {
            r += static_cast<char>(0xC0u | (cp >> 6));
            r += static_cast<char>(0x80u | (cp & 0x3Fu));
        } else {
            r += static_cast<char>(0xE0u | (cp >> 12));
            r += static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu));
            r += static_cast<char>(0x80u | (cp & 0x3Fu));
        }
        return r;
    }

    // -----------------------------------------------------------------------
    // Numeric value / Unicode category
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the numeric value of @p c, or -1.0 if @p c has no numeric value.
     *
     * C++ counterpart of .NET Char.GetNumericValue(char).
     */
    static double GetNumericValue(SharpRuntime::charcs c) {
        return Globalization::CharUnicodeInfo::GetNumericValue(c);
    }

    /**
     * @brief Returns the numeric value of the character at @p index in @p s.
     *
     * C++ counterpart of .NET Char.GetNumericValue(string, int).
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    static double GetNumericValue(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return GetNumericValue(
            static_cast<SharpRuntime::charcs>(static_cast<unsigned char>(s[static_cast<size_t>(index)])));
    }

    /**
     * @brief Returns the Unicode category of @p c.
     *
     * C++ counterpart of .NET Char.GetUnicodeCategory(char).
     */
    static Globalization::UnicodeCategory GetUnicodeCategory(SharpRuntime::charcs c) {
        return Globalization::CharUnicodeInfo::GetUnicodeCategory(c);
    }

    /**
     * @brief Returns the Unicode category of the character at @p index in @p s.
     *
     * C++ counterpart of .NET Char.GetUnicodeCategory(string, int).
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     */
    static Globalization::UnicodeCategory GetUnicodeCategory(const std::string& s,
                                                              SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return GetUnicodeCategory(
            static_cast<SharpRuntime::charcs>(static_cast<unsigned char>(s[static_cast<size_t>(index)])));
    }

    // -----------------------------------------------------------------------
    // Surrogate / UTF-32 conversion
    // -----------------------------------------------------------------------

    /**
     * @brief Converts a Unicode code point to its UTF-8 string representation.
     *
     * C++ counterpart of .NET Char.ConvertFromUtf32(int).
     * BMP code points return a 1–3 byte UTF-8 string; supplementary code points return 4 bytes.
     * @throws System::ArgumentOutOfRangeException for out-of-range or surrogate code points.
     */
    static std::string ConvertFromUtf32(SharpRuntime::intcs utf32) {
        uint32_t cp = static_cast<uint32_t>(utf32);
        if (utf32 < 0 || utf32 > 0x10FFFF || (cp >= 0xD800u && cp <= 0xDFFFu))
            throw ArgumentOutOfRangeException("utf32",
                "A valid UTF32 value is between 0x000000 and 0x10ffff, inclusive, "
                "and should not include surrogate codepoint values.");
        if (cp <= 0xFFFFu) return ToString(static_cast<SharpRuntime::charcs>(cp));
        std::string r;
        r += static_cast<char>(0xF0u | (cp >> 18));
        r += static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu));
        r += static_cast<char>(0x80u | ((cp >>  6) & 0x3Fu));
        r += static_cast<char>(0x80u | ( cp         & 0x3Fu));
        return r;
    }

    /**
     * @brief Converts a surrogate pair to its UTF-32 code point.
     *
     * C++ counterpart of .NET Char.ConvertToUtf32(char, char).
     * @throws System::ArgumentOutOfRangeException if either argument is not a valid surrogate.
     */
    static SharpRuntime::intcs ConvertToUtf32(SharpRuntime::charcs high, SharpRuntime::charcs low) {
        if (!IsHighSurrogate(high))
            throw ArgumentOutOfRangeException("highSurrogate",
                "A valid high surrogate character is between 0xd800 and 0xdbff, inclusive.");
        if (!IsLowSurrogate(low))
            throw ArgumentOutOfRangeException("lowSurrogate",
                "A valid low surrogate character is between 0xdc00 and 0xdfff, inclusive.");
        return ((static_cast<SharpRuntime::intcs>(high) - 0xD800) << 10)
             +  (static_cast<SharpRuntime::intcs>(low)  - 0xDC00)
             + 0x10000;
    }

    /**
     * @brief Converts the character or surrogate pair at @p index in @p s to a UTF-32 code point.
     *
     * C++ counterpart of .NET Char.ConvertToUtf32(string, int).
     * @throws System::ArgumentOutOfRangeException if @p index is out of range.
     * @throws System::ArgumentException if a lone surrogate is found.
     */
    static SharpRuntime::intcs ConvertToUtf32(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        auto c = static_cast<SharpRuntime::charcs>(
            static_cast<unsigned char>(s[static_cast<size_t>(index)]));
        if (IsHighSurrogate(c)) {
            if (static_cast<size_t>(index + 1) < s.size()) {
                auto low = static_cast<SharpRuntime::charcs>(
                    static_cast<unsigned char>(s[static_cast<size_t>(index + 1)]));
                if (IsLowSurrogate(low))
                    return ((static_cast<SharpRuntime::intcs>(c)   - 0xD800) << 10)
                         +  (static_cast<SharpRuntime::intcs>(low) - 0xDC00)
                         + 0x10000;
            }
            throw ArgumentException(
                "A high surrogate character must be followed by a low surrogate character.", "s");
        }
        if (IsLowSurrogate(c))
            throw ArgumentException("A low surrogate character is not allowed.", "s");
        return static_cast<SharpRuntime::intcs>(c);
    }

    // -----------------------------------------------------------------------
    // String-indexed overloads
    //
    // Status: PARTIAL. Every method below (and ConvertToUtf32(string, int)'s
    // surrogate-pair branch above) reads @p s through charAt(), which returns the raw
    // *byte* at @p index -- consistent with this project's existing single-byte-per-char
    // std::string convention used throughout System::String (index is a byte offset, not
    // a UTF-16 code-unit offset). This differs from Parse()/ToString()/ConvertFromUtf32()
    // above, which do encode/decode real multi-byte UTF-8. A practical consequence: since
    // a single byte can never fall in the surrogate range (0xD800-0xDFFF is only reachable
    // by a 3-byte UTF-8 sequence), IsHighSurrogate(s, index)/IsLowSurrogate(s, index)/
    // IsSurrogate(s, index) can never return true, and ConvertToUtf32(s, index)'s
    // surrogate-pair-combining branch is unreachable through these overloads. Any @p s
    // containing non-ASCII (multi-byte UTF-8) text will have interior bytes of a multi-byte
    // sequence misclassified, since a lone continuation/lead byte does not correspond to any
    // single meaningful Unicode character. Correct only for ASCII input.
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if the character at @p index in @p s is a control character.
     *
     * C++ counterpart of .NET Char.IsControl(string, int).
     */
    static bool IsControl(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsControl(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a decimal digit.
     *
     * C++ counterpart of .NET Char.IsDigit(string, int).
     */
    static bool IsDigit(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsDigit(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a Unicode letter.
     *
     * C++ counterpart of .NET Char.IsLetter(string, int).
     */
    static bool IsLetter(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsLetter(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a letter or digit.
     *
     * C++ counterpart of .NET Char.IsLetterOrDigit(string, int).
     */
    static bool IsLetterOrDigit(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsLetterOrDigit(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a lowercase letter.
     *
     * C++ counterpart of .NET Char.IsLower(string, int).
     */
    static bool IsLower(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsLower(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is an uppercase letter.
     *
     * C++ counterpart of .NET Char.IsUpper(string, int).
     */
    static bool IsUpper(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsUpper(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is white space.
     *
     * C++ counterpart of .NET Char.IsWhiteSpace(string, int).
     */
    static bool IsWhiteSpace(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsWhiteSpace(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a number.
     *
     * C++ counterpart of .NET Char.IsNumber(string, int).
     */
    static bool IsNumber(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsNumber(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a punctuation mark.
     *
     * C++ counterpart of .NET Char.IsPunctuation(string, int).
     */
    static bool IsPunctuation(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsPunctuation(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a Unicode separator.
     *
     * C++ counterpart of .NET Char.IsSeparator(string, int).
     */
    static bool IsSeparator(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsSeparator(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is any surrogate code unit.
     *
     * C++ counterpart of .NET Char.IsSurrogate(string, int).
     */
    static bool IsSurrogate(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsSurrogate(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a symbol.
     *
     * C++ counterpart of .NET Char.IsSymbol(string, int).
     */
    static bool IsSymbol(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsSymbol(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a high surrogate.
     *
     * C++ counterpart of .NET Char.IsHighSurrogate(string, int).
     */
    static bool IsHighSurrogate(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsHighSurrogate(charAt(s, index));
    }

    /**
     * @brief Returns true if the character at @p index in @p s is a low surrogate.
     *
     * C++ counterpart of .NET Char.IsLowSurrogate(string, int).
     */
    static bool IsLowSurrogate(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        return IsLowSurrogate(charAt(s, index));
    }

    /**
     * @brief Returns true if the characters at @p index and @p index+1 in @p s form a surrogate pair.
     *
     * C++ counterpart of .NET Char.IsSurrogatePair(string, int).
     */
    static bool IsSurrogatePair(const std::string& s, SharpRuntime::intcs index) {
        if (static_cast<size_t>(index) >= s.size())
            throw ArgumentOutOfRangeException("index");
        if (static_cast<size_t>(index + 1) >= s.size()) return false;
        return IsSurrogatePair(charAt(s, index), charAt(s, index + 1));
    }

private:
    // Returns the raw byte at s[i] widened to charcs -- NOT a UTF-8 decode. See the
    // "String-indexed overloads" section note above for what this means for non-ASCII input.
    static SharpRuntime::charcs charAt(const std::string& s, SharpRuntime::intcs i) noexcept {
        return static_cast<SharpRuntime::charcs>(static_cast<unsigned char>(s[static_cast<size_t>(i)]));
    }
};

} // namespace System
