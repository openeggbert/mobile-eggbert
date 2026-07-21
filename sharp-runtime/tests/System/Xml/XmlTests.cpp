// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// XmlReader/XmlWriter are backed by vendored tinyxml2 (see XmlReader.cpp/XmlWriter.cpp).
// System::Xml::Linq types (XName, XAttribute, XElement, XDocument) are fully implemented,
// including real Parse()/Load() (tinyxml2-backed) and Save(). See XLinqNodeTests.cpp for the
// full XObject/XNode/XContainer/XText/XComment/XCData/XProcessingInstruction/XDocumentType/
// XStreamingElement/comparer/Extensions coverage. See XmlDomTests.cpp for the classic
// XmlDocument DOM API and XmlSupportTests.cpp for XmlException/XmlConvert/XmlQualifiedName/enum
// coverage.
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include "System/NotImplementedException.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/Linq/XName.hpp"
#include "System/Xml/Linq/XNamespace.hpp"
#include "System/Xml/Linq/XAttribute.hpp"
#include "System/Xml/Linq/XCData.hpp"
#include "System/Xml/Linq/XElement.hpp"
#include "System/Xml/Linq/XDocument.hpp"

using System::NotImplementedException;
using System::Xml::ReadState;
using System::Xml::XmlException;
using System::Xml::XmlNodeType;
using System::Xml::XmlReader;
using System::Xml::XmlWriter;
using System::Xml::XmlWriterSettings;
using System::Xml::Linq::XName;
using System::Xml::Linq::XNamespace;
using System::Xml::Linq::XAttribute;
using System::Xml::Linq::XCData;
using System::Xml::Linq::XElement;
using System::Xml::Linq::XDeclaration;
using System::Xml::Linq::XDocument;

// ===========================================================================
// ReadState enum
// ===========================================================================

TEST(ReadStateTests, Initial_IsZero) {
    EXPECT_EQ(static_cast<int>(ReadState::Initial), 0);
}

TEST(ReadStateTests, Interactive_IsOne) {
    EXPECT_EQ(static_cast<int>(ReadState::Interactive), 1);
}

TEST(ReadStateTests, EndOfFile_IsThree) {
    EXPECT_EQ(static_cast<int>(ReadState::EndOfFile), 3);
}

TEST(ReadStateTests, Closed_IsFour) {
    EXPECT_EQ(static_cast<int>(ReadState::Closed), 4);
}

// ===========================================================================
// XmlNodeType enum
// ===========================================================================

TEST(XmlNodeTypeTests, None_IsZero) {
    EXPECT_EQ(static_cast<int>(XmlNodeType::None), 0);
}

TEST(XmlNodeTypeTests, Element_IsOne) {
    EXPECT_EQ(static_cast<int>(XmlNodeType::Element), 1);
}

TEST(XmlNodeTypeTests, Text_IsThree) {
    EXPECT_EQ(static_cast<int>(XmlNodeType::Text), 3);
}

TEST(XmlNodeTypeTests, Comment_IsEight) {
    EXPECT_EQ(static_cast<int>(XmlNodeType::Comment), 8);
}

TEST(XmlNodeTypeTests, XmlDeclaration_Is17) {
    EXPECT_EQ(static_cast<int>(XmlNodeType::XmlDeclaration), 17);
}

// ===========================================================================
// XmlReader — tinyxml2-backed implementation
// ===========================================================================

static const char* kSimpleXml =
    "<root attr=\"hello\"><child>text</child><empty/></root>";

TEST(XmlReaderTests, CreateFromString_DoesNotThrow) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    EXPECT_NE(r, nullptr);
}

// Regression tests for ticket 349: Create(inputUri)'s file-vs-content heuristic previously
// treated ANY string containing '/' as a file path -- misclassifying extremely common XML
// content (self-closing tags like "<br/>", URLs in attribute/text content) and sending it to
// LoadFile(), which fails and throws a misleading "parse error" for perfectly valid XML text.
// A whitespace-trimmed string starting with '<' is now checked first and always treated as
// content, since a file path essentially never starts with '<'.
TEST(XmlReaderTests, Create_ContentWithSelfClosingTag_ParsesAsContentNotFilePath) {
    // "<br/>" contains '/', which the old heuristic misread as a path separator.
    std::unique_ptr<XmlReader> r(XmlReader::Create("<root><br/></root>"));
    EXPECT_TRUE(r->Read());
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Element);
    EXPECT_EQ(r->getNameProperty(), "root");
}

TEST(XmlReaderTests, Create_ContentWithUrlAttribute_ParsesAsContentNotFilePath) {
    // A namespace-URI-style attribute value contains multiple '/' characters.
    std::unique_ptr<XmlReader> r(XmlReader::Create("<root xmlns=\"http://example.com/ns\"/>"));
    EXPECT_TRUE(r->Read());
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Element);
}

TEST(XmlReaderTests, Create_LeadingWhitespaceThenContent_ParsesAsContent) {
    std::unique_ptr<XmlReader> r(XmlReader::Create("   \n<root/>"));
    EXPECT_TRUE(r->Read());
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Element);
}

