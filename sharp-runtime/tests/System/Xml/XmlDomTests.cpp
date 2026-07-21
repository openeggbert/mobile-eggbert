// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Coverage for the classic XmlDocument DOM API (XmlNode/XmlDocument/XmlElement/XmlAttribute/etc.).
#include <gtest/gtest.h>
#include <cstdio>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlEntity.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlImplementation.hpp"
#include "System/Xml/XmlWriter.hpp"

using namespace System::Xml;

// ===========================================================================
// Basic construction and CreateElement/AppendChild
// ===========================================================================

TEST(XmlDocumentTests, CreateElement_AppendAsRoot_BecomesDocumentElement) {
    XmlDocument doc;
    auto* root = doc.CreateElement("catalog");
    doc.AppendChild(root);
    ASSERT_NE(doc.getDocumentElementProperty(), nullptr);
    EXPECT_EQ(doc.getDocumentElementProperty()->getNameProperty(), "catalog");
}

TEST(XmlDocumentTests, AppendChild_NestedElements_NavigationWorks) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* child = doc.CreateElement("child");
    root->AppendChild(child);

    EXPECT_EQ(root->getFirstChildProperty(), child);
    EXPECT_EQ(root->getLastChildProperty(), child);
    EXPECT_EQ(child->getParentNodeProperty(), root);
    EXPECT_EQ(child->getParentNodeProperty()->getNameProperty(), "root");
}

TEST(XmlDocumentTests, NodeIdentity_RepeatedNavigation_ReturnsSamePointer) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* child = doc.CreateElement("child");
    root->AppendChild(child);

    EXPECT_EQ(root->getFirstChildProperty(), root->getFirstChildProperty());
    EXPECT_EQ(child->getParentNodeProperty(), child->getParentNodeProperty());
}

TEST(XmlDocumentTests, SiblingNavigation) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* a = doc.CreateElement("a");
    auto* b = doc.CreateElement("b");
    root->AppendChild(a);
    root->AppendChild(b);

    EXPECT_EQ(a->getNextSiblingProperty(), b);
    EXPECT_EQ(b->getPreviousSiblingProperty(), a);
    EXPECT_EQ(a->getPreviousSiblingProperty(), nullptr);
    EXPECT_EQ(b->getNextSiblingProperty(), nullptr);
}

// ===========================================================================
// Attributes
// ===========================================================================

TEST(XmlElementTests, SetAttribute_ThenGetAttribute_RoundTrips) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    el->SetAttribute("isbn", "12345");
    EXPECT_EQ(el->GetAttribute("isbn"), "12345");
    EXPECT_TRUE(el->HasAttribute("isbn"));
}

TEST(XmlElementTests, GetAttribute_Missing_ReturnsEmptyString) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    EXPECT_EQ(el->GetAttribute("missing"), "");
    EXPECT_FALSE(el->HasAttribute("missing"));
}

TEST(XmlElementTests, RemoveAttribute_RemovesIt) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    el->SetAttribute("isbn", "12345");
    el->RemoveAttribute("isbn");
    EXPECT_FALSE(el->HasAttribute("isbn"));
}

TEST(XmlElementTests, GetAttributeNode_IdentityStable) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    el->SetAttribute("isbn", "12345");
    auto* a1 = el->GetAttributeNode("isbn");
    auto* a2 = el->GetAttributeNode("isbn");
    EXPECT_EQ(a1, a2);
    EXPECT_EQ(a1->getValueProperty(), "12345");
}

TEST(XmlElementTests, Attributes_CollectionReflectsCurrentAttributes) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    el->SetAttribute("isbn", "12345");
    el->SetAttribute("title", "Foo");
    auto* attrs = el->getAttributesProperty();
    EXPECT_EQ(attrs->getCountProperty(), 2);
    EXPECT_NE((*attrs)["isbn"], nullptr);
    EXPECT_EQ((*attrs)["isbn"]->getValueProperty(), "12345");
}

