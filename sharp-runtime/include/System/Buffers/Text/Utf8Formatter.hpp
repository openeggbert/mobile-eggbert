// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/FormatException.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Buffers/StandardFormat.hpp"
#include "System/Span.hpp"

namespace System::Buffers::Text {

using SharpRuntime::intcs;

/**
 * @brief Methods to format common data types as UTF-8 strings.
 *
 * C++ counterpart of .NET System.Buffers.Text.Utf8Formatter.
 * All methods are static TryFormat overloads that write into a Span&lt;byte&gt;
 * and return true on success or false if the destination is too small.
 *
 * Covers bool and all integer types (byte/sbyte/short/ushort/int/uint/long/ulong)
 * with the 'G' (default), 'D', 'N', and 'X' format specifiers. .NET's Guid,
 * DateTime, DateTimeOffset, TimeSpan, and floating-point/decimal TryFormat
 * overloads are not yet implemented in this port — a documented gap, not a
 * silent omission (see NEXT.md).
 */
class Utf8Formatter {
public:
    Utf8Formatter() = delete;

    // -----------------------------------------------------------------------
    // bool
    // -----------------------------------------------------------------------

    /**
     * @brief Formats a bool as a UTF-8 string ("True"/"False" or "true"/"false").
     *
     * C++ counterpart of .NET Utf8Formatter.TryFormat(bool, Span&lt;byte&gt;, out int, StandardFormat).
     * @param value        Value to format.
     * @param destination  Buffer to write into.
     * @param bytesWritten Receives number of bytes written.
     * @param format       Format specifier: default/'G' for True/False, 'l' for true/false.
     * @return true on success; false if destination is too small.
     * @throws FormatException if @p format's symbol is not default, 'G', or 'l'.
     */
    static bool TryFormat(bool value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        char symbol = format.getIsDefaultProperty() ? 'G' : format.getSymbolProperty();
        const char* str;
        if (symbol == 'G') str = value ? "True" : "False";
        else if (symbol == 'l') str = value ? "true" : "false";
        else throw System::FormatException("Format specifier was invalid.");

        intcs len = static_cast<intcs>(std::strlen(str));
        if (destination.getLengthProperty() < len) { bytesWritten = 0; return false; }
        std::memcpy(destination.getPointer(), str, static_cast<size_t>(len));
        bytesWritten = len;
        return true;
    }

    // -----------------------------------------------------------------------
    // Integer types
    // -----------------------------------------------------------------------

    /**
     * @brief Formats a uint8_t as UTF-8 text.
     * C++ counterpart of .NET Utf8Formatter.TryFormat(byte, Span&lt;byte&gt;, out int, StandardFormat).
     * Formats supported: G/g (default), D/d, N/n, X/x.
     */
    static bool TryFormat(uint8_t value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        return tryFormatIntegerCore(false, 0, value, 1, destination, bytesWritten, format);
    }

    /**
     * @brief Formats an int8_t as UTF-8 text.
     * C++ counterpart of .NET Utf8Formatter.TryFormat(sbyte, Span&lt;byte&gt;, out int, StandardFormat).
     * Formats supported: G/g (default), D/d, N/n, X/x.
     */
    static bool TryFormat(int8_t value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        return tryFormatIntegerCore(true, value, 0, 1, destination, bytesWritten, format);
    }

    /**
     * @brief Formats a uint16_t as UTF-8 text.
     * C++ counterpart of .NET Utf8Formatter.TryFormat(ushort, Span&lt;byte&gt;, out int, StandardFormat).
     * Formats supported: G/g (default), D/d, N/n, X/x.
     */
    static bool TryFormat(uint16_t value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        return tryFormatIntegerCore(false, 0, value, 2, destination, bytesWritten, format);
    }

    /**
     * @brief Formats an int16_t as UTF-8 text.
     * C++ counterpart of .NET Utf8Formatter.TryFormat(short, Span&lt;byte&gt;, out int, StandardFormat).
     * Formats supported: G/g (default), D/d, N/n, X/x.
     */
    static bool TryFormat(int16_t value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        return tryFormatIntegerCore(true, value, 0, 2, destination, bytesWritten, format);
    }

    /**
     * @brief Formats a uint32_t as UTF-8 text.
     * C++ counterpart of .NET Utf8Formatter.TryFormat(uint, Span&lt;byte&gt;, out int, StandardFormat).
     * Formats supported: G/g (default), D/d, N/n, X/x.
     */
    static bool TryFormat(uint32_t value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        return tryFormatIntegerCore(false, 0, value, 4, destination, bytesWritten, format);
    }