TEST(XmlReaderTests, FirstRead_ReturnsTrueAndElementNode) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    EXPECT_TRUE(r->Read());
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Element);
}

TEST(XmlReaderTests, FirstElement_NameIsRoot) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    r->Read();
    EXPECT_EQ(r->getNameProperty(), "root");
}

TEST(XmlReaderTests, GetAttribute_ReturnsValue) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    r->Read(); // <root>
    EXPECT_EQ(r->GetAttribute("attr"), "hello");
}

TEST(XmlReaderTests, GetAttribute_Missing_ReturnsEmpty) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    r->Read();
    EXPECT_EQ(r->GetAttribute("nonexistent"), "");
}

TEST(XmlReaderTests, ReadSequence_ChildElement) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    r->Read(); // <root>
    r->Read(); // <child>
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Element);
    EXPECT_EQ(r->getNameProperty(), "child");
}

TEST(XmlReaderTests, ReadSequence_TextNode) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    r->Read(); // <root>
    r->Read(); // <child>
    r->Read(); // text
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Text);
    EXPECT_EQ(r->getValueProperty(), "text");
}

TEST(XmlReaderTests, ReadSequence_EndElement) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    r->Read(); // <root>
    r->Read(); // <child>
    r->Read(); // text
    r->Read(); // </child>
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::EndElement);
    EXPECT_EQ(r->getNameProperty(), "child");
}

TEST(XmlReaderTests, EmptyElement_IsEmptyElement) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(kSimpleXml));
    r->Read(); // <root>
    r->Read(); // <child>
    r->Read(); // text
    r->Read(); // </child>
    r->Read(); // <empty/>
    EXPECT_EQ(r->getNameProperty(), "empty");
    EXPECT_TRUE(r->getIsEmptyElementProperty());
}

TEST(XmlReaderTests, ReadPastEnd_ReturnsFalse) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<x/>"));
    while (r->Read()) {}
    EXPECT_FALSE(r->Read());
    EXPECT_EQ(r->getReadStateProperty(), ReadState::EndOfFile);
}

// Regression test for a wave-3 audit finding: most accessors only guarded pos < 0, not
// pos >= events.size() -- after Read() returns false at EOF, pos sits exactly at
// events.size(), so every one of these indexed events[pos] out of bounds (UB/crash) instead
// of returning a safe default. Only getNodeTypeProperty() had the correct upper-bound check.
TEST(XmlReaderTests, AccessorsAfterEOF_ReturnSafeDefaults_DoNotCrash) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<x a=\"1\"><y/></x>"));
    while (r->Read()) {}
    ASSERT_EQ(r->getReadStateProperty(), ReadState::EndOfFile);

    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::None);
    EXPECT_EQ(r->getNameProperty(), "");
    EXPECT_EQ(r->getValueProperty(), "");
    EXPECT_FALSE(r->getIsEmptyElementProperty());
    EXPECT_FALSE(r->MoveToElement());
    EXPECT_FALSE(r->MoveToNextAttribute());
    EXPECT_EQ(r->GetAttribute("a"), "");
    EXPECT_THROW(r->ReadStartElement(), XmlException);
    EXPECT_THROW(r->ReadEndElement(), XmlException);
}

TEST(XmlReaderTests, InitialState_IsInitial) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<x/>"));
    EXPECT_EQ(r->getReadStateProperty(), ReadState::Initial);
}

TEST(XmlReaderTests, Close_DoesNotThrow) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<x/>"));
    EXPECT_NO_THROW(r->Close());
}

TEST(XmlReaderTests, MoveToNextAttribute_IteratesAttributes) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<el a=\"1\" b=\"2\"/>"));
    r->Read(); // <el>
    EXPECT_TRUE(r->MoveToNextAttribute());
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Attribute);
    EXPECT_EQ(r->getNameProperty(), "a");
    EXPECT_EQ(r->getValueProperty(), "1");
    EXPECT_TRUE(r->MoveToNextAttribute());
    EXPECT_EQ(r->getNameProperty(), "b");
    EXPECT_FALSE(r->MoveToNextAttribute()); // no more
}

TEST(XmlReaderTests, MoveToElement_AfterAttributes) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<el a=\"1\"/>"));
    r->Read();
    r->MoveToNextAttribute();
    EXPECT_TRUE(r->MoveToElement());
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Element);
}

TEST(XmlReaderTests, XmlDeclaration_NodeType) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(
        "<?xml version=\"1.0\"?><root/>"));
    r->Read(); // declaration
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::XmlDeclaration);
}

TEST(XmlReaderTests, ReadElementContentAsString_ReturnsText) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<msg>hello world</msg>"));
    r->Read(); // <msg>
    std::string text = r->ReadElementContentAsString();
    EXPECT_EQ(text, "hello world");
}