TEST(XmlAttributeTests, SetValue_ThroughAttachedAttribute_UpdatesElement) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    el->SetAttribute("isbn", "12345");
    auto* attr = el->GetAttributeNode("isbn");
    attr->setValueProperty("99999");
    EXPECT_EQ(el->GetAttribute("isbn"), "99999");
}

TEST(XmlAttributeTests, CreateAttribute_Unattached_HoldsLocalValue) {
    XmlDocument doc;
    auto* attr = doc.CreateAttribute("standalone");
    attr->setValueProperty("hello");
    EXPECT_EQ(attr->getValueProperty(), "hello");
    EXPECT_EQ(attr->getOwnerElementProperty(), nullptr);
}

TEST(XmlAttributeTests, SetAttributeNode_AttachesUnattachedAttribute) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    auto* attr = doc.CreateAttribute("isbn");
    attr->setValueProperty("12345");
    el->SetAttributeNode(attr);
    EXPECT_EQ(el->GetAttribute("isbn"), "12345");
    EXPECT_EQ(attr->getOwnerElementProperty(), el);
}

TEST(XmlAttributeTests, NamespaceURI_PrefixedAttribute_ResolvesFromAncestorXmlnsDeclaration) {
    XmlDocument doc;
    doc.LoadXml("<root xmlns:ns=\"urn:test\"><child ns:attr=\"v\"/></root>");
    auto* root = doc.getDocumentElementProperty();
    auto* child = static_cast<XmlElement*>(root->getFirstChildProperty());
    auto* attr = child->GetAttributeNode("ns:attr");
    ASSERT_NE(attr, nullptr);
    EXPECT_EQ(attr->getNamespaceURIProperty(), "urn:test");
}

TEST(XmlAttributeTests, NamespaceURI_UnprefixedAttribute_IsEmpty) {
    // XML Namespaces spec: an element's default `xmlns="..."` declaration does not apply to
    // its own unprefixed attributes.
    XmlDocument doc;
    doc.LoadXml("<root xmlns=\"urn:default\" plain=\"v\"/>");
    auto* root = doc.getDocumentElementProperty();
    auto* attr = root->GetAttributeNode("plain");
    ASSERT_NE(attr, nullptr);
    EXPECT_EQ(attr->getNamespaceURIProperty(), "");
}

TEST(XmlAttributeTests, GetAttributesCollection_GetNamedItemByLocalNameAndNamespace_FindsPrefixedAttribute) {
    // Regression: XmlNamedNodeMap::GetNamedItem(localName, namespaceURI) dispatches virtually
    // through XmlAttribute::getNamespaceURIProperty() -- previously always "", so this lookup
    // could never match a prefixed attribute regardless of namespace.
    XmlDocument doc;
    doc.LoadXml("<root xmlns:ns=\"urn:test\"><child ns:attr=\"v\"/></root>");
    auto* root = doc.getDocumentElementProperty();
    auto* child = static_cast<XmlElement*>(root->getFirstChildProperty());
    auto* found = child->getAttributesProperty()->GetNamedItem("attr", "urn:test");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getValueProperty(), "v");
}

TEST(XmlAttributeTests, CloneNode_CopiesNameAndValue) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    el->SetAttribute("isbn", "12345");
    auto* attr = el->GetAttributeNode("isbn");
    auto* clone = attr->CloneNode(false);
    ASSERT_NE(clone, nullptr);
    auto* clonedAttr = static_cast<XmlAttribute*>(clone);
    EXPECT_EQ(clonedAttr->getNameProperty(), "isbn");
    EXPECT_EQ(clonedAttr->getValueProperty(), "12345");
    EXPECT_EQ(clonedAttr->getOwnerElementProperty(), nullptr);
}

// ===========================================================================
// InnerText / InnerXml / OuterXml
// ===========================================================================

