// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlConvert.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <limits>

#include "System/Boolean.hpp"
#include "System/Byte.hpp"
#include "System/Char.hpp"
#include "System/FormatException.hpp"
#include "System/Double.hpp"
#include "System/Int16.hpp"
#include "System/Int32.hpp"
#include "System/Int64.hpp"
#include "System/SByte.hpp"
#include "System/Single.hpp"
#include "System/UInt16.hpp"
#include "System/UInt32.hpp"
#include "System/UInt64.hpp"
#include "System/ArgumentException.hpp"
#include "System/Xml/XmlException.hpp"

namespace System::Xml {

    namespace {
        // Matches XmlConvert.cs's private WhitespaceChars array exactly.
        constexpr const char* WhitespaceChars = " \t\n\r";

        std::string TrimXmlWhitespace(const std::string& s) {
            auto begin = s.find_first_not_of(WhitespaceChars);
            if (begin == std::string::npos) return "";
            auto end = s.find_last_not_of(WhitespaceChars);
            return s.substr(begin, end - begin + 1);
        }
    } // namespace

    // --- Character classification (practical ASCII-range subset; see class doc-comment) ------

    bool XmlConvert::IsStartNCNameChar(SharpRuntime::charcs ch) {
        auto c = static_cast<unsigned int>(ch);
        if (c >= 128) return true; // permissive: treat all non-ASCII as valid name chars
        return std::isalpha(static_cast<int>(c)) || ch == '_';
    }

    bool XmlConvert::IsNCNameChar(SharpRuntime::charcs ch) {
        auto c = static_cast<unsigned int>(ch);
        if (c >= 128) return true;
        return std::isalnum(static_cast<int>(c)) || ch == '_' || ch == '-' || ch == '.';
    }

    bool XmlConvert::IsXmlChar(SharpRuntime::charcs ch) {
        auto c = static_cast<unsigned int>(ch);
        // XML 1.0 Char production: #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
        return c == 0x9 || c == 0xA || c == 0xD ||
               (c >= 0x20 && c <= 0xD7FF) ||
               (c >= 0xE000 && c <= 0xFFFD);
    }

