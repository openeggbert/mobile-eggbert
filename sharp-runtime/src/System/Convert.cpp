// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Convert.hpp"
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"
#include "System/Math.hpp"
#include "System/OverflowException.hpp"
#include "System/Boolean.hpp"
#include "System/Double.hpp"
#include "System/Int32.hpp"
#include "System/Int64.hpp"
#include "System/UInt32.hpp"
#include "System/UInt64.hpp"

#include <stdexcept>
#include <cstdlib>
#include <cerrno>
#include <array>
#include <charconv>
#include <climits>
#include <limits>
#include <sstream>
#include <iomanip>
#include <bitset>

namespace System {

    using SharpRuntime::ushortcs;
    using SharpRuntime::sbytecs;

    namespace {
        // Used only by ToInt32(string, fromBase) now -- the plain decimal ToInt32(string)/
        // ToInt64(string) overloads delegate to Int32::Parse/Int64::Parse below instead (see
        // their comments). Real .NET's Convert.ToInt32(string, int fromBase) restricts fromBase
        // to {2, 8, 10, 16} (ParseNumbers.StringToLong throws ArgumentException otherwise) --
        // this previously accepted any base strtoll itself supports (0, 2-36), silently
        // producing a result for a base real .NET would reject outright.
        intcs parseIntBase(const std::string& value, int base) {
            if (base != 2 && base != 8 && base != 10 && base != 16)
                throw System::ArgumentException("Invalid Base.");
            if (value.empty()) throw FormatException("Input string was not in a correct format.");
            errno = 0;
            char* end = nullptr;
            long long result = std::strtoll(value.c_str(), &end, base);
            if (end == value.c_str() || *end != '\0')
                throw FormatException("Input string was not in a correct format.");
            if (errno == ERANGE || result > INT32_MAX || result < INT32_MIN)
                throw OverflowException("Value was either too large or too small for an Int32.");
            return static_cast<intcs>(result);
        }
    }

    intcs Convert::ToInt32(const std::string& value) {
        // Verified against Convert.cs's ToInt32(string): delegates straight to int.Parse(value),
        // whose default NumberStyles.Integer tolerates leading AND trailing whitespace (plus a
        // leading sign). This previously reimplemented parsing inline via strtoll, which skips
        // leading whitespace but rejects ANY trailing character including whitespace -- so
        // Convert::ToInt32(" 42 ") (a very natural call, e.g. from ported C# reading user input)
        // threw FormatException where real .NET succeeds. System::Int32::Parse already
        // correctly implements this trim behavior (see its own doc comment), matching the
        // established fix pattern already applied to ToDouble/ToBoolean/ToUInt32/ToUInt64
        // below/above in this same file.
        return Int32::Parse(value);
    }
    intcs Convert::ToInt32(double value) {
        // Real .NET's Convert.ToInt32(double) rounds to the nearest integer, ties to even,
        // matching Math.Round's default -- it previously truncated toward zero instead (e.g.
        // ToInt32(2.9) returned 2, not 3), a confirmed post-stabilization-audit finding.
        double rounded = Math::Round(value);
        if (rounded > INT_MAX || rounded < INT_MIN) throw OverflowException();
        return static_cast<intcs>(rounded);
    }
    intcs Convert::ToInt32(float value) {
        return ToInt32(static_cast<double>(value));
    }
    intcs Convert::ToInt32(longcs value) {
        if (value > INT_MAX || value < INT_MIN) throw OverflowException();
        return static_cast<intcs>(value);
    }
    intcs Convert::ToInt32(const std::string& value, intcs fromBase) {
        return parseIntBase(value, fromBase);
    }

    longcs Convert::ToInt64(const std::string& value) {
        // Same fix and rationale as ToInt32(string) above -- delegates to Int64::Parse, which
        // already correctly tolerates leading/trailing whitespace matching real .NET's
        // long.Parse default NumberStyles.Integer.
        return Int64::Parse(value);
    }
    longcs Convert::ToInt64(double value) {
        // Real .NET: checked((long)Math.Round(value)) -- rounds to nearest (ties to even), then
        // throws OverflowException outside long's range. Previously this was a bare
        // static_cast<longcs>(value) with NO rounding and NO overflow check at all --
        // ToInt64(1e20) returned LLONG_MIN (a silent wraparound) instead of throwing, a confirmed
        // post-stabilization-audit finding. The boundary compares against 2^63
        // (9223372036854775808.0), not LLONG_MAX, because LLONG_MAX (2^63-1) is not exactly
        // representable as a double -- the nearest representable double to LLONG_MAX is 2^63
        // itself, so comparing against a cast LLONG_MAX would incorrectly accept some
        // just-out-of-range inputs.
        double rounded = Math::Round(value);
        if (rounded < -9223372036854775808.0 || rounded >= 9223372036854775808.0)
            throw OverflowException();
        return static_cast<longcs>(rounded);
    }