TEST(XmlElementTests, SetInnerText_CreatesTextChild) {
    XmlDocument doc;
    auto* el = doc.CreateElement("title");
    el->setInnerTextProperty("Hello World");
    EXPECT_EQ(el->getInnerTextProperty(), "Hello World");
}

TEST(XmlElementTests, InnerText_ConcatenatesDescendantText) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    auto* child = doc.CreateElement("child");
    root->AppendChild(child);
    child->setInnerTextProperty("nested");
    EXPECT_EQ(root->getInnerTextProperty(), "nested");
}

TEST(XmlElementTests, OuterXml_ContainsTagAndAttributes) {
    XmlDocument doc;
    auto* el = doc.CreateElement("book");
    el->SetAttribute("isbn", "12345");
    el->setInnerTextProperty("Title");
    std::string xml = el->getOuterXmlProperty();
    EXPECT_NE(xml.find("<book"), std::string::npos);
    EXPECT_NE(xml.find("isbn=\"12345\""), std::string::npos);
    EXPECT_NE(xml.find("Title"), std::string::npos);
}

TEST(XmlElementTests, OuterXml_NestedElements_ProducesExactMarkupNoInsertedWhitespace) {
    // Regression: real .NET's OuterXml serializes exact markup with no inserted whitespace
    // between nodes. tinyxml2::XMLPrinter defaults to pretty-printing (inserted newlines and
    // indentation), which this port previously left unchanged.
    XmlDocument doc;
    doc.LoadXml("<root><a/><b>text</b></root>");
    auto* root = doc.getDocumentElementProperty();
    EXPECT_EQ(root->getOuterXmlProperty(), "<root><a/><b>text</b></root>");
}

TEST(XmlElementTests, InnerXml_NestedElements_ProducesExactMarkupNoInsertedWhitespace) {
    XmlDocument doc;
    doc.LoadXml("<root><a/><b>text</b></root>");
    auto* root = doc.getDocumentElementProperty();
    EXPECT_EQ(root->getInnerXmlProperty(), "<a/><b>text</b>");
}

TEST(XmlElementTests, SetInnerXml_ParsesAndAppendsChildren) {
    XmlDocument doc;
    auto* el = doc.CreateElement("root");
    el->setInnerXmlProperty("<a/><b>text</b>");
    auto* children = el->getChildNodesProperty();
    EXPECT_EQ(children->getCountProperty(), 2);
}

// ===========================================================================
// Tree mutation: RemoveChild reuse, ReplaceChild, CloneNode
// ===========================================================================

TEST(XmlNodeTests, RemoveChild_NodeRemainsReusable) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* child = doc.CreateElement("child");
    root->AppendChild(child);

    root->RemoveChild(child);
    EXPECT_EQ(root->getFirstChildProperty(), nullptr);
    EXPECT_EQ(child->getParentNodeProperty(), nullptr);

    // Reinsert elsewhere — must still be usable.
    auto* other = doc.CreateElement("other");
    doc.AppendChild(other); // documents can only have one root in practice; use as container test
    other->AppendChild(child);
    EXPECT_EQ(child->getParentNodeProperty(), other);
}

TEST(XmlNodeTests, ReplaceChild_ReturnsOldNode) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* oldChild = doc.CreateElement("old");
    auto* newChild = doc.CreateElement("new");
    root->AppendChild(oldChild);

    auto* returned = root->ReplaceChild(newChild, oldChild);
    EXPECT_EQ(returned, oldChild);
    EXPECT_EQ(root->getFirstChildProperty(), newChild);
}

TEST(XmlNodeTests, CloneNode_Deep_CopiesChildren) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    root->SetAttribute("a", "1");
    root->setInnerTextProperty("hi");

    auto* clone = static_cast<XmlElement*>(root->CloneNode(true));
    EXPECT_EQ(clone->GetAttribute("a"), "1");
    EXPECT_EQ(clone->getInnerTextProperty(), "hi");
    EXPECT_NE(clone, root);
}