    bool XmlConvert::IsXmlSurrogatePair(SharpRuntime::charcs lowChar, SharpRuntime::charcs highChar) {
        auto low = static_cast<unsigned int>(lowChar);
        auto high = static_cast<unsigned int>(highChar);
        if (high < 0xD800 || high > 0xDBFF || low < 0xDC00 || low > 0xDFFF) return false;
        unsigned int codePoint = 0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00);
        return codePoint <= 0x10FFFF;
    }

    bool XmlConvert::IsPublicIdChar(SharpRuntime::charcs ch) {
        static const std::string extra = " \r\n-'()+,./:=?;!*#@$_%";
        return std::isalnum(static_cast<unsigned char>(ch)) != 0 ||
               extra.find(static_cast<char>(ch)) != std::string::npos;
    }

    bool XmlConvert::IsWhitespaceChar(SharpRuntime::charcs ch) {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    // --- Name encoding -------------------------------------------------------------------

    namespace {
        std::string EncodeNameImpl(const std::string& name, bool isNmToken, bool isLocal) {
            if (name.empty()) return name;
            std::string result;
            result.reserve(name.size());
            for (size_t i = 0; i < name.size(); ++i) {
                char ch = name[i];
                bool isFirst = (i == 0) && !isLocal && !isNmToken;
                bool valid = isFirst ? XmlConvert::IsStartNCNameChar(ch) : XmlConvert::IsNCNameChar(ch);
                // A literal underscore that looks like the start of an escape ("_x....") must
                // itself be escaped, or DecodeName could misinterpret it on round-trip.
                bool looksLikeEscape = ch == '_' && i + 6 < name.size() && name[i + 1] == 'x' &&
                                        std::isxdigit(static_cast<unsigned char>(name[i + 2])) &&
                                        std::isxdigit(static_cast<unsigned char>(name[i + 3])) &&
                                        std::isxdigit(static_cast<unsigned char>(name[i + 4])) &&
                                        std::isxdigit(static_cast<unsigned char>(name[i + 5])) &&
                                        name[i + 6] == '_';
                if (valid && !looksLikeEscape) {
                    result += ch;
                } else {
                    char buf[16];
                    std::snprintf(buf, sizeof(buf), "_x%04X_", static_cast<unsigned int>(static_cast<unsigned char>(ch)));
                    result += buf;
                }
            }
            return result;
        }
    }

    std::string XmlConvert::EncodeName(const std::string& name) { return EncodeNameImpl(name, false, false); }
    std::string XmlConvert::EncodeNmToken(const std::string& name) { return EncodeNameImpl(name, true, false); }
    std::string XmlConvert::EncodeLocalName(const std::string& name) { return EncodeNameImpl(name, false, true); }

    std::string XmlConvert::DecodeName(const std::string& name) {
        if (name.empty()) return name;
        std::string result;
        result.reserve(name.size());
        size_t i = 0;
        while (i < name.size()) {
            if (name[i] == '_' && i + 6 < name.size() && name[i + 1] == 'x' && name[i + 6] == '_' &&
                std::isxdigit(static_cast<unsigned char>(name[i + 2])) &&
                std::isxdigit(static_cast<unsigned char>(name[i + 3])) &&
                std::isxdigit(static_cast<unsigned char>(name[i + 4])) &&
                std::isxdigit(static_cast<unsigned char>(name[i + 5]))) {
                unsigned int code = 0;
                std::sscanf(name.c_str() + i + 2, "%4x", &code);
                result += static_cast<char>(code);
                i += 7;
            } else {
                result += name[i];
                ++i;
            }
        }
        return result;
    }

    // --- Verification --------------------------------------------------------------------

    std::string XmlConvert::VerifyName(const std::string& name) {
        if (name.empty()) throw System::ArgumentException("The value cannot be an empty string.", "name");
        if (!IsStartNCNameChar(name[0]))
            throw XmlException("Invalid XML name: '" + name + "'.");
        for (size_t i = 1; i < name.size(); ++i)
            if (!IsNCNameChar(name[i]) && name[i] != ':')
                throw XmlException("Invalid XML name: '" + name + "'.");
        return name;
    }

    std::string XmlConvert::VerifyNCName(const std::string& name) {
        if (name.empty()) throw System::ArgumentException("The value cannot be an empty string.", "name");
        if (!IsStartNCNameChar(name[0]))
            throw XmlException("Invalid XML NCName: '" + name + "'.");
        for (size_t i = 1; i < name.size(); ++i)
            if (!IsNCNameChar(name[i]))
                throw XmlException("Invalid XML NCName: '" + name + "'.");
        return name;
    }

    std::string XmlConvert::VerifyTOKEN(const std::string& token) {
        if (token.empty()) return token;
        bool startsOrEndsWithSpace = token.front() == ' ' || token.back() == ' ';
        bool hasControlChar = token.find_first_of("\t\n\r") != std::string::npos;
        bool hasDoubleSpace = token.find("  ") != std::string::npos;
        if (startsOrEndsWithSpace || hasControlChar || hasDoubleSpace)
            throw XmlException(
                "line-feed (#xA) or tab (#x9) characters, leading or trailing spaces and sequences of "
                "one or more spaces (#x20) are not allowed in 'xs:token'.");
        return token;
    }

    std::string XmlConvert::VerifyNMTOKEN(const std::string& name) {
        if (name.empty()) throw XmlException("Invalid NmToken value '" + name + "'.");
        for (char ch : name)
            if (!IsNCNameChar(ch) && ch != ':')
                throw XmlException("Invalid NmToken value '" + name + "'.");
        return name;
    }

    std::string XmlConvert::VerifyXmlChars(const std::string& content) {
        for (char ch : content)
            if (!IsXmlChar(ch))
                throw XmlException("Invalid XML character in content.");
        return content;
    }

    std::string XmlConvert::VerifyPublicId(const std::string& publicId) {
        for (char ch : publicId)
            if (!IsPublicIdChar(ch))
                throw XmlException("Invalid character in public identifier: '" + publicId + "'.");
        return publicId;
    }

    std::string XmlConvert::VerifyWhitespace(const std::string& content) {
        for (char ch : content)
            if (!IsWhitespaceChar(ch))
                throw XmlException("Invalid whitespace character in content.");
        return content;
    }

    // --- ToString --------------------------------------------------------------------

    std::string XmlConvert::ToString(bool value) { return value ? "true" : "false"; }
    std::string XmlConvert::ToString(SharpRuntime::charcs value) { return System::Char::ToString(value); }
    std::string XmlConvert::ToString(System::Decimal value) { return value.ToString(); }
    std::string XmlConvert::ToString(SharpRuntime::sbytecs value) { return System::SByte::ToString(value); }
    std::string XmlConvert::ToString(SharpRuntime::shortcs value) { return System::Int16::ToString(value); }
    std::string XmlConvert::ToString(SharpRuntime::intcs value) { return System::Int32::ToString(value); }
    std::string XmlConvert::ToString(SharpRuntime::longcs value) { return System::Int64::ToString(value); }
    std::string XmlConvert::ToString(SharpRuntime::bytecs value) { return System::Byte::ToString(value); }
    std::string XmlConvert::ToString(SharpRuntime::ushortcs value) { return System::UInt16::ToString(value); }
    std::string XmlConvert::ToString(SharpRuntime::uintcs value) { return System::UInt32::ToString(value); }
    std::string XmlConvert::ToString(SharpRuntime::ulongcs value) { return System::UInt64::ToString(value); }
    // Verified against XmlConvert.cs's ToString(float)/ToString(double): real XmlConvert uses
    // the XML Schema lexical-space tokens "INF"/"-INF" for infinity, not .NET Double/Single's
    // general-purpose ToString() tokens "Infinity"/"-Infinity" -- output using the latter is
    // invalid per the XML Schema `double`/`float` lexical space.
    std::string XmlConvert::ToString(float value) {
        if (std::isinf(value)) return value < 0 ? "-INF" : "INF";
        return System::Single::ToString(value);
    }
    std::string XmlConvert::ToString(double value) {
        if (std::isinf(value)) return value < 0 ? "-INF" : "INF";
        return System::Double::ToString(value);
    }
    // KNOWN GAP (audited, not fixed -- see ToTimeSpan(string) below for the matching Parse-side
    // gap and the full rationale for leaving both undone here): real .NET's
    // XmlConvert.ToString(TimeSpan) formats using the XML Schema `duration` lexical form
    // (PnYnMnDTnHnMnS, e.g. "P1DT2H3M4S"), not .NET's native TimeSpan.ToString() colon-separated
    // format ("1.02:03:04") that this port's TimeSpan::ToString() produces.
    std::string XmlConvert::ToString(const System::TimeSpan& value) { return value.ToString(); }
    std::string XmlConvert::ToString(const System::DateTime& value) { return value.ToString(); }
    std::string XmlConvert::ToString(const System::DateTime& value, const std::string& format) { return value.ToString(format); }

    std::string XmlConvert::ToString(const System::DateTime& value, XmlDateTimeSerializationMode /*mode*/) {
        // System::DateTime does not track DateTimeKind (see its own doc-comment), so Local/Utc/
        // Unspecified/RoundtripKind cannot be distinguished here; all modes use the same
        // round-trip ISO 8601 format.
        return value.ToString();
    }

    std::string XmlConvert::ToString(const System::DateTimeOffset& value) { return value.ToString(); }
    std::string XmlConvert::ToString(const System::DateTimeOffset& value, const std::string& format) { return value.ToString(format); }
    std::string XmlConvert::ToString(const System::Guid& value) { return value.ToString(); }

    // --- ToXxx -----------------------------------------------------------------------

    bool XmlConvert::ToBoolean(const std::string& s) {
        auto notws = [](unsigned char c) { return !std::isspace(c); };
        auto b = std::find_if(s.begin(), s.end(), notws);
        auto e = std::find_if(s.rbegin(), s.rend(), notws).base();
        std::string t(b, e < b ? b : e);
        if (t == "1" || t == "true") return true;
        if (t == "0" || t == "false") return false;
        throw System::FormatException("String '" + s + "' was not recognized as a valid Boolean.");
    }
    SharpRuntime::charcs XmlConvert::ToChar(const std::string& s) { return System::Char::Parse(s); }
    // Verified against XmlConvert.cs's ToDecimal(string), which passes NumberStyles.
    // AllowLeadingWhite | AllowTrailingWhite to decimal.Parse. System::Decimal::TryParse
    // tolerates NO whitespace at all (any non-digit/dot/comma/sign character anywhere fails
    // immediately), so XML decimal content with surrounding whitespace -- common from document
    // formatting/indentation -- previously threw FormatException; same bug class as
    // ToSingle/ToDouble below.
    System::Decimal XmlConvert::ToDecimal(const std::string& s) { return System::Decimal::Parse(TrimXmlWhitespace(s)); }
    SharpRuntime::sbytecs XmlConvert::ToSByte(const std::string& s) { return System::SByte::Parse(s); }
    SharpRuntime::shortcs XmlConvert::ToInt16(const std::string& s) { return System::Int16::Parse(s); }
    SharpRuntime::intcs XmlConvert::ToInt32(const std::string& s) { return System::Int32::Parse(s); }
    SharpRuntime::longcs XmlConvert::ToInt64(const std::string& s) { return System::Int64::Parse(s); }
    SharpRuntime::bytecs XmlConvert::ToByte(const std::string& s) { return System::Byte::Parse(s); }
    SharpRuntime::ushortcs XmlConvert::ToUInt16(const std::string& s) { return System::UInt16::Parse(s); }
    SharpRuntime::uintcs XmlConvert::ToUInt32(const std::string& s) { return System::UInt32::Parse(s); }
    SharpRuntime::ulongcs XmlConvert::ToUInt64(const std::string& s) { return System::UInt64::Parse(s); }
    // Verified against XmlConvert.cs's ToSingle(string)/ToDouble(string): real XmlConvert trims
    // XML whitespace, then recognizes "-INF"/"INF" as the XML Schema lexical-space infinity
    // tokens before falling through to ordinary numeric parsing -- valid schema input like
    // "INF" previously failed to parse (Single::Parse/Double::Parse only recognize .NET's own
    // "Infinity"/"-Infinity" spelling).
    //
    // Also fixed here: the fallback path called Parse(s) with the ORIGINAL untrimmed string
    // instead of the already-computed `trimmed` one. Single::Parse/Double::Parse delegate to
    // std::from_chars, which -- unlike .NET's float.Parse/double.Parse -- does NOT skip leading
    // or trailing whitespace at all (confirmed via a standalone repro: from_chars fails outright
    // on " 3.14 ", never even reaching the digits). Since XML element/attribute text content
    // commonly has surrounding whitespace from document formatting/indentation (e.g.
    // "<value> 3.14 </value>"), this silently threw FormatException for extremely common,
    // perfectly valid XML Schema float/double content.
    float XmlConvert::ToSingle(const std::string& s) {
        std::string trimmed = TrimXmlWhitespace(s);
        if (trimmed == "-INF") return -std::numeric_limits<float>::infinity();
        if (trimmed == "INF") return std::numeric_limits<float>::infinity();
        return System::Single::Parse(trimmed);
    }
    double XmlConvert::ToDouble(const std::string& s) {
        std::string trimmed = TrimXmlWhitespace(s);
        if (trimmed == "-INF") return -std::numeric_limits<double>::infinity();
        if (trimmed == "INF") return std::numeric_limits<double>::infinity();
        return System::Double::Parse(trimmed);
    }
    // KNOWN GAP (audited, not fixed): real .NET's XmlConvert.ToTimeSpan parses the XML Schema
    // `duration` lexical form (PnYnMnDTnHnMnS, e.g. "P1DT2H3M4S") via a dedicated XsdDuration
    // parser -- an entirely different grammar from .NET's native TimeSpan.Parse() colon-
    // separated format ("1.02:03:04") that this port's TimeSpan::Parse() expects. A correct fix
    // needs a full XSD duration parser/formatter pair (matching ToString(TimeSpan) above) --
    // optional Y/M/D/H/M/S components, sign handling, fractional seconds, and .NET's specific
    // TimeSpan-from-duration conversion rules (e.g. how a duration's Y/M components, which have
    // no fixed length, map onto TimeSpan's fixed-length representation) -- a substantially
    // larger undertaking than a single-pass audit fix, so documented rather than attempted here
    // (same call as several other structural gaps found this session: too large/risky to rush).
    System::TimeSpan XmlConvert::ToTimeSpan(const std::string& s) { return System::TimeSpan::Parse(s); }
    System::DateTime XmlConvert::ToDateTime(const std::string& s) { return System::DateTime::Parse(s); }
    System::DateTime XmlConvert::ToDateTime(const std::string& s, const std::string& /*format*/) { return System::DateTime::Parse(s); }
    System::DateTime XmlConvert::ToDateTime(const std::string& s, XmlDateTimeSerializationMode /*mode*/) { return System::DateTime::Parse(s); }
    System::DateTimeOffset XmlConvert::ToDateTimeOffset(const std::string& s) { return System::DateTimeOffset::Parse(s); }
    System::DateTimeOffset XmlConvert::ToDateTimeOffset(const std::string& s, const std::string& /*format*/) { return System::DateTimeOffset::Parse(s); }
    System::Guid XmlConvert::ToGuid(const std::string& s) { return System::Guid::Parse(s); }

} // namespace System::Xml
