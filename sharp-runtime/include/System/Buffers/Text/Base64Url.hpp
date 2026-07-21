// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/Buffers/OperationStatus.hpp"
#include "System/Span.hpp"
#include "System/ReadOnlySpan.hpp"

namespace System::Buffers::Text {

using SharpRuntime::intcs;

/**
 * @brief Converts between binary data and URL-safe base64url encoded text.
 *
 * C++ counterpart of .NET System.Buffers.Text.Base64Url (introduced .NET 9).
 * Uses the RFC 4648 §5 alphabet ('+' -> '-', '/' -> '_') without padding.
 * Any amount of ASCII whitespace (' ', '\\t', '\\r', '\\n') is allowed anywhere
 * in decode/validate input, matching .NET's Base64Url behavior.
 */
class Base64Url {
    static constexpr intcs kMaximumEncodeLength = 1610612733;

    static constexpr char kEncTable[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    static constexpr int8_t kDecTable[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,63,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    };

    static bool isWhitespace(uint8_t c) noexcept {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    // Shared decode core (no padding: trailing remainder of 2 or 3 symbols decodes
    // partially; remainder of 1 symbol is never valid).
    template<typename CharT>
    static OperationStatus decodeCore(const CharT* src, intcs srcLen, uint8_t* dst, intcs dstLen,
                                       intcs& consumed, intcs& written, bool isFinalBlock) {
        consumed = 0; written = 0;
        intcs si = 0, di = 0;
        int8_t vals[4] = {0, 0, 0, 0};
        int valCount = 0;

        while (si < srcLen) {
            uint8_t ch = static_cast<uint8_t>(src[si]);
            if (isWhitespace(ch)) { ++si; continue; }
            int8_t v = kDecTable[ch];
            if (v < 0) return OperationStatus::InvalidData;
            vals[valCount++] = v;
            ++si;

            if (valCount == 4) {
                if (dstLen - di < 3) return OperationStatus::DestinationTooSmall;
                uint32_t val = (uint32_t(vals[0])<<18)|(uint32_t(vals[1])<<12)|(uint32_t(vals[2])<<6)|uint32_t(vals[3]);
                dst[di++] = static_cast<uint8_t>(val >> 16);
                dst[di++] = static_cast<uint8_t>(val >> 8);
                dst[di++] = static_cast<uint8_t>(val);
                consumed = si; written = di;
                valCount = 0;
            }
        }

        if (valCount > 0) {
            if (!isFinalBlock) return OperationStatus::NeedMoreData;
            if (valCount == 1) return OperationStatus::InvalidData;
            if (valCount == 2) {
                if (dstLen - di < 1) return OperationStatus::DestinationTooSmall;
                uint32_t val = (uint32_t(vals[0])<<18)|(uint32_t(vals[1])<<12);
                dst[di++] = static_cast<uint8_t>(val >> 16);
            } else { // valCount == 3
                if (dstLen - di < 2) return OperationStatus::DestinationTooSmall;
                uint32_t val = (uint32_t(vals[0])<<18)|(uint32_t(vals[1])<<12)|(uint32_t(vals[2])<<6);
                dst[di++] = static_cast<uint8_t>(val >> 16);
                dst[di++] = static_cast<uint8_t>(val >> 8);
            }
        }
        consumed = si; written = di;
        return OperationStatus::Done;
    }

    template<typename CharT>
    static bool validateCore(const CharT* src, intcs srcLen, intcs& decodedLength) {
        decodedLength = 0;
        intcs si = 0, di = 0;
        int valCount = 0;

        while (si < srcLen) {
            uint8_t ch = static_cast<uint8_t>(src[si]);
            if (isWhitespace(ch)) { ++si; continue; }
            if (kDecTable[ch] < 0) return false;
            ++valCount; ++si;
            if (valCount == 4) { di += 3; valCount = 0; }
        }
        if (valCount == 1) return false;
        di += (valCount == 2) ? 1 : (valCount == 3 ? 2 : 0);
        decodedLength = di;
        return true;
    }

    // Shared encode core (no padding: a trailing remainder of 1 byte encodes to 2 chars,
    // a trailing remainder of 2 bytes encodes to 3 chars).
    template<typename CharT>
    static OperationStatus encodeCore(const uint8_t* src, intcs srcLen, CharT* dst, intcs dstLen,
                                       intcs& bytesConsumed, intcs& written, bool isFinalBlock) {
        bytesConsumed = 0; written = 0;
        intcs fullGroups = srcLen / 3;
        intcs remainder  = srcLen % 3;

        for (intcs i = 0; i < fullGroups; ++i) {
            if (dstLen - written < 4) return OperationStatus::DestinationTooSmall;
            uint32_t b = (uint32_t(src[0]) << 16) | (uint32_t(src[1]) << 8) | src[2];
            dst[0] = static_cast<CharT>(kEncTable[(b >> 18) & 0x3F]);
            dst[1] = static_cast<CharT>(kEncTable[(b >> 12) & 0x3F]);
            dst[2] = static_cast<CharT>(kEncTable[(b >>  6) & 0x3F]);
            dst[3] = static_cast<CharT>(kEncTable[(b      ) & 0x3F]);
            src += 3; dst += 4;
            bytesConsumed += 3;
            written  += 4;
        }

        if (!isFinalBlock && remainder > 0) return OperationStatus::NeedMoreData;

        if (isFinalBlock && remainder == 1) {
            if (dstLen - written < 2) return OperationStatus::DestinationTooSmall;
            uint32_t b = uint32_t(src[0]) << 16;
            dst[0] = static_cast<CharT>(kEncTable[(b >> 18) & 0x3F]);
            dst[1] = static_cast<CharT>(kEncTable[(b >> 12) & 0x3F]);
            bytesConsumed += 1; written += 2;
        } else if (isFinalBlock && remainder == 2) {
            if (dstLen - written < 3) return OperationStatus::DestinationTooSmall;
            uint32_t b = (uint32_t(src[0]) << 16) | (uint32_t(src[1]) << 8);
            dst[0] = static_cast<CharT>(kEncTable[(b >> 18) & 0x3F]);
            dst[1] = static_cast<CharT>(kEncTable[(b >> 12) & 0x3F]);
            dst[2] = static_cast<CharT>(kEncTable[(b >>  6) & 0x3F]);
            bytesConsumed += 2; written += 3;
        }
        return OperationStatus::Done;
    }

    static std::string invalidDataMessage() {
        return "The input is not a valid Base-64 string as it contains a non-base 64 character, more than two padding characters, or an illegal character among the padding characters.";
    }

public:
    Base64Url() = delete;

    // -----------------------------------------------------------------------
    // Length helpers
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the number of UTF-8 bytes needed to encode @p bytesLength raw bytes.
     * C++ counterpart of .NET Base64Url.GetEncodedLength(int).
     * Base64Url output is unpadded: length = whole*4 + (remainder>0 ? remainder+1 : 0).
     * @throws ArgumentOutOfRangeException if @p bytesLength is negative or exceeds 1610612733.
     */
    [[nodiscard]] static intcs GetEncodedLength(intcs bytesLength) {
        if (bytesLength < 0 || bytesLength > kMaximumEncodeLength)
            throw ArgumentOutOfRangeException("bytesLength", "Non-negative number required, and less than or equal to 1610612733.");
        intcs whole = bytesLength / 3;
        intcs remainder = bytesLength % 3;
        return whole * 4 + (remainder > 0 ? remainder + 1 : 0);
    }

    /**
     * @brief Returns the maximum number of bytes that decoding @p base64UrlLength chars can produce.
     * C++ counterpart of .NET Base64Url.GetMaxDecodedLength(int).
     * @throws ArgumentOutOfRangeException if @p base64UrlLength is negative.
     */
    [[nodiscard]] static intcs GetMaxDecodedLength(intcs base64UrlLength) {
        if (base64UrlLength < 0) throw ArgumentOutOfRangeException("base64UrlLength", "Non-negative number required.");
        intcs whole = base64UrlLength / 4;
        intcs remainder = base64UrlLength % 4;
        return whole * 3 + (remainder > 0 ? remainder - 1 : 0);
    }

    // -----------------------------------------------------------------------
    // Encode — bytes
    // -----------------------------------------------------------------------

    /**
     * @brief Encodes binary bytes as UTF-8 base64url (no padding).
     * C++ counterpart of .NET Base64Url.EncodeToUtf8(ReadOnlySpan, Span, out int, out int, bool).
     */
    static OperationStatus EncodeToUtf8(
        System::ReadOnlySpan<uint8_t> bytes,
        System::Span<uint8_t>         utf8,
        intcs&                        bytesConsumed,
        intcs&                        bytesWritten,
        bool                          isFinalBlock = true)
    {
        return encodeCore<uint8_t>(bytes.getPointer(), bytes.getLengthProperty(),
                                    utf8.getPointer(), utf8.getLengthProperty(),
                                    bytesConsumed, bytesWritten, isFinalBlock);
    }

    /**
     * @brief Encodes binary bytes as UTF-8 base64url text, throwing if @p destination is too small.
     * C++ counterpart of .NET Base64Url.EncodeToUtf8(ReadOnlySpan, Span).
     */
    static intcs EncodeToUtf8(System::ReadOnlySpan<uint8_t> source, System::Span<uint8_t> destination) {
        intcs consumed = 0, written = 0;
        OperationStatus status = EncodeToUtf8(source, destination, consumed, written);
        if (status == OperationStatus::Done) return written;
        throw ArgumentException("Destination is too short.", "destination");
    }

    /**
     * @brief Encodes binary bytes as UTF-8 base64url text into a newly allocated buffer.
     * C++ counterpart of .NET Base64Url.EncodeToUtf8(ReadOnlySpan).
     */
    [[nodiscard]] static std::vector<uint8_t> EncodeToUtf8(System::ReadOnlySpan<uint8_t> source) {
        intcs len = GetEncodedLength(source.getLengthProperty());
        std::vector<uint8_t> destination(static_cast<size_t>(len));
        intcs consumed = 0, written = 0;
        System::Span<uint8_t> dst(destination.data(), len);
        EncodeToUtf8(source, dst, consumed, written);
        return destination;
    }

    /**
     * @brief Attempts to encode binary bytes as UTF-8 base64url text.
     * C++ counterpart of .NET Base64Url.TryEncodeToUtf8(ReadOnlySpan, Span, out int).
     */
    static bool TryEncodeToUtf8(System::ReadOnlySpan<uint8_t> source, System::Span<uint8_t> destination,
                                 intcs& bytesWritten) {
        intcs consumed = 0;
        return EncodeToUtf8(source, destination, consumed, bytesWritten) == OperationStatus::Done;
    }

    /**
     * @brief Attempts to encode @p dataLength bytes at the start of @p buffer, in place, as base64url text.
     *
     * C++ counterpart of .NET Base64Url.TryEncodeToUtf8InPlace(Span, int, out int).
     * Processes 3-byte source groups from the last to the first so the (larger) encoded
     * groups never overwrite not-yet-read source bytes.
     */
    static bool TryEncodeToUtf8InPlace(System::Span<uint8_t> buffer, intcs dataLength, intcs& bytesWritten) {
        bytesWritten = 0;
        intcs encodedLen = GetEncodedLength(dataLength);
        if (encodedLen > buffer.getLengthProperty()) return false;

        uint8_t* buf = buffer.getPointer();
        intcs fullGroups = dataLength / 3;
        intcs remainder  = dataLength % 3;

        for (intcs i = fullGroups - 1; i >= 0; --i) {
            intcs srcOff = i * 3, dstOff = i * 4;
            uint32_t b = (uint32_t(buf[srcOff]) << 16) | (uint32_t(buf[srcOff+1]) << 8) | buf[srcOff+2];
            buf[dstOff]   = static_cast<uint8_t>(kEncTable[(b >> 18) & 0x3F]);
            buf[dstOff+1] = static_cast<uint8_t>(kEncTable[(b >> 12) & 0x3F]);
            buf[dstOff+2] = static_cast<uint8_t>(kEncTable[(b >>  6) & 0x3F]);
            buf[dstOff+3] = static_cast<uint8_t>(kEncTable[(b      ) & 0x3F]);
        }

        intcs dstOff = fullGroups * 4;
        if (remainder == 1) {
            uint8_t b0 = buf[fullGroups * 3];
            uint32_t b = uint32_t(b0) << 16;
            buf[dstOff]   = static_cast<uint8_t>(kEncTable[(b >> 18) & 0x3F]);
            buf[dstOff+1] = static_cast<uint8_t>(kEncTable[(b >> 12) & 0x3F]);
        } else if (remainder == 2) {
            uint8_t b0 = buf[fullGroups * 3], b1 = buf[fullGroups * 3 + 1];
            uint32_t b = (uint32_t(b0) << 16) | (uint32_t(b1) << 8);
            buf[dstOff]   = static_cast<uint8_t>(kEncTable[(b >> 18) & 0x3F]);
            buf[dstOff+1] = static_cast<uint8_t>(kEncTable[(b >> 12) & 0x3F]);
            buf[dstOff+2] = static_cast<uint8_t>(kEncTable[(b >>  6) & 0x3F]);
        }
        bytesWritten = encodedLen;
        return true;
    }

    // -----------------------------------------------------------------------
    // Encode — chars
    // -----------------------------------------------------------------------

    /**
     * @brief Encodes binary bytes as base64url ASCII chars.
     * C++ counterpart of .NET Base64Url.EncodeToChars(ReadOnlySpan&lt;byte&gt;, Span&lt;char&gt;, out int, out int, bool).
     */
    static OperationStatus EncodeToChars(
        System::ReadOnlySpan<uint8_t> source,
        System::Span<char>            destination,
        intcs&                        bytesConsumed,
        intcs&                        charsWritten,
        bool                          isFinalBlock = true)
    {
        return encodeCore<char>(source.getPointer(), source.getLengthProperty(),
                                 destination.getPointer(), destination.getLengthProperty(),
                                 bytesConsumed, charsWritten, isFinalBlock);
    }

    /**
     * @brief Encodes binary bytes as base64url ASCII chars, throwing if @p destination is too small.
     * C++ counterpart of .NET Base64Url.EncodeToChars(ReadOnlySpan&lt;byte&gt;, Span&lt;char&gt;).
     */
    static intcs EncodeToChars(System::ReadOnlySpan<uint8_t> source, System::Span<char> destination) {
        intcs consumed = 0, written = 0;
        OperationStatus status = EncodeToChars(source, destination, consumed, written);
        if (status == OperationStatus::Done) return written;
        throw ArgumentException("Destination is too short.", "destination");
    }

    /**
     * @brief Attempts to encode binary bytes as base64url ASCII chars.
     * C++ counterpart of .NET Base64Url.TryEncodeToChars(ReadOnlySpan&lt;byte&gt;, Span&lt;char&gt;, out int).
     */
    static bool TryEncodeToChars(System::ReadOnlySpan<uint8_t> source, System::Span<char> destination,
                                  intcs& charsWritten) {
        intcs consumed = 0;
        return EncodeToChars(source, destination, consumed, charsWritten) == OperationStatus::Done;
    }

    /**
     * @brief Encodes binary bytes as a std::string of base64url text.
     * C++ counterpart of .NET Base64Url.EncodeToString(ReadOnlySpan&lt;byte&gt;).
     */
    [[nodiscard]] static std::string EncodeToString(System::ReadOnlySpan<uint8_t> source) {
        intcs len = GetEncodedLength(source.getLengthProperty());
        std::string result(static_cast<size_t>(len), '\0');
        intcs consumed = 0, written = 0;
        System::Span<char> dst(result.data(), len);
        EncodeToChars(source, dst, consumed, written);
        return result;
    }

    /**
     * @brief Encodes binary bytes as a std::string of base64url text.
     * Convenience overload accepting a std::vector directly (not in .NET API, but useful in C++).
     */
    [[nodiscard]] static std::string EncodeToString(const std::vector<uint8_t>& bytes) {
        return EncodeToString(System::ReadOnlySpan<uint8_t>(bytes));
    }

    // -----------------------------------------------------------------------
    // Decode — bytes
    // -----------------------------------------------------------------------

    /**
     * @brief Decodes UTF-8 base64url text to binary bytes.
     *
     * C++ counterpart of .NET Base64Url.DecodeFromUtf8(ReadOnlySpan, Span, out int, out int, bool).
     * As padding is optional, @p utf8 need not be a multiple of 4 in length even when
     * @p isFinalBlock is true: a remainder of 3 decodes to 2 bytes, a remainder of 2
     * decodes to 1 byte, and a remainder of 1 is InvalidData.
     */
    static OperationStatus DecodeFromUtf8(
        System::ReadOnlySpan<uint8_t> utf8,
        System::Span<uint8_t>         bytes,
        intcs&                        bytesConsumed,
        intcs&                        bytesWritten,
        bool                          isFinalBlock = true)
    {
        return decodeCore<uint8_t>(utf8.getPointer(), utf8.getLengthProperty(),
                                    bytes.getPointer(), bytes.getLengthProperty(),
                                    bytesConsumed, bytesWritten, isFinalBlock);
    }

    /**
     * @brief Decodes UTF-8 base64url text to binary bytes, throwing on invalid data or a too-small destination.
     * C++ counterpart of .NET Base64Url.DecodeFromUtf8(ReadOnlySpan, Span).
     */
    static intcs DecodeFromUtf8(System::ReadOnlySpan<uint8_t> source, System::Span<uint8_t> destination) {
        intcs consumed = 0, written = 0;
        OperationStatus status = DecodeFromUtf8(source, destination, consumed, written);
        if (status == OperationStatus::Done) return written;
        if (status == OperationStatus::DestinationTooSmall)
            throw ArgumentException("Destination is too short.", "destination");
        throw FormatException(invalidDataMessage());
    }

    /**
     * @brief Decodes UTF-8 base64url text into a newly allocated buffer.
     * C++ counterpart of .NET Base64Url.DecodeFromUtf8(ReadOnlySpan).
     */
    [[nodiscard]] static std::vector<uint8_t> DecodeFromUtf8(System::ReadOnlySpan<uint8_t> source) {
        intcs upperBound = GetMaxDecodedLength(source.getLengthProperty());
        std::vector<uint8_t> destination(static_cast<size_t>(upperBound));
        intcs consumed = 0, written = 0;
        System::Span<uint8_t> dst(destination.data(), upperBound);
        OperationStatus status = DecodeFromUtf8(source, dst, consumed, written);
        if (status != OperationStatus::Done) throw FormatException(invalidDataMessage());
        destination.resize(static_cast<size_t>(written));
        return destination;
    }

    /**
     * @brief Attempts to decode UTF-8 base64url text to binary bytes.
     * C++ counterpart of .NET Base64Url.TryDecodeFromUtf8(ReadOnlySpan, Span, out int).
     */
    static bool TryDecodeFromUtf8(System::ReadOnlySpan<uint8_t> source, System::Span<uint8_t> destination,
                                   intcs& bytesWritten) {
        intcs consumed = 0;
        OperationStatus status = DecodeFromUtf8(source, destination, consumed, bytesWritten);
        if (status == OperationStatus::InvalidData) throw FormatException(invalidDataMessage());
        return status == OperationStatus::Done;
    }

    /**
     * @brief Decodes base64url text in @p buffer into binary data, in place.
     *
     * C++ counterpart of .NET Base64Url.DecodeFromUtf8InPlace(Span). Unlike
     * Base64::DecodeFromUtf8InPlace this is not an OperationStatus API: it throws on
     * invalid input instead of returning a status, matching .NET's real signature.
     * @return The number of bytes written into @p buffer.
     * @throws FormatException if @p buffer contains invalid base64url data.
     */
    static intcs DecodeFromUtf8InPlace(System::Span<uint8_t> buffer) {
        intcs consumed = 0, written = 0;
        intcs len = buffer.getLengthProperty();
        System::ReadOnlySpan<uint8_t> source(buffer.getPointer(), len);
        OperationStatus status = DecodeFromUtf8(source, buffer, consumed, written, true);
        if (status == OperationStatus::InvalidData) throw FormatException(invalidDataMessage());
        return written;
    }

    // -----------------------------------------------------------------------
    // Decode — chars
    // -----------------------------------------------------------------------

    /**
     * @brief Decodes base64url ASCII chars to binary bytes.
     * C++ counterpart of .NET Base64Url.DecodeFromChars(ReadOnlySpan&lt;char&gt;, Span&lt;byte&gt;, out int, out int, bool).
     */
    static OperationStatus DecodeFromChars(
        System::ReadOnlySpan<char> source,
        System::Span<uint8_t>      destination,
        intcs&                     charsConsumed,
        intcs&                     bytesWritten,
        bool                       isFinalBlock = true)
    {
        return decodeCore<char>(source.getPointer(), source.getLengthProperty(),
                                 destination.getPointer(), destination.getLengthProperty(),
                                 charsConsumed, bytesWritten, isFinalBlock);
    }

    /**
     * @brief Decodes base64url ASCII chars to binary bytes, throwing on invalid data or a too-small destination.
     * C++ counterpart of .NET Base64Url.DecodeFromChars(ReadOnlySpan&lt;char&gt;, Span&lt;byte&gt;).
     */
    static intcs DecodeFromChars(System::ReadOnlySpan<char> source, System::Span<uint8_t> destination) {
        intcs consumed = 0, written = 0;
        OperationStatus status = DecodeFromChars(source, destination, consumed, written);
        if (status == OperationStatus::Done) return written;
        if (status == OperationStatus::DestinationTooSmall)
            throw ArgumentException("Destination is too short.", "destination");
        throw FormatException(invalidDataMessage());
    }

    /**
     * @brief Attempts to decode base64url ASCII chars to binary bytes.
     * C++ counterpart of .NET Base64Url.TryDecodeFromChars(ReadOnlySpan&lt;char&gt;, Span&lt;byte&gt;, out int).
     */
    static bool TryDecodeFromChars(System::ReadOnlySpan<char> source, System::Span<uint8_t> destination,
                                    intcs& bytesWritten) {
        intcs consumed = 0;
        OperationStatus status = DecodeFromChars(source, destination, consumed, bytesWritten);
        if (status == OperationStatus::InvalidData) throw FormatException(invalidDataMessage());
        return status == OperationStatus::Done;
    }

    /**
     * @brief Decodes base64url ASCII chars into a newly allocated buffer.
     * C++ counterpart of .NET Base64Url.DecodeFromChars(ReadOnlySpan&lt;char&gt;).
     */
    [[nodiscard]] static std::vector<uint8_t> DecodeFromChars(System::ReadOnlySpan<char> source) {
        intcs upperBound = GetMaxDecodedLength(source.getLengthProperty());
        std::vector<uint8_t> destination(static_cast<size_t>(upperBound));
        intcs consumed = 0, written = 0;
        System::Span<uint8_t> dst(destination.data(), upperBound);
        OperationStatus status = DecodeFromChars(source, dst, consumed, written);
        if (status != OperationStatus::Done) throw FormatException(invalidDataMessage());
        destination.resize(static_cast<size_t>(written));
        return destination;
    }

    // -----------------------------------------------------------------------
    // Validate
    // -----------------------------------------------------------------------

    /**
     * @brief Returns true if @p base64UrlText is valid base64url (no padding expected).
     * C++ counterpart of .NET Base64Url.IsValid(ReadOnlySpan&lt;byte&gt;).
     */
    [[nodiscard]] static bool IsValid(System::ReadOnlySpan<uint8_t> base64UrlText) {
        intcs decodedLength = 0;
        return validateCore<uint8_t>(base64UrlText.getPointer(), base64UrlText.getLengthProperty(), decodedLength);
    }

    /**
     * @brief Returns true if @p base64UrlText (char) is valid base64url.
     * C++ counterpart of .NET Base64Url.IsValid(ReadOnlySpan&lt;char&gt;).
     */
    [[nodiscard]] static bool IsValid(System::ReadOnlySpan<char> base64UrlText) {
        intcs decodedLength = 0;
        return validateCore<char>(base64UrlText.getPointer(), base64UrlText.getLengthProperty(), decodedLength);
    }

    /**
     * @brief Validates base64url input and returns true along with the decoded byte count.
     * C++ counterpart of .NET Base64Url.IsValid(ReadOnlySpan&lt;byte&gt;, out int).
     */
    [[nodiscard]] static bool IsValid(System::ReadOnlySpan<uint8_t> base64UrlText, intcs& decodedLength) {
        return validateCore<uint8_t>(base64UrlText.getPointer(), base64UrlText.getLengthProperty(), decodedLength);
    }

    /**
     * @brief Validates base64url char input and returns true along with the decoded byte count.
     * C++ counterpart of .NET Base64Url.IsValid(ReadOnlySpan&lt;char&gt;, out int).
     */
    [[nodiscard]] static bool IsValid(System::ReadOnlySpan<char> base64UrlText, intcs& decodedLength) {
        return validateCore<char>(base64UrlText.getPointer(), base64UrlText.getLengthProperty(), decodedLength);
    }
};

} // namespace System::Buffers::Text