TEST(XmlNodeTests, InsertBefore_NullRefChild_AppendsAtEnd) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    auto* a = doc.CreateElement("a");
    root->AppendChild(a);
    auto* b = doc.CreateElement("b");
    root->InsertBefore(b, nullptr);
    EXPECT_EQ(root->getLastChildProperty(), b);
}

TEST(XmlNodeTests, InsertBefore_WithRefChild_InsertsInOrder) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    auto* a = doc.CreateElement("a");
    auto* c = doc.CreateElement("c");
    root->AppendChild(a);
    root->AppendChild(c);
    auto* b = doc.CreateElement("b");
    root->InsertBefore(b, c);
    EXPECT_EQ(a->getNextSiblingProperty(), b);
    EXPECT_EQ(b->getNextSiblingProperty(), c);
}

// ===========================================================================
// Regression: AppendChild/PrependChild/InsertBefore/InsertAfter previously had no
// ancestor-cycle or cross-document guard (verified against XmlNode.cs's InsertBefore/
// InsertAfter/AppendChild, which throw ArgumentException for both cases) — inserting a
// node's own ancestor as its child created a genuine shared_ptr-free but still real
// tinyxml2 parent/child cycle (UB in traversal/destruction), and cross-document inserts
// silently no-oped while reporting success.
// ===========================================================================

TEST(XmlNodeTests, AppendChild_Self_ThrowsArgumentException) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    EXPECT_THROW(root->AppendChild(root), System::ArgumentException);
}

TEST(XmlNodeTests, AppendChild_Ancestor_ThrowsArgumentException) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* child = doc.CreateElement("child");
    root->AppendChild(child);
    auto* grandchild = doc.CreateElement("grandchild");
    child->AppendChild(grandchild);

    EXPECT_THROW(grandchild->AppendChild(root), System::ArgumentException);
    EXPECT_THROW(grandchild->AppendChild(child), System::ArgumentException);
    // Tree must be unmodified after the throw.
    EXPECT_EQ(grandchild->getFirstChildProperty(), nullptr);
    EXPECT_EQ(child->getParentNodeProperty(), root);
}

TEST(XmlNodeTests, PrependChild_Ancestor_ThrowsArgumentException) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* child = doc.CreateElement("child");
    root->AppendChild(child);
    EXPECT_THROW(child->PrependChild(root), System::ArgumentException);
}

TEST(XmlNodeTests, InsertBefore_Ancestor_ThrowsArgumentException) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* child = doc.CreateElement("child");
    root->AppendChild(child);
    auto* grandchild = doc.CreateElement("grandchild");
    child->AppendChild(grandchild);
    EXPECT_THROW(grandchild->InsertBefore(root, nullptr), System::ArgumentException);
}

TEST(XmlNodeTests, InsertAfter_Ancestor_ThrowsArgumentException) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* child = doc.CreateElement("child");
    root->AppendChild(child);
    auto* grandchild = doc.CreateElement("grandchild");
    child->AppendChild(grandchild);
    EXPECT_THROW(grandchild->InsertAfter(root, nullptr), System::ArgumentException);
}

TEST(XmlNodeTests, AppendChild_CrossDocument_ThrowsArgumentException) {
    XmlDocument doc1;
    auto* root1 = doc1.CreateElement("root1");
    doc1.AppendChild(root1);

    XmlDocument doc2;
    auto* root2 = doc2.CreateElement("root2");
    doc2.AppendChild(root2);
    auto* other = doc2.CreateElement("other");

    EXPECT_THROW(root1->AppendChild(other), System::ArgumentException);
}

