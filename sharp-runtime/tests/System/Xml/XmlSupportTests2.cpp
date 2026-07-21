// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Coverage for XmlQualifiedName, XmlConvert, NameTable, and the small XML interfaces.
#include <gtest/gtest.h>
#include <limits>
#include "System/ArgumentException.hpp"
#include "System/FormatException.hpp"
#include "System/Xml/IHasXmlNode.hpp"
#include "System/Xml/IXmlLineInfo.hpp"
#include "System/Xml/IXmlNamespaceResolver.hpp"
#include "System/Xml/NameTable.hpp"
#include "System/Xml/XmlConvert.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlNodeChangedEventArgs.hpp"
#include "System/Xml/XmlQualifiedName.hpp"

using namespace System::Xml;

// ===========================================================================
// XmlQualifiedName
// ===========================================================================

TEST(XmlQualifiedNameTests, DefaultCtor_IsEmpty) {
    XmlQualifiedName qn;
    EXPECT_TRUE(qn.getIsEmptyProperty());
}

TEST(XmlQualifiedNameTests, NameOnlyCtor_NamespaceIsEmpty) {
    XmlQualifiedName qn("foo");
    EXPECT_EQ(qn.getNameProperty(), "foo");
    EXPECT_EQ(qn.getNamespaceProperty(), "");
    EXPECT_FALSE(qn.getIsEmptyProperty());
}

TEST(XmlQualifiedNameTests, ToString_WithNamespace_UsesColonForm) {
    XmlQualifiedName qn("foo", "urn:bar");
    EXPECT_EQ(qn.ToString(), "urn:bar:foo");
}

TEST(XmlQualifiedNameTests, ToString_WithoutNamespace_IsJustName) {
    XmlQualifiedName qn("foo");
    EXPECT_EQ(qn.ToString(), "foo");
}

TEST(XmlQualifiedNameTests, Equals_SameNameAndNamespace) {
    XmlQualifiedName a("foo", "urn:bar");
    XmlQualifiedName b("foo", "urn:bar");
    EXPECT_TRUE(a == b);
}

TEST(XmlQualifiedNameTests, Equals_DifferentNamespace_NotEqual) {
    XmlQualifiedName a("foo", "urn:bar");
    XmlQualifiedName b("foo", "urn:baz");
    EXPECT_TRUE(a != b);
}

TEST(XmlQualifiedNameTests, Empty_IsStaticEmptyInstance) {
    EXPECT_TRUE(XmlQualifiedName::Empty.getIsEmptyProperty());
}

// ===========================================================================
// NameTable
// ===========================================================================