    /**
     * @brief Formats an int32_t as UTF-8 text.
     * C++ counterpart of .NET Utf8Formatter.TryFormat(int, Span&lt;byte&gt;, out int, StandardFormat).
     * Formats supported: G/g (default), D/d, N/n, X/x.
     */
    static bool TryFormat(int32_t value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        return tryFormatIntegerCore(true, value, 0, 4, destination, bytesWritten, format);
    }

    /**
     * @brief Formats a uint64_t as UTF-8 text.
     * C++ counterpart of .NET Utf8Formatter.TryFormat(ulong, Span&lt;byte&gt;, out int, StandardFormat).
     * Formats supported: G/g (default), D/d, N/n, X/x.
     */
    static bool TryFormat(uint64_t value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        return tryFormatIntegerCore(false, 0, value, 8, destination, bytesWritten, format);
    }

    /**
     * @brief Formats an int64_t as UTF-8 text.
     * C++ counterpart of .NET Utf8Formatter.TryFormat(long, Span&lt;byte&gt;, out int, StandardFormat).
     * Formats supported: G/g (default), D/d, N/n, X/x.
     */
    static bool TryFormat(int64_t value, System::Span<uint8_t> destination, intcs& bytesWritten,
                          System::Buffers::StandardFormat format = {}) {
        return tryFormatIntegerCore(true, value, 0, 8, destination, bytesWritten, format);
    }

private:
    static bool tryFormatUnsignedDecimal(uint64_t v, intcs minDigits, System::Span<uint8_t> dest, intcs& written) {
        // minDigits comes from StandardFormat's precision, which the type validates to
        // [0, MaxPrecision] = [0, 99] at construction -- a caller-controlled, publicly
        // reachable value via e.g. StandardFormat('D', 99). The buffer must hold the larger
        // of the natural max digit count (20, for UINT64_MAX) and minDigits itself, since the
        // zero-padding loop below writes up to minDigits characters regardless of the actual
        // value's width. Confirmed via a standalone ASan repro before this fix: TryFormat(5,
        // dest, written, StandardFormat('D', 99)) triggered a genuine stack-buffer-overflow
        // write past the previous 26-byte buffer.
        char buf[100];
        intcs len = 0;
        if (v == 0) { buf[len++] = '0'; }
        else { while (v > 0) { buf[len++] = static_cast<char>('0' + v % 10); v /= 10; } }
        while (len < minDigits) buf[len++] = '0';
        std::reverse(buf, buf + len);
        if (dest.getLengthProperty() < len) { written = 0; return false; }
        std::memcpy(dest.getPointer(), buf, static_cast<size_t>(len));
        written = len;
        return true;
    }

    static bool tryFormatSignedDecimal(int64_t v, intcs minDigits, System::Span<uint8_t> dest, intcs& written) {
        bool neg = v < 0;
        uint64_t u = neg ? static_cast<uint64_t>(-(v + 1)) + 1 : static_cast<uint64_t>(v);
        // Same sizing rationale as tryFormatUnsignedDecimal's buf[100] above (minDigits is
        // caller-controlled up to StandardFormat::MaxPrecision=99), plus one extra byte for
        // the sign character. Confirmed via a standalone ASan repro before this fix:
        // TryFormat(int64_t, dest, written, StandardFormat('D', 99)) overflowed the previous
        // 27-byte buffer.
        char buf[101];
        intcs len = 0;
        if (u == 0) { buf[len++] = '0'; }
        else { while (u > 0) { buf[len++] = static_cast<char>('0' + u % 10); u /= 10; } }
        while (len < minDigits) buf[len++] = '0';
        if (neg) buf[len++] = '-';
        std::reverse(buf, buf + len);
        if (dest.getLengthProperty() < len) { written = 0; return false; }
        std::memcpy(dest.getPointer(), buf, static_cast<size_t>(len));
        written = len;
        return true;
    }

    static bool tryFormatUnsignedHex(uint64_t v, intcs byteWidth, bool uppercase, intcs minDigits,
                                     System::Span<uint8_t> dest, intcs& written) {
        uint64_t mask = (byteWidth >= 8) ? ~0ULL : ((1ULL << (byteWidth * 8)) - 1);
        v &= mask;
        const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
        // Same sizing rationale as tryFormatUnsignedDecimal's buf[100] (minDigits is
        // caller-controlled up to StandardFormat::MaxPrecision=99) -- 16 hex digits covers
        // UINT64_MAX's natural width but not a caller-requested minDigits up to 99. Confirmed
        // via a standalone ASan repro before this fix: TryFormat(uint64_t, dest, written,
        // StandardFormat('X', 99)) overflowed the previous 16-byte buffer.
        char buf[100];
        intcs len = 0;
        if (v == 0) { buf[len++] = '0'; }
        else { while (v > 0) { buf[len++] = digits[v & 0xF]; v >>= 4; } }
        while (len < minDigits) buf[len++] = '0';
        std::reverse(buf, buf + len);
        if (dest.getLengthProperty() < len) { written = 0; return false; }
        std::memcpy(dest.getPointer(), buf, static_cast<size_t>(len));
        written = len;
        return true;
    }