// Regression test for ticket 349: ReadElementContentAsString() didn't track element nesting
// depth, so a NESTED child element's own EndElement event was indistinguishable from the
// enclosing element's end. For "<a><b>x</b>y</a>", the old code hit <b>'s EndElement first,
// mistook it for </a>, returned "x" (dropping "y" entirely), and left the reader positioned
// mid-content (on the Text("y") node) instead of past </a> -- corrupting every subsequent Read()
// call, not just the returned string. Verifies both the returned content and that the reader
// ends up correctly positioned past the enclosing element's own EndElement.
TEST(XmlReaderTests, ReadElementContentAsString_NestedElement_DoesNotCorruptPositionOrTruncate) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<root><a><b>x</b>y</a>z</root>"));
    r->Read(); // <root>
    r->Read(); // <a>
    (void)r->ReadElementContentAsString(); // must consume through </a>, whatever it returns
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Text);
    EXPECT_EQ(r->getValueProperty(), "z");
}

// Regression test for a wave-3 audit finding: CDATA sections were reported as plain
// XmlNodeType::Text (tinyxml2's XMLText::CData() flag was never consulted), so a reader
// could not distinguish `<![CDATA[...]]>` from ordinary text -- matches XmlTextReaderImpl.cs,
// which reports XmlNodeType.CDATA for CDATA sections.
TEST(XmlReaderTests, CDataSection_ReportsCDataNodeType) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<msg><![CDATA[a<b]]></msg>"));
    r->Read(); // <msg>
    r->Read(); // CDATA
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::CDATA);
    EXPECT_EQ(r->getValueProperty(), "a<b");
}

// Regression test: a real processing instruction (target != "xml") was previously
// misreported as XmlNodeType::XmlDeclaration with a hardcoded name of "xml" -- tinyxml2
// parses every `<?target data?>` form uniformly, so this port has to split target vs. data
// itself. Matches XmlTextReaderImpl.cs, which only treats a target of exactly "xml" as the
// document declaration.
TEST(XmlReaderTests, ProcessingInstruction_ReportsPINodeTypeAndSplitsTargetData) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(
        "<?xml-stylesheet type=\"text/xsl\" href=\"style.xsl\"?><root/>"));
    r->Read();
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::ProcessingInstruction);
    EXPECT_EQ(r->getNameProperty(), "xml-stylesheet");
    EXPECT_EQ(r->getValueProperty(), "type=\"text/xsl\" href=\"style.xsl\"");
}

// Regression test: a real `<?xml ...?>` declaration's Value used to include the "xml"
// target token itself ("xml version=\"1.0\""); real .NET's XmlDeclaration.Value is
// everything after the target, e.g. "version=\"1.0\"".
TEST(XmlReaderTests, XmlDeclaration_ValueExcludesTargetToken) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(
        "<?xml version=\"1.0\"?><root/>"));
    r->Read();
    EXPECT_EQ(r->getNameProperty(), "xml");
    EXPECT_EQ(r->getValueProperty(), "version=\"1.0\"");
}

// Regression test for a wave-3 audit finding: DOCTYPE declarations parse as tinyxml2's
// XMLUnknown, which buildEvents() had no branch for at all -- the node silently vanished
// from the event stream instead of surfacing as XmlNodeType::DocumentType.
TEST(XmlReaderTests, DocumentType_ReportsDocumentTypeNodeType) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(
        "<!DOCTYPE root><root/>"));
    r->Read();
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::DocumentType);
    EXPECT_EQ(r->getNameProperty(), "root");
}

// Regression test for a wave-3 audit finding: isEmptyElement was computed from
// `!FirstChild()`, so an explicitly-closed empty element (`<a></a>`) was indistinguishable
// from a self-closing one (`<a/>`) and silently lost its EndElement event. Real .NET's
// IsEmptyElement is only true for the `<a/>` empty-tag syntax; `<a></a>` always produces a
// separate EndElement node.
TEST(XmlReaderTests, ExplicitlyClosedEmptyElement_IsNotEmptyElement_GetsEndElement) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<root><a></a></root>"));
    r->Read(); // <root>
    r->Read(); // <a>
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::Element);
    EXPECT_EQ(r->getNameProperty(), "a");
    EXPECT_FALSE(r->getIsEmptyElementProperty());
    r->Read();
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::EndElement);
    EXPECT_EQ(r->getNameProperty(), "a");
}

TEST(XmlReaderTests, SelfClosingEmptyElement_IsEmptyElement_NoSeparateEndElement) {
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString("<root><a/></root>"));
    r->Read(); // <root>
    r->Read(); // <a/>
    EXPECT_TRUE(r->getIsEmptyElementProperty());
    r->Read();
    EXPECT_EQ(r->getNodeTypeProperty(), XmlNodeType::EndElement);
    EXPECT_EQ(r->getNameProperty(), "root");
}

// ===========================================================================
// XmlWriter — tinyxml2-backed implementation
// ===========================================================================

TEST(XmlWriterTests, CreateToString_DoesNotThrow) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    EXPECT_NE(w, nullptr);
}

