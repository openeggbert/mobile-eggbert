// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Guid.hpp"
#include "System/FormatException.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/OverflowException.hpp"
#include "System/DateTimeOffset.hpp"

#include <random>
#include <algorithm>
#include <utility>

namespace System {

    namespace {

        constexpr const char* MSG_UNRECOGNIZED       = "Unrecognized Guid format.";
        constexpr const char* MSG_INVLEN             = "Guid should contain 32 digits with 4 dashes (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx).";
        constexpr const char* MSG_DASHES             = "Dashes are in the wrong position for GUID parsing.";
        constexpr const char* MSG_INVALIDCHAR        = "Guid string should only contain hexadecimal characters.";
        constexpr const char* MSG_BRACE              = "Expected {0xdddddddd, etc}.";
        constexpr const char* MSG_COMMA              = "Could not find a comma, or the length between the previous token and the comma was zero (i.e., '0x,'etc.).";
        constexpr const char* MSG_HEXPREFIX          = "Expected 0x prefix.";
        constexpr const char* MSG_BRACE_AFTER_LAST   = "Could not find a brace, or the length between the previous token and the brace was zero (i.e., '0x,'etc.).";
        constexpr const char* MSG_ENDBRACE           = "Could not find the ending brace.";
        constexpr const char* MSG_EXTRAJUNK          = "Additional non-parsable characters are at the end of the string.";
        constexpr const char* MSG_OVERFLOW_U32       = "Value was either too large or too small for a UInt32.";
        constexpr const char* MSG_OVERFLOW_BYTE      = "Value was either too large or too small for an unsigned byte.";
        constexpr const char* MSG_INVALID_FORMAT_SPEC = "Format string can be only \"D\", \"d\", \"N\", \"n\", \"P\", \"p\", \"B\", \"b\", \"X\" or \"x\".";
        constexpr const char* MSG_ARG_GUID_16        = "Byte array for Guid must be exactly 16 bytes long.";

        enum class GuidParseStatus { Ok, Format, Overflow };

        bool IsAsciiWhitespace(char c) noexcept {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
        }

        std::string Trim(const std::string& s) {
            std::size_t b = 0, e = s.size();
            while (b < e && IsAsciiWhitespace(s[b])) ++b;
            while (e > b && IsAsciiWhitespace(s[e - 1])) --e;
            return s.substr(b, e - b);
        }

        std::string StripAsciiWhitespace(const std::string& s) {
            std::string out;
            out.reserve(s.size());
            for (char c : s) if (!IsAsciiWhitespace(c)) out.push_back(c);
            return out;
        }

        int HexVal(char c) noexcept {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        }

        bool DecodeByte(char hi, char lo, bytecs& out) noexcept {
            int h = HexVal(hi), l = HexVal(lo);
            if (h < 0 || l < 0) return false;
            out = static_cast<bytecs>((h << 4) | l);
            return true;
        }

        // Parses a run of hex digits (no "0x" prefix; the caller has already consumed
        // it) into a 32-bit value. Sets overflow=true when more than 8 significant hex
        // digits are present, matching .NET's Guid "X"-format component parsing (Guid.cs
        // TryParseHex). Real .NET counts significant digits as it goes and, upon hitting
        // an invalid character, reports overflow (not a format error) if more than 8
        // significant digits were already consumed -- overflow takes precedence over an
        // invalid trailing character. This must mirror that exactly: counting only the
        // *successfully processed* digits via `processedDigits`, not the raw span length,
        // matters for a string like "123456789z" (9 valid digits, then invalid 'z').
        bool TryParseHexRun(const std::string& s, uint32_t& outVal, bool& overflow) {
            if (s.empty()) return false;
            std::size_t i = 0;
            while (i < s.size() && s[i] == '0') ++i;
            uint64_t acc = 0;
            std::size_t processedDigits = 0;
            for (; i < s.size(); ++i) {
                int v = HexVal(s[i]);
                if (v < 0) {
                    if (processedDigits > 8) overflow = true;
                    return false;
                }
                acc = (acc << 4) | static_cast<uint32_t>(v);
                ++processedDigits;
            }
            if (processedDigits > 8) overflow = true;
            outVal = static_cast<uint32_t>(acc & 0xFFFFFFFFull);
            return true;
        }