// Regression test for a wave-3 audit finding: Normalize() only merged adjacent text nodes
// among direct children, never recursing into child elements -- verified against
// XmlNode.cs's Normalize(), which explicitly recurses (`case XmlNodeType.Element:
// crtChild.Normalize(); goto default;`) into the full depth of the sub-tree.
TEST(XmlNodeTests, Normalize_MergesAdjacentTextInDescendantElements) {
    XmlDocument doc;
    doc.LoadXml("<root><child>a</child></root>");
    auto* root = doc.getDocumentElementProperty();
    auto* child = static_cast<XmlElement*>(root->getFirstChildProperty());
    // Simulate two adjacent text nodes under the descendant element (as real .NET code
    // building a DOM by hand, e.g. repeated AppendChild(CreateTextNode(...)), would produce).
    child->AppendChild(doc.CreateTextNode("b"));

    root->Normalize();

    auto* mergedText = child->getFirstChildProperty();
    ASSERT_NE(mergedText, nullptr);
    EXPECT_EQ(mergedText->getValueProperty(), "ab");
    EXPECT_EQ(mergedText->getNextSiblingProperty(), nullptr);
}

// ===========================================================================
// Comment / Text / CDATA
// ===========================================================================

TEST(XmlCharacterDataTests, CreateComment_HasCorrectValueAndType) {
    XmlDocument doc;
    auto* c = doc.CreateComment("a comment");
    EXPECT_EQ(c->getNodeTypeProperty(), XmlNodeType::Comment);
    EXPECT_EQ(c->getValueProperty(), "a comment");
}

TEST(XmlCharacterDataTests, CreateCDataSection_HasCorrectType) {
    XmlDocument doc;
    auto* cdata = doc.CreateCDataSection("<raw>");
    EXPECT_EQ(cdata->getNodeTypeProperty(), XmlNodeType::CDATA);
    EXPECT_EQ(cdata->getDataProperty(), "<raw>");
}

TEST(XmlCharacterDataTests, CreateWhitespace_ValidWhitespace_DoesNotThrow) {
    XmlDocument doc;
    auto* ws = doc.CreateWhitespace(" \t\r\n ");
    EXPECT_EQ(ws->getNodeTypeProperty(), XmlNodeType::Whitespace);
    EXPECT_EQ(ws->getDataProperty(), " \t\r\n ");
}

TEST(XmlCharacterDataTests, CreateWhitespace_EmptyString_DoesNotThrow) {
    XmlDocument doc;
    EXPECT_NO_THROW(doc.CreateWhitespace(""));
}

// Regression test for a code-audit finding (ticket 304): CreateWhitespace never validated that
// its content is actually all whitespace, unlike real .NET's XmlWhitespace constructor (whose
// CheckOnData check throws ArgumentException for any non-whitespace character), so a node
// claiming to be Whitespace-typed could silently hold arbitrary text.
TEST(XmlCharacterDataTests, CreateWhitespace_NonWhitespaceContent_Throws) {
    XmlDocument doc;
    EXPECT_THROW(doc.CreateWhitespace("not whitespace!"), System::ArgumentException);
}

TEST(XmlCharacterDataTests, CreateSignificantWhitespace_ValidWhitespace_DoesNotThrow) {
    XmlDocument doc;
    auto* ws = doc.CreateSignificantWhitespace("  ");
    EXPECT_EQ(ws->getNodeTypeProperty(), XmlNodeType::SignificantWhitespace);
}

// Same bug as CreateWhitespace above -- CreateSignificantWhitespace shared the same missing
// validation (XmlSignificantWhiteSpace.cs's constructor has the identical CheckOnData check).
TEST(XmlCharacterDataTests, CreateSignificantWhitespace_NonWhitespaceContent_Throws) {
    XmlDocument doc;
    EXPECT_THROW(doc.CreateSignificantWhitespace("abc"), System::ArgumentException);
}

TEST(XmlCharacterDataTests, AppendData_AppendsToExistingText) {
    XmlDocument doc;
    auto* t = doc.CreateTextNode("Hello");
    t->AppendData(" World");
    EXPECT_EQ(t->getDataProperty(), "Hello World");
}