TEST(XmlWriterTests, SimpleElement_ToString_ContainsTag) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("root");
    w->WriteEndElement();
    EXPECT_NE(w->ToString().find("<root"), std::string::npos);
}

TEST(XmlWriterTests, WriteAttributeString_InOutput) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("el");
    w->WriteAttributeString("key", "val");
    w->WriteEndElement();
    std::string out = w->ToString();
    EXPECT_NE(out.find("key=\"val\""), std::string::npos);
}

TEST(XmlWriterTests, WriteString_InOutput) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("msg");
    w->WriteString("hello");
    w->WriteEndElement();
    EXPECT_NE(w->ToString().find("hello"), std::string::npos);
}

TEST(XmlWriterTests, WriteElementString_ShortForm) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("root");
    w->WriteElementString("name", "Alice");
    w->WriteEndElement();
    std::string out = w->ToString();
    EXPECT_NE(out.find("<name>Alice</name>"), std::string::npos);
}

TEST(XmlWriterTests, WriteStartDocument_AddsDeclaration) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartDocument();
    w->WriteStartElement("root");
    w->WriteEndElement();
    std::string out = w->ToString();
    EXPECT_NE(out.find("<?xml"), std::string::npos);
}

TEST(XmlWriterTests, WriteComment_InOutput) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("root");
    w->WriteComment("a comment");
    w->WriteEndElement();
    EXPECT_NE(w->ToString().find("a comment"), std::string::npos);
}

TEST(XmlWriterTests, NestedElements_InOutput) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("root");
    w->WriteStartElement("child");
    w->WriteEndElement();
    w->WriteEndElement();
    std::string out = w->ToString();
    EXPECT_NE(out.find("<child"), std::string::npos);
    EXPECT_NE(out.find("</root>"), std::string::npos);
}

TEST(XmlWriterTests, Flush_DoesNotThrow) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    EXPECT_NO_THROW(w->Flush());
}

TEST(XmlWriterTests, Close_DoesNotThrow) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("root");
    w->WriteEndElement();
    EXPECT_NO_THROW(w->Close());
}

// Regression tests for audit finding A-02 (2026-07-14): XmlWriter::~XmlWriter() used to call
// Close() directly with no exception handling; Close() calls Flush(), which throws XmlException
// when tinyxml2's SaveFile() fails (e.g. a path under a nonexistent directory). Since destructors
// are implicitly noexcept, that used to call std::terminate instead of unwinding normally.
TEST(XmlWriterTests, Close_SaveFileFails_ThrowsXmlException) {
    std::unique_ptr<XmlWriter> w(XmlWriter::Create("/nonexistent-dir-for-a02-test/out.xml"));
    w->WriteStartElement("root");
    w->WriteEndElement();
    EXPECT_THROW(w->Close(), XmlException);
}

TEST(XmlWriterTests, Destructor_SaveFileFails_DoesNotTerminateProcess) {
    // The destructor swallows the SaveFile failure (best-effort cleanup); reaching this line at
    // all -- rather than the process being killed by std::terminate -- is the actual assertion.
    {
        std::unique_ptr<XmlWriter> w(XmlWriter::Create("/nonexistent-dir-for-a02-test/out.xml"));
        w->WriteStartElement("root");
        w->WriteEndElement();
    }
    SUCCEED();
}

// Regression test for a wave-3 audit finding: ToString() always pretty-printed via
// tinyxml2's XMLPrinter default (compact=false), ignoring XmlWriterSettings::Indent, whose
// real .NET default is false (compact, no inserted whitespace). Matches
// XmlWriterSettings.cs's documented default and XmlTextEncoder's un-indented output.
TEST(XmlWriterTests, DefaultSettings_ProducesCompactOutput_NoIndentWhitespace) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("root");
    w->WriteStartElement("child");
    w->WriteEndElement();
    w->WriteEndElement();
    std::string out = w->ToString();
    EXPECT_EQ(out, "<root><child/></root>");
}

// Regression tests for a wave-3 audit finding: WriteComment/WriteProcessingInstruction/
// WriteCData wrote text completely raw with no protection against embedded terminator
// sequences, silently producing malformed/corrupted XML. Verified against
// XmlEncodedRawTextWriter.WriteCommentOrPi/WriteCDataSection: real .NET self-heals rather
// than throwing (inserts a protective space, or splits the CDATA section), and the
// original content round-trips unchanged.
TEST(XmlWriterTests, WriteComment_EmbeddedDoubleDash_InsertsProtectiveSpace) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteComment("a--b");
    std::string out = w->ToString();
    EXPECT_EQ(out, "<!--a- -b-->");
}

TEST(XmlWriterTests, WriteComment_TrailingDash_InsertsProtectiveSpace) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteComment("a-");
    std::string out = w->ToString();
    EXPECT_EQ(out, "<!--a- -->");
}

TEST(XmlWriterTests, WriteProcessingInstruction_EmbeddedQuestionGreaterThan_InsertsProtectiveSpace) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteProcessingInstruction("pi", "a?>b");
    std::string out = w->ToString();
    EXPECT_EQ(out, "<?pi a? >b?>");
}