    shortcs Convert::ToInt16(const std::string& value) {
        intcs v = ToInt32(value);
        if (v > 32767 || v < -32768) throw OverflowException();
        return static_cast<shortcs>(v);
    }
    shortcs Convert::ToInt16(intcs value) {
        if (value > 32767 || value < -32768) throw OverflowException();
        return static_cast<shortcs>(value);
    }
    shortcs Convert::ToInt16(longcs value) {
        if (value > 32767 || value < -32768) throw OverflowException();
        return static_cast<shortcs>(value);
    }

    double Convert::ToDouble(const std::string& value) {
        // Verified against Convert.cs's ToDouble(string): delegates straight to
        // double.Parse(value). Previously this reimplemented parsing inline with the same
        // "special-case exact NaN/Infinity/-Infinity strings, then fall through to
        // std::from_chars" pattern that was found to silently accept abbreviated spellings
        // like "inf"/"nan(123)" in ticket 245 (Double::Parse) -- from_chars's floating-point
        // grammar recognizes those regardless of chars_format, a C++ standard requirement.
        // Delegating to the already-fixed Double::Parse avoids duplicating that bug here.
        return Double::Parse(value);
    }

    Single Convert::ToSingle(const std::string& value) {
        return static_cast<Single>(ToDouble(value));
    }

    bytecs Convert::ToByte(intcs value) {
        if (value < 0 || value > 255) throw OverflowException();
        return static_cast<bytecs>(value);
    }
    bytecs Convert::ToByte(const std::string& value) {
        return ToByte(ToInt32(value));
    }

    bool Convert::ToBoolean(const std::string& value) {
        // Verified against Convert.cs's ToBoolean(string): delegates to bool.Parse(value),
        // which (per Boolean.cs's TryParse) matches "true"/"false" case-insensitively (with
        // optional surrounding whitespace) and nothing else -- "1"/"0" throw FormatException
        // in real .NET. This previously accepted "1"/"0" as valid and only matched the two
        // exact casings "True"/"true" and "False"/"false" rather than being fully
        // case-insensitive (e.g. "TRUE" or "tRuE" would have thrown). System::Boolean::Parse
        // already correctly implements .NET's case-insensitive-with-trim semantics.
        return Boolean::Parse(value);
    }