TEST(XmlCharacterDataTests, SplitText_SplitsIntoTwoSiblings) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* t = doc.CreateTextNode("HelloWorld");
    root->AppendChild(t);
    auto* tail = t->SplitText(5);
    EXPECT_EQ(t->getDataProperty(), "Hello");
    EXPECT_EQ(tail->getDataProperty(), "World");
    EXPECT_EQ(t->getNextSiblingProperty(), tail);
}

TEST(XmlCharacterDataTests, SplitText_OffsetGreaterThanLength_ThrowsArgumentOutOfRangeException) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* t = doc.CreateTextNode("Hello");
    root->AppendChild(t);
    EXPECT_THROW(t->SplitText(10), System::ArgumentOutOfRangeException);
}

TEST(XmlCharacterDataTests, SplitText_NoParent_ThrowsInvalidOperationException) {
    XmlDocument doc;
    auto* t = doc.CreateTextNode("Hello");
    EXPECT_THROW(t->SplitText(2), System::InvalidOperationException);
}

TEST(XmlCharacterDataTests, Substring_EmptyData_ReturnsEmptyRegardlessOfOffset) {
    XmlDocument doc;
    auto* t = doc.CreateTextNode("");
    EXPECT_EQ(t->Substring(5, 3), "");
    EXPECT_EQ(t->Substring(0, 0), "");
}

TEST(XmlCharacterDataTests, Substring_NonEmptyData_ClampsCountToLength) {
    XmlDocument doc;
    auto* t = doc.CreateTextNode("Hello");
    EXPECT_EQ(t->Substring(2, 100), "llo");
}

// ===========================================================================
// XmlDeclaration / XmlProcessingInstruction / XmlDocumentType
// ===========================================================================

TEST(XmlDeclarationTests, CreateXmlDeclaration_StoresVersionEncodingStandalone) {
    XmlDocument doc;
    auto* decl = doc.CreateXmlDeclaration("1.0", "UTF-8", "yes");
    EXPECT_EQ(decl->getVersionProperty(), "1.0");
    EXPECT_EQ(decl->getEncodingProperty(), "UTF-8");
    EXPECT_EQ(decl->getStandaloneProperty(), "yes");
}

TEST(XmlDeclarationTests, CreateXmlDeclaration_InvalidVersion_Throws) {
    XmlDocument doc;
    EXPECT_THROW(doc.CreateXmlDeclaration("9.9", "", ""), System::ArgumentException);
}

TEST(XmlDeclarationTests, CreateXmlDeclaration_InvalidStandalone_Throws) {
    XmlDocument doc;
    EXPECT_THROW(doc.CreateXmlDeclaration("1.0", "", "maybe"), System::ArgumentException);
}

TEST(XmlDeclarationTests, SetStandaloneProperty_InvalidValue_Throws) {
    XmlDocument doc;
    auto* decl = doc.CreateXmlDeclaration("1.0", "", "");
    EXPECT_THROW(decl->setStandaloneProperty("maybe"), System::ArgumentException);
}

TEST(XmlProcessingInstructionTests, CreateProcessingInstruction_SplitsTargetAndData) {
    XmlDocument doc;
    auto* pi = doc.CreateProcessingInstruction("xml-stylesheet", "type=\"text/xsl\" href=\"style.xsl\"");
    EXPECT_EQ(pi->getTargetProperty(), "xml-stylesheet");
    EXPECT_EQ(pi->getDataProperty(), "type=\"text/xsl\" href=\"style.xsl\"");
}

// Regression test for a wave-3 audit finding: CreateProcessingInstruction never checked
// that target was non-empty. Matches XmlProcessingInstruction.cs's constructor
// (ArgumentException.ThrowIfNullOrEmpty(target)).
TEST(XmlProcessingInstructionTests, CreateProcessingInstruction_EmptyTarget_Throws) {
    XmlDocument doc;
    EXPECT_THROW(doc.CreateProcessingInstruction("", "data"), System::ArgumentException);
}

