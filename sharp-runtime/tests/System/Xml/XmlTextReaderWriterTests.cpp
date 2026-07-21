// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include "System/Xml/NameTable.hpp"
#include "System/Xml/XmlNamespaceManager.hpp"
#include "System/Xml/XmlParserContext.hpp"
#include "System/Xml/XmlTextReader.hpp"
#include "System/Xml/XmlTextWriter.hpp"
#include "System/Xml/XmlValidatingReader.hpp"

using namespace System::Xml;

namespace {
    // XmlTextReader's constructor takes a URL/file path (matching .NET's XmlTextReader(string
    // url), which never accepts raw XML text), so tests write to a temp file first.
    class TempXmlFile {
    public:
        explicit TempXmlFile(const std::string& xml) : path_("/tmp/sharp_rt_xmltextreader_test.xml") {
            std::ofstream out(path_);
            out << xml;
        }
        ~TempXmlFile() { std::remove(path_.c_str()); }
        [[nodiscard]] const std::string& path() const { return path_; }
    private:
        std::string path_;
    };
}

TEST(XmlTextReaderTests, ReadsElementsAndAttributes) {
    TempXmlFile file("<root attr=\"v\"><child>text</child></root>");
    XmlTextReader reader(file.path());
    ASSERT_TRUE(reader.Read());
    EXPECT_EQ(reader.getNodeTypeProperty(), XmlNodeType::Element);
    EXPECT_EQ(reader.getNameProperty(), "root");
    EXPECT_EQ(reader.GetAttribute("attr"), "v");
}

TEST(XmlTextReaderTests, NamespacesAndWhitespaceHandling_AreSettableProperties) {
    TempXmlFile file("<root/>");
    XmlTextReader reader(file.path());
    reader.setNamespacesProperty(false);
    EXPECT_FALSE(reader.getNamespacesProperty());
    reader.setWhitespaceHandlingProperty(WhitespaceHandling::None);
    EXPECT_EQ(reader.getWhitespaceHandlingProperty(), WhitespaceHandling::None);
}

TEST(XmlTextWriterTests, WritesElementAndAttributeToString) {
    XmlTextWriter writer;
    writer.WriteStartElement("root");
    writer.WriteAttributeString("attr", "v");
    writer.WriteString("text");
    writer.WriteEndElement();
    std::string result = writer.ToString();
    EXPECT_NE(result.find("<root"), std::string::npos);
    EXPECT_NE(result.find("attr=\"v\""), std::string::npos);
    EXPECT_NE(result.find("text"), std::string::npos);
}

TEST(XmlTextWriterTests, FormattingProperty_IsSettable) {
    XmlTextWriter writer;
    writer.setFormattingProperty(Formatting::Indented);
    EXPECT_EQ(writer.getFormattingProperty(), Formatting::Indented);
}

TEST(XmlValidatingReaderTests, DelegatesReadToInnerReader) {
    std::unique_ptr<XmlReader> inner(XmlReader::CreateFromString("<root attr=\"v\"/>"));
    XmlValidatingReader validating(inner.get());
    ASSERT_TRUE(validating.Read());
    EXPECT_EQ(validating.getNameProperty(), "root");
    EXPECT_EQ(validating.GetAttribute("attr"), "v");
}

TEST(XmlValidatingReaderTests, ValidationTypeProperty_IsSettable) {
    std::unique_ptr<XmlReader> inner(XmlReader::CreateFromString("<root/>"));
    XmlValidatingReader validating(inner.get());
    validating.setValidationTypeProperty(ValidationType::Schema);
    EXPECT_EQ(validating.getValidationTypeProperty(), ValidationType::Schema);
}

TEST(XmlParserContextTests, StoresAllConstructorArguments) {
    auto nt = std::make_shared<NameTable>();
    auto nsMgr = std::make_shared<XmlNamespaceManager>(nt);
    XmlParserContext ctx(nt, nsMgr, "root", "pub", "sys", "subset", "base://uri", "en", XmlSpace::Preserve, "utf-8");
    EXPECT_EQ(ctx.getDocTypeNameProperty(), "root");
    EXPECT_EQ(ctx.getPublicIdProperty(), "pub");
    EXPECT_EQ(ctx.getSystemIdProperty(), "sys");
    EXPECT_EQ(ctx.getInternalSubsetProperty(), "subset");
    EXPECT_EQ(ctx.getBaseURIProperty(), "base://uri");
    EXPECT_EQ(ctx.getXmlLangProperty(), "en");
    EXPECT_EQ(ctx.getXmlSpaceProperty(), XmlSpace::Preserve);
    EXPECT_EQ(ctx.getEncodingProperty(), "utf-8");
}
