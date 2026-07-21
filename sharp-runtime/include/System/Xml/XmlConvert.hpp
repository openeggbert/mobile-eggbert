// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/DateTime.hpp"
#include "System/DateTimeOffset.hpp"
#include "System/Decimal.hpp"
#include "System/Guid.hpp"
#include "System/TimeSpan.hpp"
#include "System/Xml/XmlDateTimeSerializationMode.hpp"

namespace System::Xml {

    /**
     * @brief Encodes/decodes XML names and converts between XML string representations and
     * culture-invariant CLR types.
     *
     * C++ counterpart of .NET System.Xml.XmlConvert. The ToString/ToXxx family delegates to this
     * runtime's existing System::* primitive wrapper types (Boolean, Int32, Double, DateTime,
     * etc.), whose ToString()/Parse() already use culture-invariant, round-trip-safe formatting —
     * the same contract XmlConvert provides in real .NET.
     *
     * @note Simplification: EncodeName/DecodeName/the character-classification helpers
     * (IsStartNCNameChar etc.) use a practical ASCII-range definition of valid XML name
     * characters plus a permissive allowance for non-ASCII code points, rather than replicating
     * the full Unicode NameChar production tables from the XML 1.0 specification (Appendix B).
     * This covers the overwhelming majority of real-world XML names (identifiers, tag names)
     * used by ported game code.
     */
    class XmlConvert {
    public:
        // --- Name encoding -----------------------------------------------------------------

        /** @brief Encodes a name (e.g. a DataTable/DataColumn name) into a valid XML Name, escaping invalid characters as "_xHHHH_". */
        [[nodiscard]] static std::string EncodeName(const std::string& name);
        /** @brief Encodes a name into a valid XML Nmtoken. */
        [[nodiscard]] static std::string EncodeNmToken(const std::string& name);
        /** @brief Encodes a name into a valid XML local name. */
        [[nodiscard]] static std::string EncodeLocalName(const std::string& name);
        /** @brief Reverses EncodeName/EncodeNmToken/EncodeLocalName, restoring "_xHHHH_" escapes to their original characters. */
        [[nodiscard]] static std::string DecodeName(const std::string& name);

        // --- Name validation -----------------------------------------------------------------

        /** @brief Verifies @p name is a valid XML Name (production [5]); returns it unchanged, or throws XmlException. */
        [[nodiscard]] static std::string VerifyName(const std::string& name);
        /** @brief Verifies @p name is a valid XML NCName (no colon); returns it unchanged, or throws XmlException. */
        [[nodiscard]] static std::string VerifyNCName(const std::string& name);
        /** @brief Verifies @p token is a valid XML Nmtoken; returns it unchanged, or throws XmlException. */
        [[nodiscard]] static std::string VerifyTOKEN(const std::string& token);
        /** @brief Verifies @p name is a valid XML Nmtoken (throws ArgumentException on failure, unlike VerifyTOKEN). */
        [[nodiscard]] static std::string VerifyNMTOKEN(const std::string& name);
        /** @brief Verifies every character of @p content is a valid XML character; returns it unchanged, or throws XmlException. */
        [[nodiscard]] static std::string VerifyXmlChars(const std::string& content);
        /** @brief Verifies @p publicId contains only valid PubidChar characters; returns it unchanged, or throws XmlException. */
        [[nodiscard]] static std::string VerifyPublicId(const std::string& publicId);
        /** @brief Verifies @p content contains only XML whitespace characters; returns it unchanged, or throws XmlException. */
        [[nodiscard]] static std::string VerifyWhitespace(const std::string& content);