// Regression tests for a wave-3 audit finding: CreateElement/CreateAttribute never validated
// that the given name is a well-formed XML name -- silently accepting e.g. "1bad" or "" and
// producing unparseable markup on write-out. Matches XmlDocument.cs's CheckName (called via
// CreateElement/CreateAttribute's XmlElement/XmlAttribute constructors), which throws
// XmlException for a malformed name.
TEST(XmlDocumentTests, CreateElement_InvalidName_Throws) {
    XmlDocument doc;
    EXPECT_THROW(doc.CreateElement("1bad"), System::Exception);
    EXPECT_THROW(doc.CreateElement(""), System::Exception);
    EXPECT_THROW(doc.CreateElement("bad name"), System::Exception);
}

TEST(XmlDocumentTests, CreateElement_ValidQualifiedName_DoesNotThrow) {
    XmlDocument doc;
    EXPECT_NO_THROW(doc.CreateElement("ns:tag"));
}

TEST(XmlAttributeTests, CreateAttribute_InvalidName_Throws) {
    XmlDocument doc;
    EXPECT_THROW(doc.CreateAttribute("1bad"), System::Exception);
    EXPECT_THROW(doc.CreateAttribute(""), System::Exception);
}

// Regression test for a wave-3 audit finding: CreateEntityReference never rejected a name
// starting with '#', which would collide with a character reference. Matches
// XmlEntityReference.cs's constructor.
TEST(XmlEntityReferenceTests, CreateEntityReference_NameStartsWithHash_Throws) {
    XmlDocument doc;
    EXPECT_THROW(doc.CreateEntityReference("#65"), System::ArgumentException);
}

TEST(XmlEntityReferenceTests, CreateEntityReference_ValidName_DoesNotThrow) {
    XmlDocument doc;
    EXPECT_NO_THROW(doc.CreateEntityReference("amp"));
}

TEST(XmlDocumentTypeTests, CreateDocumentType_StoresNameAndIds) {
    XmlDocument doc;
    auto* dt = doc.CreateDocumentType("html", "-//W3C//DTD XHTML 1.0//EN",
                                      "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd", "");
    EXPECT_EQ(dt->getNameProperty(), "html");
    EXPECT_EQ(dt->getPublicIdProperty(), "-//W3C//DTD XHTML 1.0//EN");
}

// ===========================================================================
// Load / LoadXml / Save
// ===========================================================================

TEST(XmlDocumentTests, LoadXml_ParsesDocumentElement) {
    XmlDocument doc;
    doc.LoadXml("<root attr=\"1\"><child>text</child></root>");
    ASSERT_NE(doc.getDocumentElementProperty(), nullptr);
    EXPECT_EQ(doc.getDocumentElementProperty()->getNameProperty(), "root");
    EXPECT_EQ(doc.getDocumentElementProperty()->GetAttribute("attr"), "1");
}

TEST(XmlDocumentTests, LoadXml_MalformedXml_ThrowsXmlException) {
    XmlDocument doc;
    EXPECT_THROW(doc.LoadXml("<root><unclosed></root>"), XmlException);
}

TEST(XmlDocumentTests, GetElementsByTagName_FindsAllDescendants) {
    XmlDocument doc;
    doc.LoadXml("<root><item/><nested><item/></nested></root>");
    // GetElementsByTagName returns a caller-owned XmlNodeList (documented at its call site in
    // XmlDocument.cpp) -- delete it explicitly, confirmed as a real AddressSanitizer-caught leak
    // otherwise (2026-07-14).
    auto* items = doc.GetElementsByTagName("item");
    EXPECT_EQ(items->getCountProperty(), 2);
    delete items;
}

TEST(XmlDocumentTests, GetElementById_FindsMatchingElement) {
    XmlDocument doc;
    doc.LoadXml("<root><item id=\"foo\"/><item id=\"bar\"/></root>");
    auto* found = doc.GetElementById("bar");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->GetAttribute("id"), "bar");
}

