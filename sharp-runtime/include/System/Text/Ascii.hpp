// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cctype>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Buffers/OperationStatus.hpp"

namespace System::Text {

using SharpRuntime::bytecs;
using SharpRuntime::charcs;
using SharpRuntime::intcs;

/** Provides static methods for working with ASCII characters and text. */
class Ascii {
public:
    Ascii() = delete;

    // --- IsValid ---

    /** @return True if the byte value is a valid ASCII character (<= 127). */
    static bool IsValid(bytecs value) { return value <= 127; }

    /** @return True if the char value is a valid ASCII character (<= 127). */
    static bool IsValid(charcs value) { return value <= 127; }

    /** @return True if all bytes in @p value are valid ASCII (<= 127). */
    static bool IsValid(const std::vector<bytecs>& value) {
        for (bytecs b : value) if (b > 127) return false;
        return true;
    }

    /** @return True if all code units in the UTF-16 string are valid ASCII (<= 127). */
    static bool IsValid(const std::u16string& value) {
        for (char16_t c : value) if (c > 127) return false;
        return true;
    }

    /** @return True if all bytes in the std::string are valid ASCII (<= 127). */
    static bool IsValid(const std::string& value) {
        for (unsigned char c : value) if (c > 127) return false;
        return true;
    }

    // --- Equals ---

    /** @return True if @p left and @p right contain identical byte sequences. */
    static bool Equals(const std::vector<bytecs>& left, const std::vector<bytecs>& right) {
        if (left.size() != right.size()) return false;
        for (size_t i = 0; i < left.size(); ++i)
            if (left[i] != right[i]) return false;
        return true;
    }

    /** @return True if @p left and @p right are equal using ASCII case-insensitive comparison. */
    static bool EqualsIgnoreCase(const std::vector<bytecs>& left, const std::vector<bytecs>& right) {
        if (left.size() != right.size()) return false;
        for (size_t i = 0; i < left.size(); ++i)
            if (std::tolower(left[i]) != std::tolower(right[i])) return false;
        return true;
    }

    /** @return True if @p left and @p right are equal using ASCII case-insensitive comparison. */
    static bool EqualsIgnoreCase(const std::string& left, const std::string& right) {
        if (left.size() != right.size()) return false;
        for (size_t i = 0; i < left.size(); ++i)
            if (std::tolower(static_cast<unsigned char>(left[i])) !=
                std::tolower(static_cast<unsigned char>(right[i]))) return false;
        return true;
    }

    // --- ToUpper / ToLower in-place ---

    /**
     * Converts all bytes in @p value to upper-case ASCII in place.
     * @param bytesWritten Set to the number of bytes processed.
     * @return OperationStatus::Done.
     */
    static System::Buffers::OperationStatus ToUpperInPlace(std::vector<bytecs>& value, intcs& bytesWritten) {
        for (bytecs& b : value)
            b = static_cast<bytecs>(std::toupper(b));
        bytesWritten = static_cast<intcs>(value.size());
        return System::Buffers::OperationStatus::Done;
    }

    /**
     * Converts all bytes in @p value to lower-case ASCII in place.
     * @param bytesWritten Set to the number of bytes processed.
     * @return OperationStatus::Done.
     */
    static System::Buffers::OperationStatus ToLowerInPlace(std::vector<bytecs>& value, intcs& bytesWritten) {
        for (bytecs& b : value)
            b = static_cast<bytecs>(std::tolower(b));
        bytesWritten = static_cast<intcs>(value.size());
        return System::Buffers::OperationStatus::Done;
    }

    /**
     * Converts all characters in @p value to upper-case ASCII in place.
     * @param charsWritten Set to the number of characters processed.
     * @return OperationStatus::Done.
     */
    static System::Buffers::OperationStatus ToUpperInPlace(std::string& value, intcs& charsWritten) {
        for (char& c : value)
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        charsWritten = static_cast<intcs>(value.size());
        return System::Buffers::OperationStatus::Done;
    }

    /**
     * Converts all characters in @p value to lower-case ASCII in place.
     * @param charsWritten Set to the number of characters processed.
     * @return OperationStatus::Done.
     */
    static System::Buffers::OperationStatus ToLowerInPlace(std::string& value, intcs& charsWritten) {
        for (char& c : value)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        charsWritten = static_cast<intcs>(value.size());
        return System::Buffers::OperationStatus::Done;
    }

    // --- ToUpper / ToLower with destination ---

    /**
     * Copies @p source to @p destination converting each byte to upper-case.
     * @param bytesWritten Number of bytes written.
     * @return Done, or DestinationTooSmall if destination is shorter than source.
     */
    static System::Buffers::OperationStatus ToUpper(const std::vector<bytecs>& source,
                                                    std::vector<bytecs>& destination,
                                                    intcs& bytesWritten) {
        size_t n = std::min(source.size(), destination.size());
        for (size_t i = 0; i < n; ++i)
            destination[i] = static_cast<bytecs>(std::toupper(source[i]));
        bytesWritten = static_cast<intcs>(n);
        return (n == source.size()) ? System::Buffers::OperationStatus::Done
                                    : System::Buffers::OperationStatus::DestinationTooSmall;
    }

    /**
     * Copies @p source to @p destination converting each byte to lower-case.
     * @param bytesWritten Number of bytes written.
     * @return Done, or DestinationTooSmall if destination is shorter than source.
     */
    static System::Buffers::OperationStatus ToLower(const std::vector<bytecs>& source,
                                                    std::vector<bytecs>& destination,
                                                    intcs& bytesWritten) {
        size_t n = std::min(source.size(), destination.size());
        for (size_t i = 0; i < n; ++i)
            destination[i] = static_cast<bytecs>(std::tolower(source[i]));
        bytesWritten = static_cast<intcs>(n);
        return (n == source.size()) ? System::Buffers::OperationStatus::Done
                                    : System::Buffers::OperationStatus::DestinationTooSmall;
    }

    // --- Trim helpers (return trimmed string) ---

    /**
     * @brief Returns true if @p c is one of the exact six bytes .NET's Ascii.Trim* family
     * treats as ASCII whitespace: TAB, LF, VT, FF, CR, or space (Ascii.Trimming.cs's
     * TrimMask) -- NOT every byte <= 32 (e.g. NUL and other C0 controls are excluded).
     */
    static bool isAsciiTrimWhitespace(unsigned char c) {
        return c == 0x09 || c == 0x0A || c == 0x0B || c == 0x0C || c == 0x0D || c == 0x20;
    }

    /** @return @p value with leading and trailing ASCII whitespace removed. */
    static std::string Trim(const std::string& value) {
        size_t s = 0, e = value.size();
        while (s < e && isAsciiTrimWhitespace(static_cast<unsigned char>(value[s]))) ++s;
        while (e > s && isAsciiTrimWhitespace(static_cast<unsigned char>(value[e - 1]))) --e;
        return value.substr(s, e - s);
    }

    /** @return @p value with leading ASCII whitespace removed. */
    static std::string TrimStart(const std::string& value) {
        size_t s = 0;
        while (s < value.size() && isAsciiTrimWhitespace(static_cast<unsigned char>(value[s]))) ++s;
        return value.substr(s);
    }

    /** @return @p value with trailing ASCII whitespace removed. */
    static std::string TrimEnd(const std::string& value) {
        size_t e = value.size();
        while (e > 0 && isAsciiTrimWhitespace(static_cast<unsigned char>(value[e - 1]))) --e;
        return value.substr(0, e);
    }
};

} // namespace System::Text