        std::array<bytecs, 16> ReverseComponentEndianness(std::array<bytecs, 16> b) {
            std::swap(b[0], b[3]);
            std::swap(b[1], b[2]);
            std::swap(b[4], b[5]);
            std::swap(b[6], b[7]);
            return b;
        }

        std::string HexFixed(uint32_t v, int digits) {
            static constexpr char kDigits[] = "0123456789abcdef";
            std::string s(static_cast<std::size_t>(digits), '0');
            for (int i = digits - 1; i >= 0; --i) {
                s[static_cast<std::size_t>(i)] = kDigits[v & 0xF];
                v >>= 4;
            }
            return s;
        }

        // e.g. "d85b1407-351d-4694-9392-03acc5870eb1"
        GuidParseStatus TryParseExactD(const std::string& s, std::array<bytecs, 16>& bytes, std::string& err) {
            if (s.size() != 36 || s[8] != '-' || s[13] != '-' || s[18] != '-' || s[23] != '-') {
                err = (s.size() != 36) ? MSG_INVLEN : MSG_DASHES;
                return GuidParseStatus::Format;
            }
            static constexpr std::size_t idx[16][2] = {
                {0, 1}, {2, 3}, {4, 5}, {6, 7},
                {9, 10}, {11, 12},
                {14, 15}, {16, 17},
                {19, 20}, {21, 22}, {24, 25}, {26, 27}, {28, 29}, {30, 31}, {32, 33}, {34, 35}
            };
            for (int k = 0; k < 16; ++k) {
                bytecs v;
                if (!DecodeByte(s[idx[k][0]], s[idx[k][1]], v)) {
                    err = MSG_INVALIDCHAR;
                    return GuidParseStatus::Format;
                }
                bytes[static_cast<std::size_t>(k)] = v;
            }
            return GuidParseStatus::Ok;
        }

        // e.g. "d85b1407351d4694939203acc5870eb1"
        GuidParseStatus TryParseExactN(const std::string& s, std::array<bytecs, 16>& bytes, std::string& err) {
            if (s.size() != 32) {
                err = MSG_INVLEN;
                return GuidParseStatus::Format;
            }
            for (int k = 0; k < 16; ++k) {
                bytecs v;
                if (!DecodeByte(s[static_cast<std::size_t>(k) * 2], s[static_cast<std::size_t>(k) * 2 + 1], v)) {
                    err = MSG_INVALIDCHAR;
                    return GuidParseStatus::Format;
                }
                bytes[static_cast<std::size_t>(k)] = v;
            }
            return GuidParseStatus::Ok;
        }

        // e.g. "{d85b1407-351d-4694-9392-03acc5870eb1}"
        GuidParseStatus TryParseExactB(const std::string& s, std::array<bytecs, 16>& bytes, std::string& err) {
            if (s.size() != 38 || s.front() != '{' || s.back() != '}') {
                err = MSG_INVLEN;
                return GuidParseStatus::Format;
            }
            return TryParseExactD(s.substr(1, 36), bytes, err);
        }

        // e.g. "(d85b1407-351d-4694-9392-03acc5870eb1)"
        GuidParseStatus TryParseExactP(const std::string& s, std::array<bytecs, 16>& bytes, std::string& err) {
            if (s.size() != 38 || s.front() != '(' || s.back() != ')') {
                err = MSG_INVLEN;
                return GuidParseStatus::Format;
            }
            return TryParseExactD(s.substr(1, 36), bytes, err);
        }