    static bool tryFormatGrouped(bool neg, uint64_t magnitude, intcs decimalDigits,
                                 System::Span<uint8_t> dest, intcs& written) {
        char digitsBuf[24];
        intcs dlen = 0;
        uint64_t v = magnitude;
        if (v == 0) { digitsBuf[dlen++] = '0'; }
        else { while (v > 0) { digitsBuf[dlen++] = static_cast<char>('0' + v % 10); v /= 10; } }
        std::reverse(digitsBuf, digitsBuf + dlen);

        // decimalDigits is caller-controlled up to StandardFormat::MaxPrecision=99 (the 'N'
        // format's precision selects the number of post-decimal-point zeros written below).
        // Max needed: 1 sign + 20 magnitude digits (UINT64_MAX) + up to 6 grouping commas +
        // 1 decimal point + up to 99 zeros = 127; sized generously to 150. Confirmed via a
        // standalone ASan repro before this fix: TryFormat(uint64_t, dest, written,
        // StandardFormat('N', 99)) overflowed the previous 48-byte buffer.
        char out[150];
        intcs olen = 0;
        if (neg) out[olen++] = '-';
        for (intcs i = 0; i < dlen; ++i) {
            if (i > 0 && (dlen - i) % 3 == 0) out[olen++] = ',';
            out[olen++] = digitsBuf[i];
        }
        if (decimalDigits > 0) {
            out[olen++] = '.';
            for (intcs i = 0; i < decimalDigits; ++i) out[olen++] = '0';
        }
        if (dest.getLengthProperty() < olen) { written = 0; return false; }
        std::memcpy(dest.getPointer(), out, static_cast<size_t>(olen));
        written = olen;
        return true;
    }

    // isSigned selects which of signedValue/unsignedValue holds the value; byteWidth is the
    // type's width in bytes (1/2/4/8), used to mask hex output to the correct bit pattern.
    static bool tryFormatIntegerCore(bool isSigned, int64_t signedValue, uint64_t unsignedValue, intcs byteWidth,
                                      System::Span<uint8_t> dest, intcs& written,
                                      System::Buffers::StandardFormat format) {
        if (format.getIsDefaultProperty()) {
            return isSigned ? tryFormatSignedDecimal(signedValue, 0, dest, written)
                             : tryFormatUnsignedDecimal(unsignedValue, 0, dest, written);
        }

        char symbol = format.getSymbolProperty();
        bool hasPrecision = format.getHasPrecisionProperty();
        intcs precision = hasPrecision ? static_cast<intcs>(format.getPrecisionProperty()) : 0;

        switch (symbol | 0x20) {
            case 'd':
                return isSigned ? tryFormatSignedDecimal(signedValue, precision, dest, written)
                                 : tryFormatUnsignedDecimal(unsignedValue, precision, dest, written);

            case 'x': {
                bool uppercase = (symbol == 'X');
                uint64_t bits = isSigned ? static_cast<uint64_t>(signedValue) : unsignedValue;
                return tryFormatUnsignedHex(bits, byteWidth, uppercase, precision, dest, written);
            }

            case 'n': {
                bool neg = isSigned && signedValue < 0;
                uint64_t mag = isSigned
                    ? (neg ? static_cast<uint64_t>(-(signedValue + 1)) + 1 : static_cast<uint64_t>(signedValue))
                    : unsignedValue;
                intcs decimalDigits = hasPrecision ? precision : 2;
                return tryFormatGrouped(neg, mag, decimalDigits, dest, written);
            }

            case 'g':
            case 'r':
                if (hasPrecision) {
                    throw System::NotSupportedException(
                        "Precision is not supported for the 'G' format specifier for integer types "
                        "since it could result in a value that cannot be re-parsed unambiguously.");
                }
                return isSigned ? tryFormatSignedDecimal(signedValue, 0, dest, written)
                                 : tryFormatUnsignedDecimal(unsignedValue, 0, dest, written);

            default:
                throw System::FormatException("Format specifier was invalid.");
        }
    }
};

} // namespace System::Buffers::Text