    std::string Convert::ToString(intcs value)   { return std::to_string(value); }
    std::string Convert::ToString(longcs value)  { return std::to_string(value); }
    std::string Convert::ToString(double value) {
        std::array<char, 64> buf;
        auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return ec == std::errc{} ? std::string(buf.data(), ptr) : std::to_string(value);
    }
    std::string Convert::ToString(float value) {
        std::array<char, 32> buf;
        auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value);
        return ec == std::errc{} ? std::string(buf.data(), ptr) : std::to_string(value);
    }
    std::string Convert::ToString(char value)    { return std::string(1, value); }
    std::string Convert::ToString(bytecs value)  { return std::to_string(static_cast<int>(value)); }

    std::string Convert::ToString(intcs value, intcs toBase) {
        if (toBase == 10) return std::to_string(value);
        if (toBase == 16) {
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << std::hex << value;
            return oss.str();
        }
        if (toBase == 2) {
            if (value == 0) return "0";
            std::string result;
            auto uv = static_cast<uint32_t>(value);
            while (uv) { result = (char)('0' + (uv & 1)) + result; uv >>= 1; }
            return result;
        }
        if (toBase == 8) {
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << std::oct << value;
            return oss.str();
        }
        throw FormatException("Invalid base.");
    }

    std::string Convert::ToHexString(const std::vector<bytecs>& inArray)
    {
        // Verified against Convert.cs: ToHexString delegates to
        // HexConverter.ToString(bytes, HexConverter.Casing.Upper) -- real .NET's ToHexString
        // (unlike ToHexStringLower) is uppercase. This previously used a lowercase table
        // (swapped with what is now ToHexStringLower below), silently producing the wrong
        // casing under the correct .NET method name for any caller expecting real .NET's
        // Convert.ToHexString output (e.g. PhysicalAddress.ToString(), which real .NET
        // implements as `Convert.ToHexString(_address)`).
        static constexpr char hex[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(inArray.size() * 2);
        for (bytecs b : inArray) {
            result += hex[(b >> 4) & 0xF];
            result += hex[b & 0xF];
        }
        return result;
    }

    std::string Convert::ToHexStringLower(const std::vector<bytecs>& inArray)
    {
        // Verified against Convert.cs: ToHexStringLower delegates to
        // HexConverter.ToString(bytes, HexConverter.Casing.Lower). This method was previously
        // named ToHexStringUpper despite producing uppercase output nowhere -- a method name
        // that doesn't exist in real .NET at all -- while the real .NET Convert.ToHexString
        // name was attached to the lowercase implementation above. Renamed to match .NET's
        // actual API surface.
        static constexpr char hex[] = "0123456789abcdef";
        std::string result;
        result.reserve(inArray.size() * 2);
        for (bytecs b : inArray) {
            result += hex[(b >> 4) & 0xF];
            result += hex[b & 0xF];
        }
        return result;
    }

    std::vector<SharpRuntime::bytecs> Convert::FromHexString(const std::string& s)
    {
        if (s.size() % 2 != 0)
            throw FormatException("Hex string length must be even.");
        std::vector<bytecs> result;
        result.reserve(s.size() / 2);
        for (size_t i = 0; i < s.size(); i += 2) {
            auto hexVal = [](char c) -> uint8_t {
                if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
                if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
                if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
                throw FormatException("Invalid hex character.");
                return 0;
            };
            result.push_back(static_cast<bytecs>((hexVal(s[i]) << 4) | hexVal(s[i + 1])));
        }
        return result;
    }

    ushortcs Convert::ToUInt16(const std::string& value)
    {
        intcs v = ToInt32(value);
        if (v < 0 || v > 65535) throw OverflowException("Value is out of UInt16 range.");
        return static_cast<ushortcs>(v);
    }

    sbytecs Convert::ToSByte(const std::string& value)
    {
        intcs v = ToInt32(value);
        if (v < -128 || v > 127) throw OverflowException("Value is out of SByte range.");
        return static_cast<sbytecs>(v);
    }

    uint32_t Convert::ToUInt32(const std::string& value)
    {
        // Previously called std::stoul directly: on bad format or overflow, std::stoul
        // throws raw std::invalid_argument/std::out_of_range, not the System::FormatException/
        // System::OverflowException this class's own header documents -- the same raw-std::-
        // exception-escape bug class found repeatedly elsewhere this session. Verified real
        // .NET's Convert.ToUInt32(string) delegates to uint.Parse(value); System::UInt32::Parse
        // already correctly translates std:: exceptions to System:: ones.
        return UInt32::Parse(value);
    }

    uint64_t Convert::ToUInt64(const std::string& value)
    {
        // Same fix as ToUInt32(string) above -- verified real .NET's Convert.ToUInt64(string)
        // delegates to ulong.Parse(value).
        return UInt64::Parse(value);
    }

    static constexpr char kB64Chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string Convert::ToBase64String(const std::vector<bytecs>& inArray)
    {
        std::string out;
        size_t n = inArray.size();
        out.reserve(((n + 2) / 3) * 4);
        for (size_t i = 0; i < n; i += 3) {
            uint32_t triple = static_cast<uint32_t>(inArray[i]) << 16;
            if (i + 1 < n) triple |= static_cast<uint32_t>(inArray[i + 1]) << 8;
            if (i + 2 < n) triple |= static_cast<uint32_t>(inArray[i + 2]);
            out += kB64Chars[(triple >> 18) & 0x3F];
            out += kB64Chars[(triple >> 12) & 0x3F];
            out += (i + 1 < n) ? kB64Chars[(triple >> 6) & 0x3F] : '=';
            out += (i + 2 < n) ? kB64Chars[triple & 0x3F]        : '=';
        }
        return out;
    }

    std::vector<SharpRuntime::bytecs> Convert::FromBase64String(const std::string& s)
    {
        auto b64val = [](char c) -> int {
            if (c >= 'A' && c <= 'Z') return c - 'A';
            if (c >= 'a' && c <= 'z') return c - 'a' + 26;
            if (c >= '0' && c <= '9') return c - '0' + 52;
            if (c == '+') return 62;
            if (c == '/') return 63;
            if (c == '=') return -1;
            throw FormatException("Invalid Base64 character.");
            return 0;
        };
        if (s.size() % 4 != 0) throw FormatException("Base64 string length must be a multiple of 4.");
        std::vector<bytecs> out;
        out.reserve(s.size() / 4 * 3);
        for (size_t i = 0; i < s.size(); i += 4) {
            int v0 = b64val(s[i]), v1 = b64val(s[i+1]);
            int v2 = b64val(s[i+2]), v3 = b64val(s[i+3]);
            out.push_back(static_cast<bytecs>((v0 << 2) | (v1 >> 4)));
            if (v2 != -1) out.push_back(static_cast<bytecs>(((v1 & 0xF) << 4) | (v2 >> 2)));
            if (v3 != -1) out.push_back(static_cast<bytecs>(((v2 & 0x3) << 6) | v3));
        }
        return out;
    }

} // namespace System