TEST(NameTableTests, Add_ThenGet_ReturnsAtomizedString) {
    NameTable nt;
    nt.Add("foo");
    auto result = nt.Get(std::string("foo"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, "foo");
}

TEST(NameTableTests, Get_NotAdded_ReturnsNullopt) {
    NameTable nt;
    EXPECT_FALSE(nt.Get(std::string("missing")).has_value());
}

TEST(NameTableTests, Add_SubstringOverload) {
    NameTable nt;
    const char* buf = "helloworld";
    std::string result = nt.Add(buf, 0, 5);
    EXPECT_EQ(result, "hello");
    EXPECT_TRUE(nt.Get(std::string("hello")).has_value());
}

// ===========================================================================
// XmlConvert — name encoding
// ===========================================================================

TEST(XmlConvertTests, EncodeName_ValidName_Unchanged) {
    EXPECT_EQ(XmlConvert::EncodeName("valid_name"), "valid_name");
}

TEST(XmlConvertTests, EncodeName_InvalidChar_Escaped) {
    std::string encoded = XmlConvert::EncodeName("a b");
    EXPECT_EQ(encoded, "a_x0020_b");
}

TEST(XmlConvertTests, DecodeName_ReversesEncodeName) {
    std::string original = "a b";
    std::string encoded = XmlConvert::EncodeName(original);
    EXPECT_EQ(XmlConvert::DecodeName(encoded), original);
}

TEST(XmlConvertTests, VerifyName_Valid_ReturnsUnchanged) {
    EXPECT_EQ(XmlConvert::VerifyName("foo:bar"), "foo:bar");
}

TEST(XmlConvertTests, VerifyName_StartsWithDigit_Throws) {
    EXPECT_THROW(XmlConvert::VerifyName("1abc"), XmlException);
}

TEST(XmlConvertTests, VerifyNCName_ContainsColon_Throws) {
    EXPECT_THROW(XmlConvert::VerifyNCName("foo:bar"), XmlException);
}

TEST(XmlConvertTests, VerifyName_Empty_ThrowsArgumentException) {
    EXPECT_THROW(XmlConvert::VerifyName(""), System::ArgumentException);
}

TEST(XmlConvertTests, VerifyNCName_Empty_ThrowsArgumentException) {
    EXPECT_THROW(XmlConvert::VerifyNCName(""), System::ArgumentException);
}

TEST(XmlConvertTests, VerifyNMTOKEN_Empty_ThrowsXmlException) {
    EXPECT_THROW(XmlConvert::VerifyNMTOKEN(""), XmlException);
}

TEST(XmlConvertTests, VerifyNMTOKEN_BadChar_ThrowsXmlException) {
    EXPECT_THROW(XmlConvert::VerifyNMTOKEN("foo bar"), XmlException);
}

TEST(XmlConvertTests, VerifyNMTOKEN_Valid_ReturnsUnchanged) {
    EXPECT_EQ(XmlConvert::VerifyNMTOKEN("foo.bar-1"), "foo.bar-1");
}

TEST(XmlConvertTests, VerifyTOKEN_Empty_ReturnsUnchanged) {
    EXPECT_EQ(XmlConvert::VerifyTOKEN(""), "");
}

TEST(XmlConvertTests, VerifyTOKEN_LeadingSpace_Throws) {
    EXPECT_THROW(XmlConvert::VerifyTOKEN(" foo"), XmlException);
}

TEST(XmlConvertTests, VerifyTOKEN_TrailingSpace_Throws) {
    EXPECT_THROW(XmlConvert::VerifyTOKEN("foo "), XmlException);
}

TEST(XmlConvertTests, VerifyTOKEN_DoubleSpace_Throws) {
    EXPECT_THROW(XmlConvert::VerifyTOKEN("foo  bar"), XmlException);
}

TEST(XmlConvertTests, VerifyTOKEN_Tab_Throws) {
    EXPECT_THROW(XmlConvert::VerifyTOKEN("foo\tbar"), XmlException);
}

TEST(XmlConvertTests, VerifyTOKEN_Valid_ReturnsUnchanged) {
    EXPECT_EQ(XmlConvert::VerifyTOKEN("foo bar"), "foo bar");
}

TEST(XmlConvertTests, IsStartNCNameChar_Letter_True) {
    EXPECT_TRUE(XmlConvert::IsStartNCNameChar('a'));
}

TEST(XmlConvertTests, IsStartNCNameChar_Digit_False) {
    EXPECT_FALSE(XmlConvert::IsStartNCNameChar('1'));
}

TEST(XmlConvertTests, IsWhitespaceChar_Space_True) {
    EXPECT_TRUE(XmlConvert::IsWhitespaceChar(' '));
}

// ===========================================================================
// XmlConvert — ToString/ToXxx round-trips
// ===========================================================================

TEST(XmlConvertTests, BooleanRoundTrip) {
    EXPECT_EQ(XmlConvert::ToString(true), "true");
    EXPECT_EQ(XmlConvert::ToString(false), "false");
    EXPECT_TRUE(XmlConvert::ToBoolean("true"));
    EXPECT_TRUE(XmlConvert::ToBoolean("1"));
    EXPECT_FALSE(XmlConvert::ToBoolean("0"));
}

TEST(XmlConvertTests, ToBoolean_InvalidString_ThrowsFormatException) {
    EXPECT_THROW(XmlConvert::ToBoolean("yes"), System::FormatException);
}

TEST(XmlConvertTests, Int32RoundTrip) {
    EXPECT_EQ(XmlConvert::ToString(static_cast<SharpRuntime::intcs>(-42)), "-42");
    EXPECT_EQ(XmlConvert::ToInt32("-42"), -42);
}

TEST(XmlConvertTests, Int64RoundTrip) {
    SharpRuntime::longcs v = 1234567890123LL;
    EXPECT_EQ(XmlConvert::ToInt64(XmlConvert::ToString(v)), v);
}

TEST(XmlConvertTests, DoubleRoundTrip) {
    double v = 3.14159;
    EXPECT_DOUBLE_EQ(XmlConvert::ToDouble(XmlConvert::ToString(v)), v);
}

TEST(XmlConvertTests, ToString_Double_PositiveInfinity_UsesXmlSchemaToken) {
    EXPECT_EQ(XmlConvert::ToString(std::numeric_limits<double>::infinity()), "INF");
}

TEST(XmlConvertTests, ToString_Double_NegativeInfinity_UsesXmlSchemaToken) {
    EXPECT_EQ(XmlConvert::ToString(-std::numeric_limits<double>::infinity()), "-INF");
}

TEST(XmlConvertTests, ToString_Float_Infinity_UsesXmlSchemaToken) {
    EXPECT_EQ(XmlConvert::ToString(std::numeric_limits<float>::infinity()), "INF");
    EXPECT_EQ(XmlConvert::ToString(-std::numeric_limits<float>::infinity()), "-INF");
}

TEST(XmlConvertTests, ToDouble_ParsesXmlSchemaInfinityTokens) {
    EXPECT_EQ(XmlConvert::ToDouble("INF"), std::numeric_limits<double>::infinity());
    EXPECT_EQ(XmlConvert::ToDouble("-INF"), -std::numeric_limits<double>::infinity());
    EXPECT_EQ(XmlConvert::ToDouble(" INF \t"), std::numeric_limits<double>::infinity());
}

TEST(XmlConvertTests, ToSingle_ParsesXmlSchemaInfinityTokens) {
    EXPECT_EQ(XmlConvert::ToSingle("INF"), std::numeric_limits<float>::infinity());
    EXPECT_EQ(XmlConvert::ToSingle("-INF"), -std::numeric_limits<float>::infinity());
}

TEST(XmlConvertTests, InfinityRoundTrip_Double) {
    double posInf = std::numeric_limits<double>::infinity();
    double negInf = -std::numeric_limits<double>::infinity();
    EXPECT_EQ(XmlConvert::ToDouble(XmlConvert::ToString(posInf)), posInf);
    EXPECT_EQ(XmlConvert::ToDouble(XmlConvert::ToString(negInf)), negInf);
}

// Regression tests for ticket 350: ToSingle/ToDouble/ToDecimal computed a trimmed copy of the
// input for the INF/-INF token check but then called Parse() with the ORIGINAL untrimmed string
// for the general numeric fallback path. Single::Parse/Double::Parse delegate to
// std::from_chars, which -- unlike .NET's float.Parse/double.Parse -- does not skip leading or
// trailing whitespace at all; Decimal::TryParse tolerates no whitespace whatsoever either. XML
// element/attribute text content commonly has surrounding whitespace from document formatting
// (e.g. "<value> 3.14 </value>"), so these previously threw FormatException for perfectly valid,
// common XML Schema numeric content. Verified against XmlConvert.cs, whose ToSingle/ToDouble/
// ToDecimal all explicitly pass NumberStyles.AllowLeadingWhite | AllowTrailingWhite.
TEST(XmlConvertTests, ToDouble_TrimsSurroundingWhitespaceBeforeNumericParse) {
    EXPECT_DOUBLE_EQ(XmlConvert::ToDouble(" 3.14 "), 3.14);
    EXPECT_DOUBLE_EQ(XmlConvert::ToDouble("\t3.14\n"), 3.14);
}

TEST(XmlConvertTests, ToSingle_TrimsSurroundingWhitespaceBeforeNumericParse) {
    EXPECT_FLOAT_EQ(XmlConvert::ToSingle(" 3.14 "), 3.14f);
}

TEST(XmlConvertTests, ToDecimal_TrimsSurroundingWhitespaceBeforeNumericParse) {
    EXPECT_NO_THROW(XmlConvert::ToDecimal(" 3.14 "));
    EXPECT_EQ(XmlConvert::ToDecimal(" 3.14 ").ToString(), "3.14");
}

TEST(XmlConvertTests, GuidRoundTrip) {
    System::Guid g = System::Guid::NewGuid();
    EXPECT_EQ(XmlConvert::ToGuid(XmlConvert::ToString(g)), g);
}

TEST(XmlConvertTests, DateTimeRoundTrip) {
    System::DateTime dt = System::DateTime::Parse("2024-03-15T10:30:00");
    System::DateTime roundtripped = XmlConvert::ToDateTime(XmlConvert::ToString(dt));
    EXPECT_EQ(roundtripped.ToString(), dt.ToString());
}

// ===========================================================================
// XmlNodeChangedEventArgs
// ===========================================================================

TEST(XmlNodeChangedEventArgsTests, StoresConstructorArguments) {
    XmlNodeChangedEventArgs args(nullptr, nullptr, nullptr, "old", "new", XmlNodeChangedAction::Change);
    EXPECT_EQ(args.getActionProperty(), XmlNodeChangedAction::Change);
    EXPECT_EQ(args.getOldValueProperty(), "old");
    EXPECT_EQ(args.getNewValueProperty(), "new");
}