        // e.g. "{0xd85b1407,0x351d,0x4694,{0x93,0x92,0x03,0xac,0xc5,0x87,0x0e,0xb1}}"
        GuidParseStatus TryParseExactX(const std::string& sIn, std::array<bytecs, 16>& bytes, std::string& err) {
            const std::string s = StripAsciiWhitespace(sIn);

            if (s.empty() || s[0] != '{') {
                err = MSG_BRACE;
                return GuidParseStatus::Format;
            }

            auto isHexPrefix = [&](std::size_t i) {
                return i + 1 < s.size() && s[i] == '0' && (s[i + 1] == 'x' || s[i + 1] == 'X');
            };

            auto parseComponent = [&](std::size_t& pos, uint32_t& outVal, char terminator) -> GuidParseStatus {
                if (!isHexPrefix(pos)) {
                    err = MSG_HEXPREFIX;
                    return GuidParseStatus::Format;
                }
                std::size_t numStart = pos + 2;
                std::size_t termPos = s.find(terminator, numStart);
                if (termPos == std::string::npos || termPos == numStart) {
                    err = (terminator == ',') ? MSG_COMMA : MSG_BRACE_AFTER_LAST;
                    return GuidParseStatus::Format;
                }
                bool overflow = false;
                bool ok = TryParseHexRun(s.substr(numStart, termPos - numStart), outVal, overflow);
                if (!ok || overflow) {
                    // Matches real .NET's precedence: overflow (9+ significant digits
                    // already consumed) is reported even if an invalid character follows.
                    err = overflow ? MSG_OVERFLOW_U32 : MSG_INVALIDCHAR;
                    return overflow ? GuidParseStatus::Overflow : GuidParseStatus::Format;
                }
                pos = termPos + 1;
                return GuidParseStatus::Ok;
            };

            std::size_t pos = 1;
            uint32_t a = 0, b32 = 0, c32 = 0;
            if (auto st = parseComponent(pos, a, ','); st != GuidParseStatus::Ok) return st;
            if (auto st = parseComponent(pos, b32, ','); st != GuidParseStatus::Ok) return st;
            if (auto st = parseComponent(pos, c32, ','); st != GuidParseStatus::Ok) return st;

            if (pos >= s.size() || s[pos] != '{') {
                err = MSG_BRACE;
                return GuidParseStatus::Format;
            }
            ++pos;

            uint32_t byteVals[8] = {};
            for (int k = 0; k < 8; ++k) {
                const char terminator = (k < 7) ? ',' : '}';
                uint32_t v = 0;
                auto st = parseComponent(pos, v, terminator);
                if (st != GuidParseStatus::Ok) return st;
                if (v > 0xFFu) {
                    err = MSG_OVERFLOW_BYTE;
                    return GuidParseStatus::Overflow;
                }
                byteVals[k] = v;
            }

            if (pos >= s.size() || s[pos] != '}') {
                err = MSG_ENDBRACE;
                return GuidParseStatus::Format;
            }
            ++pos;
            if (pos != s.size()) {
                err = MSG_EXTRAJUNK;
                return GuidParseStatus::Format;
            }

            const auto bb = static_cast<uint16_t>(b32 & 0xFFFFu);
            const auto cc = static_cast<uint16_t>(c32 & 0xFFFFu);
            bytes[0] = static_cast<bytecs>((a >> 24) & 0xFF);
            bytes[1] = static_cast<bytecs>((a >> 16) & 0xFF);
            bytes[2] = static_cast<bytecs>((a >> 8) & 0xFF);
            bytes[3] = static_cast<bytecs>(a & 0xFF);
            bytes[4] = static_cast<bytecs>((bb >> 8) & 0xFF);
            bytes[5] = static_cast<bytecs>(bb & 0xFF);
            bytes[6] = static_cast<bytecs>((cc >> 8) & 0xFF);
            bytes[7] = static_cast<bytecs>(cc & 0xFF);
            for (int k = 0; k < 8; ++k) bytes[static_cast<std::size_t>(8 + k)] = static_cast<bytecs>(byteVals[k]);
            return GuidParseStatus::Ok;
        }

        GuidParseStatus DispatchExact(char formatChar, const std::string& s, std::array<bytecs, 16>& bytes, std::string& err) {
            switch (static_cast<char>(formatChar | 0x20)) {
                case 'd': return TryParseExactD(s, bytes, err);
                case 'n': return TryParseExactN(s, bytes, err);
                case 'b': return TryParseExactB(s, bytes, err);
                case 'p': return TryParseExactP(s, bytes, err);
                case 'x': return TryParseExactX(s, bytes, err);
                default:
                    err = MSG_INVALID_FORMAT_SPEC;
                    return GuidParseStatus::Format;
            }
        }

        // Auto-detects the format the way .NET's Guid.Parse/TryParse/constructor do.
        GuidParseStatus TryParseLoose(const std::string& sIn, std::array<bytecs, 16>& bytes, std::string& err) {
            const std::string s = Trim(sIn);
            if (s.size() < 32) {
                err = MSG_UNRECOGNIZED;
                return GuidParseStatus::Format;
            }
            if (s[0] == '(') return TryParseExactP(s, bytes, err);
            if (s[0] == '{') return (s[9] == '-') ? TryParseExactB(s, bytes, err) : TryParseExactX(s, bytes, err);
            return (s[8] == '-') ? TryParseExactD(s, bytes, err) : TryParseExactN(s, bytes, err);
        }

    } // namespace