TEST(XmlWriterTests, WriteCData_EmbeddedCloseSequence_SplitsSection_RoundTrips) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("msg");
    w->WriteCData("a]]>b");
    w->WriteEndElement();
    std::string out = w->ToString();
    EXPECT_EQ(out.find("]]>b]]>"), std::string::npos) << out; // never a raw, premature "]]>"

    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(out));
    r->Read(); // <msg>
    std::string content = r->ReadElementContentAsString();
    EXPECT_EQ(content, "a]]>b");
}

TEST(XmlWriterTests, IndentSettingTrue_ProducesIndentedOutput) {
    XmlWriterSettings settings;
    settings.Indent = true;
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString(settings));
    w->WriteStartElement("root");
    w->WriteStartElement("child");
    w->WriteEndElement();
    w->WriteEndElement();
    std::string out = w->ToString();
    EXPECT_NE(out.find('\n'), std::string::npos);
}

// ===========================================================================
// Round-trip: write then read back
// ===========================================================================

TEST(XmlRoundTripTests, WriteAndReadBack) {
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("data");
    w->WriteAttributeString("version", "2");
    w->WriteElementString("item", "value1");
    w->WriteEndElement();
    std::string xml = w->ToString();

    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(xml));
    r->Read(); // <data>
    EXPECT_EQ(r->getNameProperty(), "data");
    EXPECT_EQ(r->GetAttribute("version"), "2");
    r->Read(); // <item>
    EXPECT_EQ(r->getNameProperty(), "item");
    std::string text = r->ReadElementContentAsString();
    EXPECT_EQ(text, "value1");
}

// ===========================================================================
// XName
// ===========================================================================

TEST(XNameTests, DefaultConstructor_EmptyNames) {
    XName n;
    EXPECT_EQ(n.getLocalNameProperty(), "");
    EXPECT_EQ(n.getNamespaceNameProperty(), "");
}

TEST(XNameTests, LocalNameConstructor) {
    XName n("item");
    EXPECT_EQ(n.getLocalNameProperty(), "item");
    EXPECT_EQ(n.getNamespaceNameProperty(), "");
}

TEST(XNameTests, FullConstructor_StoresBoth) {
    XName n("http://example.com", "item");
    EXPECT_EQ(n.getNamespaceNameProperty(), "http://example.com");
    EXPECT_EQ(n.getLocalNameProperty(), "item");
}

TEST(XNameTests, ToString_LocalOnly) {
    XName n("item");
    EXPECT_EQ(n.ToString(), "item");
}

TEST(XNameTests, ToString_WithNamespace) {
    XName n("http://ex.com", "item");
    EXPECT_EQ(n.ToString(), "{http://ex.com}item");
}

TEST(XNameTests, Equality_SameLocalName) {
    EXPECT_TRUE(XName("a") == XName("a"));
}

TEST(XNameTests, Equality_DifferentNames) {
    EXPECT_FALSE(XName("a") == XName("b"));
}

TEST(XNameTests, Inequality) {
    EXPECT_TRUE(XName("a") != XName("b"));
}

TEST(XNameTests, Get_LocalOnly) {
    XName n = XName::Get("root");
    EXPECT_EQ(n.getLocalNameProperty(), "root");
    EXPECT_EQ(n.getNamespaceNameProperty(), "");
}

TEST(XNameTests, Get_WithNamespace) {
    XName n = XName::Get("{http://ex.com}root");
    EXPECT_EQ(n.getNamespaceNameProperty(), "http://ex.com");
    EXPECT_EQ(n.getLocalNameProperty(), "root");
}

// Regression tests for a wave-3 audit finding: Get() split on the *first* '}' instead of
// the last, and performed no validation of malformed expanded-name syntax. Verified against
// XName.cs's Get(string): a namespace URI may itself legally contain '}', so the split must
// use LastIndexOf; an empty namespace ("{}x") or missing local name ("{ns}") must throw
// ArgumentException.
TEST(XNameTests, Get_NamespaceContainingCloseBrace_SplitsOnLastBrace) {
    XName n = XName::Get("{urn:example:{weird}}root");
    EXPECT_EQ(n.getNamespaceNameProperty(), "urn:example:{weird}");
    EXPECT_EQ(n.getLocalNameProperty(), "root");
}

TEST(XNameTests, Get_EmptyString_Throws) {
    EXPECT_THROW(XName::Get(""), System::ArgumentException);
}

TEST(XNameTests, Get_EmptyNamespace_Throws) {
    EXPECT_THROW(XName::Get("{}x"), System::ArgumentException);
}

TEST(XNameTests, Get_NoLocalNameAfterBrace_Throws) {
    EXPECT_THROW(XName::Get("{ns}"), System::ArgumentException);
}

// ===========================================================================
// XAttribute
// ===========================================================================

TEST(XAttributeTests, Constructor_StringName) {
    XAttribute a("id", "42");
    EXPECT_EQ(a.getNameProperty().getLocalNameProperty(), "id");
    EXPECT_EQ(a.getValueProperty(), "42");
}