        /** @brief Returns true if @p ch is a valid first character of an XML NCName. */
        [[nodiscard]] static bool IsStartNCNameChar(SharpRuntime::charcs ch);
        /** @brief Returns true if @p ch is a valid non-first character of an XML NCName. */
        [[nodiscard]] static bool IsNCNameChar(SharpRuntime::charcs ch);
        /** @brief Returns true if @p ch is a valid XML character per the XML 1.0 Char production. */
        [[nodiscard]] static bool IsXmlChar(SharpRuntime::charcs ch);
        /** @brief Returns true if the UTF-16 surrogate pair (@p lowChar, @p highChar) forms a valid XML character. */
        [[nodiscard]] static bool IsXmlSurrogatePair(SharpRuntime::charcs lowChar, SharpRuntime::charcs highChar);
        /** @brief Returns true if @p ch is a valid PubidChar (DTD PUBLIC identifier character). */
        [[nodiscard]] static bool IsPublicIdChar(SharpRuntime::charcs ch);
        /** @brief Returns true if @p ch is XML whitespace (space, tab, CR, or LF). */
        [[nodiscard]] static bool IsWhitespaceChar(SharpRuntime::charcs ch);

        // --- ToString ---------------------------------------------------------------------

        [[nodiscard]] static std::string ToString(bool value);
        [[nodiscard]] static std::string ToString(SharpRuntime::charcs value);
        [[nodiscard]] static std::string ToString(System::Decimal value);
        [[nodiscard]] static std::string ToString(SharpRuntime::sbytecs value);
        [[nodiscard]] static std::string ToString(SharpRuntime::shortcs value);
        [[nodiscard]] static std::string ToString(SharpRuntime::intcs value);
        [[nodiscard]] static std::string ToString(SharpRuntime::longcs value);
        [[nodiscard]] static std::string ToString(SharpRuntime::bytecs value);
        [[nodiscard]] static std::string ToString(SharpRuntime::ushortcs value);
        [[nodiscard]] static std::string ToString(SharpRuntime::uintcs value);
        [[nodiscard]] static std::string ToString(SharpRuntime::ulongcs value);
        [[nodiscard]] static std::string ToString(float value);
        [[nodiscard]] static std::string ToString(double value);
        [[nodiscard]] static std::string ToString(const System::TimeSpan& value);
        [[nodiscard]] static std::string ToString(const System::DateTime& value);
        [[nodiscard]] static std::string ToString(const System::DateTime& value, const std::string& format);
        [[nodiscard]] static std::string ToString(const System::DateTime& value, XmlDateTimeSerializationMode mode);
        [[nodiscard]] static std::string ToString(const System::DateTimeOffset& value);
        [[nodiscard]] static std::string ToString(const System::DateTimeOffset& value, const std::string& format);
        [[nodiscard]] static std::string ToString(const System::Guid& value);

        // --- ToXxx ------------------------------------------------------------------------

        [[nodiscard]] static bool ToBoolean(const std::string& s);
        [[nodiscard]] static SharpRuntime::charcs ToChar(const std::string& s);
        [[nodiscard]] static System::Decimal ToDecimal(const std::string& s);
        [[nodiscard]] static SharpRuntime::sbytecs ToSByte(const std::string& s);
        [[nodiscard]] static SharpRuntime::shortcs ToInt16(const std::string& s);
        [[nodiscard]] static SharpRuntime::intcs ToInt32(const std::string& s);
        [[nodiscard]] static SharpRuntime::longcs ToInt64(const std::string& s);
        [[nodiscard]] static SharpRuntime::bytecs ToByte(const std::string& s);
        [[nodiscard]] static SharpRuntime::ushortcs ToUInt16(const std::string& s);
        [[nodiscard]] static SharpRuntime::uintcs ToUInt32(const std::string& s);
        [[nodiscard]] static SharpRuntime::ulongcs ToUInt64(const std::string& s);
        [[nodiscard]] static float ToSingle(const std::string& s);
        [[nodiscard]] static double ToDouble(const std::string& s);
        [[nodiscard]] static System::TimeSpan ToTimeSpan(const std::string& s);
        [[nodiscard]] static System::DateTime ToDateTime(const std::string& s);
        [[nodiscard]] static System::DateTime ToDateTime(const std::string& s, const std::string& format);
        [[nodiscard]] static System::DateTime ToDateTime(const std::string& s, XmlDateTimeSerializationMode mode);
        [[nodiscard]] static System::DateTimeOffset ToDateTimeOffset(const std::string& s);
        [[nodiscard]] static System::DateTimeOffset ToDateTimeOffset(const std::string& s, const std::string& format);
        [[nodiscard]] static System::Guid ToGuid(const std::string& s);
    };

} // namespace System::Xml