    const Guid Guid::Empty{};

    Guid::Guid() : bytes_{} {}

    Guid::Guid(const std::array<bytecs, 16>& b) : bytes_(ReverseComponentEndianness(b)) {}

    Guid::Guid(const std::array<bytecs, 16>& b, bool bigEndian)
        : bytes_(bigEndian ? b : ReverseComponentEndianness(b)) {}

    Guid::Guid(const ReadOnlySpan<bytecs>& b) {
        if (b.getLengthProperty() != 16) throw ArgumentException(MSG_ARG_GUID_16, "b");
        std::array<bytecs, 16> tmp{};
        std::copy(b.getPointer(), b.getPointer() + 16, tmp.begin());
        bytes_ = ReverseComponentEndianness(tmp);
    }

    Guid::Guid(const ReadOnlySpan<bytecs>& b, bool bigEndian) {
        if (b.getLengthProperty() != 16) throw ArgumentException(MSG_ARG_GUID_16, "b");
        std::array<bytecs, 16> tmp{};
        std::copy(b.getPointer(), b.getPointer() + 16, tmp.begin());
        bytes_ = bigEndian ? tmp : ReverseComponentEndianness(tmp);
    }

    Guid::Guid(uintcs a, ushortcs b, ushortcs c,
               bytecs d, bytecs e, bytecs f, bytecs g, bytecs h, bytecs i, bytecs j, bytecs k) {
        bytes_[0] = static_cast<bytecs>((a >> 24) & 0xFF);
        bytes_[1] = static_cast<bytecs>((a >> 16) & 0xFF);
        bytes_[2] = static_cast<bytecs>((a >> 8) & 0xFF);
        bytes_[3] = static_cast<bytecs>(a & 0xFF);
        bytes_[4] = static_cast<bytecs>((b >> 8) & 0xFF);
        bytes_[5] = static_cast<bytecs>(b & 0xFF);
        bytes_[6] = static_cast<bytecs>((c >> 8) & 0xFF);
        bytes_[7] = static_cast<bytecs>(c & 0xFF);
        bytes_[8] = d; bytes_[9] = e; bytes_[10] = f; bytes_[11] = g;
        bytes_[12] = h; bytes_[13] = i; bytes_[14] = j; bytes_[15] = k;
    }