TEST(XmlDocumentTests, Save_ThenLoad_RoundTrips) {
    XmlDocument doc;
    doc.LoadXml("<root><child>value</child></root>");
    std::string path = "/tmp/sharp_rt_xmldoc_roundtrip_test.xml";
    doc.Save(path);

    XmlDocument doc2;
    doc2.Load(path);
    ASSERT_NE(doc2.getDocumentElementProperty(), nullptr);
    EXPECT_EQ(doc2.getDocumentElementProperty()->getNameProperty(), "root");
    std::remove(path.c_str());
}

// ===========================================================================
// WriteTo (XmlWriter interop)
// ===========================================================================

TEST(XmlDocumentTests, WriteTo_ProducesEquivalentXml) {
    XmlDocument doc;
    doc.LoadXml("<root attr=\"v\"><child>text</child></root>");
    std::unique_ptr<XmlWriter> writer(XmlWriter::CreateToString());
    doc.WriteTo(*writer);
    std::string result = writer->ToString();
    EXPECT_NE(result.find("<root"), std::string::npos);
    EXPECT_NE(result.find("attr=\"v\""), std::string::npos);
    EXPECT_NE(result.find("text"), std::string::npos);
}

// ===========================================================================
// XmlQualifiedName-based namespace splitting on XmlNode
// ===========================================================================

TEST(XmlNodeTests, LocalNameAndPrefix_SplitOnColon) {
    XmlDocument doc;
    auto* el = doc.CreateElement("ns:book");
    EXPECT_EQ(el->getPrefixProperty(), "ns");
    EXPECT_EQ(el->getLocalNameProperty(), "book");
}

TEST(XmlNodeTests, NamespaceURI_ResolvesFromAncestorXmlnsDeclaration) {
    XmlDocument doc;
    doc.LoadXml("<root xmlns:ns=\"urn:test\"><ns:child/></root>");
    auto* root = doc.getDocumentElementProperty();
    auto* child = root->getFirstChildProperty();
    EXPECT_EQ(child->getNamespaceURIProperty(), "urn:test");
}

// ===========================================================================
// XmlImplementation.HasFeature
// ===========================================================================

TEST(XmlImplementationTests, HasFeature_Xml_EmptyVersion_ReturnsTrue) {
    XmlImplementation impl;
    EXPECT_TRUE(impl.HasFeature("XML", ""));
}

TEST(XmlImplementationTests, HasFeature_Xml_Version1_ReturnsTrue) {
    XmlImplementation impl;
    EXPECT_TRUE(impl.HasFeature("XML", "1.0"));
}

TEST(XmlImplementationTests, HasFeature_Xml_Version2_ReturnsTrue) {
    XmlImplementation impl;
    EXPECT_TRUE(impl.HasFeature("XML", "2.0"));
}

TEST(XmlImplementationTests, HasFeature_CaseInsensitiveFeatureName_ReturnsTrue) {
    XmlImplementation impl;
    EXPECT_TRUE(impl.HasFeature("xml", ""));
}

TEST(XmlImplementationTests, HasFeature_UnknownFeature_ReturnsFalse) {
    XmlImplementation impl;
    EXPECT_FALSE(impl.HasFeature("HTML", ""));
}

TEST(XmlImplementationTests, HasFeature_UnknownVersion_ReturnsFalse) {
    XmlImplementation impl;
    EXPECT_FALSE(impl.HasFeature("XML", "3.0"));
}

// ===========================================================================
// XmlEntity
// ===========================================================================

TEST(XmlEntityTests, SetInnerTextProperty_Throws) {
    XmlDocument doc;
    XmlEntity entity(&doc, "foo", "", "", "");
    EXPECT_THROW(entity.setInnerTextProperty("bar"), System::InvalidOperationException);
}
