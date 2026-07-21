// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include <stdexcept>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Globalization {

/**
 * @brief Supports the use of non-ASCII characters for Internet domain names.
 *
 * C++ counterpart of .NET System.Globalization.IdnMapping.
 * Implements Punycode (RFC 3492) / IDNA (RFC 3490) encoding and decoding.
 * GetAscii() converts a Unicode domain name to its ACE (xn--) ASCII representation;
 * GetUnicode() reverses the conversion. ASCII-only labels are passed through unchanged.
 */
class IdnMapping {
public:
    /**
     * @brief Constructs an IdnMapping with default settings.
     *
     * C++ counterpart of .NET IdnMapping().
     * AllowUnassigned and UseStd3AsciiRules default to false.
     */
    IdnMapping() = default;

    /**
     * @brief Gets a value indicating whether unassigned Unicode code points are allowed.
     *
     * C++ counterpart of .NET IdnMapping.AllowUnassigned.
     * @return true if unassigned code points are permitted; otherwise false.
     */
    [[nodiscard]] bool getAllowUnassignedProperty() const { return allowUnassigned_; }

    /**
     * @brief Sets whether unassigned Unicode code points are allowed.
     *
     * C++ counterpart of .NET IdnMapping.AllowUnassigned setter.
     * @param v true to allow unassigned code points; false to reject them.
     */
    void setAllowUnassignedProperty(bool v) { allowUnassigned_ = v; }

    /**
     * @brief Gets a value indicating whether Std3 ASCII naming rules are applied.
     *
     * C++ counterpart of .NET IdnMapping.UseStd3AsciiRules.
     * @return true if RFC 1123 Std3 naming rules are enforced; otherwise false.
     */
    [[nodiscard]] bool getUseStd3AsciiRulesProperty() const { return useStd3_; }

    /**
     * @brief Sets whether Std3 ASCII naming rules are applied.
     *
     * C++ counterpart of .NET IdnMapping.UseStd3AsciiRules setter.
     * @param v true to enforce RFC 1123 Std3 rules; false to disable.
     */
    void setUseStd3AsciiRulesProperty(bool v) { useStd3_ = v; }

    /**
     * @brief Converts a Unicode (UTF-8) domain name to its Punycode ASCII form.
     *
     * C++ counterpart of .NET IdnMapping.GetAscii(string).
     * Labels already in ASCII are passed through unchanged.
     * @param unicode The Unicode domain name to encode.
     * @return The ACE (xn--) ASCII representation.
     */
    [[nodiscard]] std::string GetAscii(const std::string& unicode) const;

    /**
     * @brief Converts a Unicode (UTF-8) domain name, starting at a byte offset, to Punycode.
     *
     * C++ counterpart of .NET IdnMapping.GetAscii(string, int). .NET's @p index is a UTF-16
     * code-unit offset; this port has no UTF-16 representation, so @p index/@p count are byte
     * offsets into the UTF-8-encoded @p unicode string instead (matching the convention
     * already established by System::String::Substring(string, intcs)).
     * @param unicode The Unicode domain name to encode.
     * @param index   The zero-based byte offset at which to start.
     * @return The ACE (xn--) ASCII representation of the substring.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than @p unicode's length.
     */
    [[nodiscard]] std::string GetAscii(const std::string& unicode, SharpRuntime::intcs index) const;

    /**
     * @brief Converts a byte-offset substring of a Unicode (UTF-8) domain name to Punycode.
     *
     * C++ counterpart of .NET IdnMapping.GetAscii(string, int, int). See the (string, int)
     * overload for the byte-offset-vs-UTF-16-code-unit deviation.
     * @param unicode The Unicode domain name to encode.
     * @param index   The zero-based byte offset at which to start.
     * @param count   The number of bytes to encode.
     * @return The ACE (xn--) ASCII representation of the substring.
     * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative, or the
     *         range they describe falls outside @p unicode.
     */
    [[nodiscard]] std::string GetAscii(const std::string& unicode, SharpRuntime::intcs index, SharpRuntime::intcs count) const;