    Guid::Guid(intcs a, shortcs b, shortcs c, const std::array<bytecs, 8>& d)
        : Guid(static_cast<uintcs>(a), static_cast<ushortcs>(b), static_cast<ushortcs>(c),
               d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]) {}

    Guid::Guid(intcs a, shortcs b, shortcs c,
               bytecs d, bytecs e, bytecs f, bytecs g, bytecs h, bytecs i, bytecs j, bytecs k)
        : Guid(static_cast<uintcs>(a), static_cast<ushortcs>(b), static_cast<ushortcs>(c),
               d, e, f, g, h, i, j, k) {}

    Guid::Guid(const std::string& guidString) : Guid(Parse(guidString)) {}

    Guid Guid::getAllBitsSetProperty() {
        std::array<bytecs, 16> b{};
        b.fill(0xFF);
        return FromCanonicalBytes(b);
    }

    Guid Guid::NewGuid() {
        static std::mt19937_64 rng{std::random_device{}()};
        static std::uniform_int_distribution<uint32_t> dist(0, 255);

        std::array<bytecs, 16> b{};
        for (auto& byte : b) byte = static_cast<bytecs>(dist(rng));
        // RFC 4122 version 4
        b[6] = (b[6] & 0x0F) | 0x40;
        b[8] = (b[8] & 0x3F) | 0x80;
        return FromCanonicalBytes(b);
    }

    Guid Guid::CreateVersion7() {
        return CreateVersion7(DateTimeOffset::getUtcNowProperty());
    }

    Guid Guid::CreateVersion7(const DateTimeOffset& timestamp) {
        Guid result = NewGuid();

        const longcs unixTsMs = timestamp.ToUnixTimeMilliseconds();
        ArgumentOutOfRangeException::ThrowIfNegative(unixTsMs, "timestamp");

        const auto ts = static_cast<uint64_t>(unixTsMs);
        const auto aVal = static_cast<uint32_t>(ts >> 16);
        const auto bVal = static_cast<uint16_t>(ts & 0xFFFFu);

        result.bytes_[0] = static_cast<bytecs>((aVal >> 24) & 0xFF);
        result.bytes_[1] = static_cast<bytecs>((aVal >> 16) & 0xFF);
        result.bytes_[2] = static_cast<bytecs>((aVal >> 8) & 0xFF);
        result.bytes_[3] = static_cast<bytecs>(aVal & 0xFF);
        result.bytes_[4] = static_cast<bytecs>((bVal >> 8) & 0xFF);
        result.bytes_[5] = static_cast<bytecs>(bVal & 0xFF);

        // Version7Value (0x7000) in the top nibble of _c.
        result.bytes_[6] = static_cast<bytecs>((result.bytes_[6] & 0x0F) | 0x70);
        // Variant10xxValue (0x80) in the top 2 bits of _d.
        result.bytes_[8] = static_cast<bytecs>((result.bytes_[8] & 0x3F) | 0x80);

        return result;
    }

    Guid Guid::Parse(const std::string& s) {
        std::array<bytecs, 16> bytes{};
        std::string err;
        const GuidParseStatus st = TryParseLoose(s, bytes, err);
        if (st == GuidParseStatus::Overflow) throw OverflowException(err);
        if (st == GuidParseStatus::Format) throw FormatException(err);
        return FromCanonicalBytes(bytes);
    }

    Guid Guid::Parse(const ReadOnlySpan<char>& s) {
        return Parse(std::string(s.getPointer(), static_cast<std::size_t>(s.getLengthProperty())));
    }

    Guid Guid::Parse(const ReadOnlySpan<bytecs>& utf8Text) {
        return Parse(std::string(reinterpret_cast<const char*>(utf8Text.getPointer()),
                                  static_cast<std::size_t>(utf8Text.getLengthProperty())));
    }

    Guid Guid::Parse(const std::string& s, const IFormatProvider* provider) {
        (void)provider;
        return Parse(s);
    }

    bool Guid::TryParse(const std::string& s, Guid& result) {
        std::array<bytecs, 16> bytes{};
        std::string err;
        if (TryParseLoose(s, bytes, err) != GuidParseStatus::Ok) return false;
        result = FromCanonicalBytes(bytes);
        return true;
    }

    bool Guid::TryParse(const ReadOnlySpan<char>& s, Guid& result) {
        return TryParse(std::string(s.getPointer(), static_cast<std::size_t>(s.getLengthProperty())), result);
    }

    bool Guid::TryParse(const ReadOnlySpan<bytecs>& utf8Text, Guid& result) {
        return TryParse(std::string(reinterpret_cast<const char*>(utf8Text.getPointer()),
                                     static_cast<std::size_t>(utf8Text.getLengthProperty())), result);
    }

    bool Guid::TryParse(const std::string& s, const IFormatProvider* provider, Guid& result) {
        (void)provider;
        return TryParse(s, result);
    }

    Guid Guid::ParseExact(const std::string& s, const std::string& format) {
        if (format.size() != 1) throw FormatException(MSG_INVALID_FORMAT_SPEC);
        std::array<bytecs, 16> bytes{};
        std::string err;
        const GuidParseStatus st = DispatchExact(format[0], Trim(s), bytes, err);
        if (st == GuidParseStatus::Overflow) throw OverflowException(err);
        if (st == GuidParseStatus::Format) throw FormatException(err);
        return FromCanonicalBytes(bytes);
    }

    bool Guid::TryParseExact(const std::string& s, const std::string& format, Guid& result) {
        if (format.size() != 1) return false;
        std::array<bytecs, 16> bytes{};
        std::string err;
        if (DispatchExact(format[0], Trim(s), bytes, err) != GuidParseStatus::Ok) return false;
        result = FromCanonicalBytes(bytes);
        return true;
    }

    std::string Guid::ToStringCore(const std::string& format) const {
        char f = 'd';
        if (!format.empty()) {
            if (format.size() != 1) throw FormatException(MSG_INVALID_FORMAT_SPEC);
            f = static_cast<char>(format[0] | 0x20);
        }

        const uint32_t a = getAProperty();
        const uint16_t b = getBProperty();
        const uint16_t c = getCProperty();

        switch (f) {
            case 'n': {
                std::string out;
                out.reserve(32);
                for (bytecs byte : bytes_) out += HexFixed(byte, 2);
                return out;
            }
            case 'd':
            case 'b':
            case 'p': {
                std::string d;
                d.reserve(36);
                d += HexFixed(a, 8);
                d += '-';
                d += HexFixed(b, 4);
                d += '-';
                d += HexFixed(c, 4);
                d += '-';
                d += HexFixed(bytes_[8], 2);
                d += HexFixed(bytes_[9], 2);
                d += '-';
                for (int idx = 10; idx < 16; ++idx) d += HexFixed(bytes_[static_cast<std::size_t>(idx)], 2);
                if (f == 'd') return d;
                if (f == 'b') return "{" + d + "}";
                return "(" + d + ")";
            }
            case 'x': {
                std::string out = "{0x" + HexFixed(a, 8) + ",0x" + HexFixed(b, 4) + ",0x" + HexFixed(c, 4) + ",{";
                for (int idx = 0; idx < 8; ++idx) {
                    out += "0x" + HexFixed(bytes_[static_cast<std::size_t>(8 + idx)], 2);
                    if (idx < 7) out += ',';
                }
                out += "}}";
                return out;
            }
            default:
                throw FormatException(MSG_INVALID_FORMAT_SPEC);
        }
    }

    std::string Guid::ToString() const { return ToStringCore(""); }

    std::string Guid::ToString(const std::string& format) const { return ToStringCore(format); }

    std::string Guid::ToString(const std::string& format, const IFormatProvider* provider) const {
        (void)provider;
        return ToStringCore(format);
    }

    bool Guid::TryFormat(Span<char> destination, intcs& charsWritten, const std::string& format) const {
        const std::string s = ToStringCore(format);
        if (static_cast<intcs>(s.size()) > destination.getLengthProperty()) {
            charsWritten = 0;
            return false;
        }
        std::copy(s.begin(), s.end(), destination.getPointer());
        charsWritten = static_cast<intcs>(s.size());
        return true;
    }

    bool Guid::TryFormatUtf8(Span<bytecs> utf8Destination, intcs& bytesWritten, const std::string& format) const {
        const std::string s = ToStringCore(format);
        if (static_cast<intcs>(s.size()) > utf8Destination.getLengthProperty()) {
            bytesWritten = 0;
            return false;
        }
        std::transform(s.begin(), s.end(), utf8Destination.getPointer(),
                        [](char c) { return static_cast<bytecs>(c); });
        bytesWritten = static_cast<intcs>(s.size());
        return true;
    }

    std::array<bytecs, 16> Guid::ToByteArray() const { return ReverseComponentEndianness(bytes_); }

    std::array<bytecs, 16> Guid::ToByteArray(bool bigEndian) const {
        return bigEndian ? bytes_ : ReverseComponentEndianness(bytes_);
    }

    bool Guid::TryWriteBytes(Span<bytecs> destination) const {
        if (destination.getLengthProperty() < 16) return false;
        const auto arr = ToByteArray();
        std::copy(arr.begin(), arr.end(), destination.getPointer());
        return true;
    }

    bool Guid::TryWriteBytes(Span<bytecs> destination, bool bigEndian, intcs& bytesWritten) const {
        if (destination.getLengthProperty() < 16) {
            bytesWritten = 0;
            return false;
        }
        const auto arr = ToByteArray(bigEndian);
        std::copy(arr.begin(), arr.end(), destination.getPointer());
        bytesWritten = 16;
        return true;
    }

    intcs Guid::GetHashCode() const {
        const auto n = ToByteArray();
        auto readLE32 = [&](std::size_t off) -> intcs {
            return static_cast<intcs>(
                static_cast<uint32_t>(n[off]) |
                (static_cast<uint32_t>(n[off + 1]) << 8) |
                (static_cast<uint32_t>(n[off + 2]) << 16) |
                (static_cast<uint32_t>(n[off + 3]) << 24));
        };
        return readLE32(0) ^ readLE32(4) ^ readLE32(8) ^ readLE32(12);
    }

    intcs Guid::CompareTo(const Guid& other) const noexcept {
        if (bytes_ < other.bytes_) return -1;
        if (bytes_ > other.bytes_) return 1;
        return 0;
    }

} // namespace System