TEST(XAttributeTests, Constructor_XName) {
    XAttribute a(XName("class"), "main");
    EXPECT_EQ(a.getValueProperty(), "main");
}

TEST(XAttributeTests, SetValue) {
    XAttribute a("x", "old");
    a.setValueProperty("new");
    EXPECT_EQ(a.getValueProperty(), "new");
}

TEST(XAttributeTests, ToString_LocalName) {
    XAttribute a("href", "url");
    EXPECT_EQ(a.ToString(), "href=\"url\"");
}

// Regression test for a wave-3 audit finding: EscapeValue() left tab/LF/CR unescaped, so
// per the XML spec's attribute-value normalization (section 3.3.3) a literal tab/LF/CR
// written into an attribute is collapsed to a plain space on reload -- the original value
// silently doesn't round-trip. Verified against XmlEncodedRawTextWriter's Tab/LineFeed/
// CarriageReturnEntity, which escape as character references instead.
TEST(XAttributeTests, ToString_TabNewlineCarriageReturn_EscapedAsCharacterReferences) {
    XAttribute a("x", "a\tb\nc\rd");
    std::string s = a.ToString();
    EXPECT_EQ(s, "x=\"a&#x9;b&#xA;c&#xD;d\"");
}

TEST(XAttributeTests, TabNewlineCarriageReturn_RoundTripsThroughReader) {
    XAttribute a("x", "a\tb\nc\rd");
    std::string xml = "<root " + a.ToString() + "/>";
    std::unique_ptr<XmlReader> r(XmlReader::CreateFromString(xml));
    r->Read();
    EXPECT_EQ(r->GetAttribute("x"), "a\tb\nc\rd");
}

TEST(XAttributeTests, ToString_NamespacedAttribute_DoesNotEmitClarkNotation) {
    // Regression: XAttribute::ToString() previously wrote XName::ToString()'s Clark notation
    // ("{namespace}local") directly as the attribute *name* -- '{'/'}' are not legal in an XML
    // Name production, so this produced literally malformed, unparseable XML for any
    // namespaced attribute instead of just a namespace-fidelity gap.
    XAttribute a(XName("http://example.com/ns", "kind"), "rare");
    std::string s = a.ToString();
    EXPECT_EQ(s.find('{'), std::string::npos);
    EXPECT_EQ(s.find('}'), std::string::npos);
    EXPECT_EQ(s, "kind=\"rare\"");
}

TEST(XAttributeTests, NextAttribute_DefaultNull) {
    XAttribute a("x", "1");
    EXPECT_EQ(a.getNextAttributeProperty(), nullptr);
}

// Regression tests for a wave-3 audit finding: XAttribute performed zero namespace-declaration
// validation. Verified against XAttribute.cs's ValidateAttribute.
TEST(XAttributeTests, IsNamespaceDeclaration_DefaultXmlns_IsTrue) {
    XAttribute a(XName("", "xmlns"), "http://example.com");
    EXPECT_TRUE(a.getIsNamespaceDeclarationProperty());
}

TEST(XAttributeTests, IsNamespaceDeclaration_PrefixedXmlns_IsTrue) {
    XAttribute a(XNamespace::Xmlns + "p", "http://example.com");
    EXPECT_TRUE(a.getIsNamespaceDeclarationProperty());
}

TEST(XAttributeTests, IsNamespaceDeclaration_OrdinaryAttribute_IsFalse) {
    XAttribute a("id", "42");
    EXPECT_FALSE(a.getIsNamespaceDeclarationProperty());
}

TEST(XAttributeTests, PrefixedXmlnsDeclaration_EmptyUri_Throws) {
    EXPECT_THROW((XAttribute(XNamespace::Xmlns + "p", "")), System::ArgumentException);
}

TEST(XAttributeTests, PrefixedXmlnsDeclaration_XmlnsUri_Throws) {
    EXPECT_THROW((XAttribute(XNamespace::Xmlns + "p", "http://www.w3.org/2000/xmlns/")),
                 System::ArgumentException);
}

TEST(XAttributeTests, PrefixedXmlnsDeclaration_XmlUriWithWrongPrefix_Throws) {
    EXPECT_THROW((XAttribute(XNamespace::Xmlns + "p", "http://www.w3.org/XML/1998/namespace")),
                 System::ArgumentException);
}

TEST(XAttributeTests, XmlnsXmlDeclaration_CorrectUri_DoesNotThrow) {
    EXPECT_NO_THROW((XAttribute(XNamespace::Xmlns + "xml", "http://www.w3.org/XML/1998/namespace")));
}

TEST(XAttributeTests, XmlnsXmlDeclaration_WrongUri_Throws) {
    EXPECT_THROW((XAttribute(XNamespace::Xmlns + "xml", "http://example.com")), System::ArgumentException);
}

TEST(XAttributeTests, XmlnsXmlnsDeclaration_Throws) {
    EXPECT_THROW((XAttribute(XNamespace::Xmlns + "xmlns", "http://example.com")), System::ArgumentException);
}