    /**
     * @brief Converts a Punycode ASCII domain name back to Unicode (UTF-8).
     *
     * C++ counterpart of .NET IdnMapping.GetUnicode(string).
     * @param ascii The ACE-encoded ASCII domain name to decode.
     * @return The decoded Unicode domain name.
     */
    [[nodiscard]] std::string GetUnicode(const std::string& ascii) const;

    /**
     * @brief Converts a Punycode ASCII domain name, starting at a byte offset, back to Unicode.
     *
     * C++ counterpart of .NET IdnMapping.GetUnicode(string, int). See GetAscii(string, int)'s
     * doc comment for the byte-offset-vs-UTF-16-code-unit deviation.
     * @param ascii The ACE-encoded ASCII domain name to decode.
     * @param index The zero-based byte offset at which to start.
     * @return The decoded Unicode domain name.
     * @throws System::ArgumentOutOfRangeException if @p index is negative or greater than @p ascii's length.
     */
    [[nodiscard]] std::string GetUnicode(const std::string& ascii, SharpRuntime::intcs index) const;

    /**
     * @brief Converts a byte-offset substring of a Punycode ASCII domain name back to Unicode.
     *
     * C++ counterpart of .NET IdnMapping.GetUnicode(string, int, int). See
     * GetAscii(string, int)'s doc comment for the byte-offset-vs-UTF-16-code-unit deviation.
     * @param ascii The ACE-encoded ASCII domain name to decode.
     * @param index The zero-based byte offset at which to start.
     * @param count The number of bytes to decode.
     * @return The decoded Unicode domain name.
     * @throws System::ArgumentOutOfRangeException if @p index or @p count is negative, or the
     *         range they describe falls outside @p ascii.
     */
    [[nodiscard]] std::string GetUnicode(const std::string& ascii, SharpRuntime::intcs index, SharpRuntime::intcs count) const;

    /**
     * @brief Returns true if both IdnMapping instances have the same settings.
     *
     * C++ counterpart of .NET IdnMapping.Equals(object).
     * @param o The other IdnMapping instance.
     * @return true if AllowUnassigned and UseStd3AsciiRules are equal.
     */
    bool operator==(const IdnMapping& o) const {
        return allowUnassigned_ == o.allowUnassigned_ && useStd3_ == o.useStd3_;
    }

    /**
     * @brief Returns a hash code for this instance.
     *
     * C++ counterpart of .NET IdnMapping.GetHashCode().
     * @return A hash code derived from AllowUnassigned and UseStd3AsciiRules.
     */
    [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
        return (allowUnassigned_ ? 1 : 0) | (useStd3_ ? 2 : 0);
    }

private:
    bool allowUnassigned_ = false;
    bool useStd3_         = false;

    // ---- Punycode constants (RFC 3492) ----
    static constexpr int Base        = 36;
    static constexpr int Tmin        = 1;
    static constexpr int Tmax        = 26;
    static constexpr int Skew        = 38;
    static constexpr int Damp        = 700;
    static constexpr int InitialBias = 72;
    static constexpr int InitialN    = 0x80;
    static constexpr int LabelMax    = 63;
    static constexpr int NameMax     = 255;

    // ---- UTF-8 helpers ----
    static std::u32string utf8ToCodePoints(const std::string& s);
    static std::string codePointsToUtf8(const std::u32string& cp);
    static bool isBasic(char32_t c) { return c < 0x80; }

    // ---- Punycode core ----
    static int  adapt(int delta, int numpoints, bool firsttime);
    static char encodeDigit(int d);
    static int  decodeDigit(char c);

    static std::string    encodeLabel(const std::u32string& label);
    static std::u32string decodeLabel(const std::string& label);

    // ---- Validation helpers ----
    /** @brief Throws ArgumentException if @p c violates the Std3 ASCII naming rules. */
    static void validateStd3Char(char32_t c, bool nextToLabelBoundary);
};

} // namespace System::Globalization