TEST(XAttributeTests, DefaultXmlnsDeclaration_XmlUri_Throws) {
    EXPECT_THROW((XAttribute(XName("", "xmlns"), "http://www.w3.org/XML/1998/namespace")),
                 System::ArgumentException);
}

TEST(XAttributeTests, DefaultXmlnsDeclaration_XmlnsUri_Throws) {
    EXPECT_THROW((XAttribute(XName("", "xmlns"), "http://www.w3.org/2000/xmlns/")),
                 System::ArgumentException);
}

TEST(XAttributeTests, SetValueProperty_ValidatesNamespaceDeclaration) {
    XAttribute a(XNamespace::Xmlns + "p", "http://example.com");
    EXPECT_THROW(a.setValueProperty(""), System::ArgumentException);
}

// ===========================================================================
// XElement
// ===========================================================================

TEST(XElementTests, Constructor_StringName) {
    XElement e("item");
    EXPECT_EQ(e.getNameProperty().getLocalNameProperty(), "item");
}

TEST(XElementTests, Constructor_NameAndValue) {
    XElement e(XName("price"), "9.99");
    EXPECT_EQ(e.getValueProperty(), "9.99");
}

TEST(XElementTests, SetValue) {
    XElement e("x");
    e.setValueProperty("hello");
    EXPECT_EQ(e.getValueProperty(), "hello");
}

// Regression test for a wave-3 audit finding: Add(const std::string&) always created a new
// XText child, even when the last child was already a plain (non-CDATA) XText node. Verified
// against XContainer.cs's AddString(): consecutive string adds merge into the existing
// trailing text node's Value instead of producing adjacent sibling text nodes.
TEST(XElementTests, Add_String_MergesIntoTrailingTextNode) {
    XElement e("root");
    e.Add(std::string("hello "));
    e.Add(std::string("world"));
    EXPECT_EQ(e.Nodes().size(), 1u);
    EXPECT_EQ(e.getValueProperty(), "hello world");
}

TEST(XElementTests, Add_String_DoesNotMergeAcrossCData) {
    XElement e("root");
    e.Add(std::make_shared<XCData>("cdata"));
    e.Add(std::string("text"));
    EXPECT_EQ(e.Nodes().size(), 2u);
}

TEST(XElementTests, Add_EmptyString_IsNoOp) {
    XElement e("root");
    e.Add(std::string(""));
    EXPECT_EQ(e.Nodes().size(), 0u);
}

TEST(XElementTests, AddAndFindAttribute) {
    XElement e("div");
    e.Add(std::make_shared<XAttribute>("class", "box"));
    auto attr = e.Attribute("class");
    ASSERT_NE(attr, nullptr);
    EXPECT_EQ(attr->getValueProperty(), "box");
}

TEST(XElementTests, Attribute_NotFound_ReturnsNull) {
    XElement e("div");
    EXPECT_EQ(e.Attribute("missing"), nullptr);
}

TEST(XElementTests, AddAndFindChildElement) {
    XElement root("root");
    root.Add(std::make_shared<XElement>("child"));
    auto child = root.Element("child");
    ASSERT_NE(child, nullptr);
    EXPECT_EQ(child->getNameProperty().getLocalNameProperty(), "child");
}

TEST(XElementTests, Element_NotFound_ReturnsNull) {
    XElement e("root");
    EXPECT_EQ(e.Element("missing"), nullptr);
}

TEST(XElementTests, Elements_ReturnsAllChildren) {
    XElement root("root");
    root.Add(std::make_shared<XElement>("a"));
    root.Add(std::make_shared<XElement>("b"));
    EXPECT_EQ(root.Elements().size(), 2u);
}

TEST(XElementTests, Elements_ByName_FiltersCorrectly) {
    XElement root("root");
    root.Add(std::make_shared<XElement>("a"));
    root.Add(std::make_shared<XElement>("b"));
    root.Add(std::make_shared<XElement>("a"));
    EXPECT_EQ(root.Elements("a").size(), 2u);
}

TEST(XElementTests, Descendants_FindsNested) {
    XElement root("root");
    auto child = std::make_shared<XElement>("child");
    auto nested = std::make_shared<XElement>("target");
    child->Add(nested);
    root.Add(child);
    auto found = root.Descendants(XName("target"));
    EXPECT_EQ(found.size(), 1u);
}

TEST(XElementTests, GetAttributeValue_Present) {
    XElement e("el");
    e.Add(std::make_shared<XAttribute>("key", "val"));
    auto v = e.getAttributeValue("key");
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(*v, "val");
}

TEST(XElementTests, GetAttributeValue_Absent_NullOpt) {
    XElement e("el");
    EXPECT_FALSE(e.getAttributeValue("missing").has_value());
}

TEST(XElementTests, ToString_SelfClosing) {
    XElement e("br");
    EXPECT_EQ(e.ToString(), "<br/>");
}

TEST(XElementTests, ToString_WithValue) {
    XElement e(XName("name"), "Alice");
    EXPECT_EQ(e.ToString(), "<name>Alice</name>");
}

TEST(XElementTests, ToString_WithAttribute) {
    XElement e("img");
    e.Add(std::make_shared<XAttribute>("src", "pic.png"));
    std::string s = e.ToString();
    EXPECT_NE(s.find("src=\"pic.png\""), std::string::npos);
}

TEST(XElementTests, ToString_NamespacedAttribute_ProducesValidXml_RoundTrips) {
    // Regression: previously ToString() (via SerializeTo -> XAttribute::ToString()) emitted
    // "{http://example.com/ns}kind=\"rare\"", which is not valid XML and would fail to
    // reparse. Confirms the fixed output both omits Clark notation and successfully
    // round-trips through Parse().
    XElement e("special");
    e.Add(std::make_shared<XAttribute>(XName("http://example.com/ns", "kind"), "rare"));
    std::string s = e.ToString();
    EXPECT_EQ(s.find('{'), std::string::npos);

    auto reloaded = XElement::Parse(s);
    ASSERT_NE(reloaded, nullptr);
    EXPECT_EQ(reloaded->Attribute("kind")->getValueProperty(), "rare");
}

// ===========================================================================
// XDeclaration + XDocument
// ===========================================================================

TEST(XDeclarationTests, Constructor_StoresFields) {
    XDeclaration decl("1.0", "utf-8", "yes");
    EXPECT_EQ(decl.getVersionProperty(), "1.0");
    EXPECT_EQ(decl.getEncodingProperty(), "utf-8");
    EXPECT_EQ(decl.getStandaloneProperty(), "yes");
}

TEST(XDeclarationTests, ToString_ContainsVersion) {
    XDeclaration decl("1.0", "utf-8", "no");
    EXPECT_NE(decl.ToString().find("1.0"), std::string::npos);
}

TEST(XDocumentTests, DefaultConstructor_NullRoot) {
    XDocument doc;
    EXPECT_EQ(doc.getRootProperty(), nullptr);
}

TEST(XDocumentTests, Constructor_WithRoot) {
    auto root = std::make_shared<XElement>("root");
    XDocument doc(root);
    ASSERT_NE(doc.getRootProperty(), nullptr);
    EXPECT_EQ(doc.getRootProperty()->getNameProperty().getLocalNameProperty(), "root");
}

TEST(XDocumentTests, SetRoot) {
    XDocument doc;
    doc.setRootProperty(std::make_shared<XElement>("x"));
    ASSERT_NE(doc.getRootProperty(), nullptr);
}

TEST(XDocumentTests, SetDeclaration) {
    XDocument doc;
    doc.setDeclarationProperty(std::make_shared<XDeclaration>("1.0", "utf-8", "yes"));
    ASSERT_NE(doc.getDeclarationProperty(), nullptr);
    EXPECT_EQ(doc.getDeclarationProperty()->getVersionProperty(), "1.0");
}

TEST(XDocumentTests, Element_MatchesRootByName) {
    auto root = std::make_shared<XElement>("catalog");
    XDocument doc(root);
    EXPECT_NE(doc.Element("catalog"), nullptr);
    EXPECT_EQ(doc.Element("other"), nullptr);
}

TEST(XDocumentTests, ToString_ContainsRootTag) {
    auto root = std::make_shared<XElement>("data");
    XDocument doc(root);
    EXPECT_NE(doc.ToString().find("<data"), std::string::npos);
}

TEST(XDocumentTests, Parse_ReturnsNonNull) {
    auto doc = XDocument::Parse("<root/>");
    EXPECT_NE(doc, nullptr);
}

TEST(XDocumentTests, Parse_ParsesRealContent) {
    auto doc = XDocument::Parse("<catalog><item id=\"1\">Widget</item></catalog>");
    ASSERT_NE(doc->getRootProperty(), nullptr);
    EXPECT_EQ(doc->getRootProperty()->getNameProperty().getLocalNameProperty(), "catalog");
    auto item = doc->getRootProperty()->Element("item");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->getAttributeValue("id"), "1");
    EXPECT_EQ(item->getValueProperty(), "Widget");
}

TEST(XDocumentTests, Parse_MalformedXml_Throws) {
    EXPECT_THROW(XDocument::Parse("<unclosed>"), XmlException);
}

TEST(XDocumentTests, Load_MissingFile_Throws) {
    EXPECT_THROW(XDocument::Load("nonexistent.xml"), XmlException);
}

TEST(XDocumentTests, Load_ExistingFile_ReturnsParsedDocument) {
    const char* path = "xdocument_load_test.xml";
    {
        std::ofstream ofs(path);
        ofs << "<root><child>value</child></root>";
    }
    auto doc = XDocument::Load(path);
    ASSERT_NE(doc->getRootProperty(), nullptr);
    EXPECT_EQ(doc->getRootProperty()->getNameProperty().getLocalNameProperty(), "root");
    std::remove(path);
}
