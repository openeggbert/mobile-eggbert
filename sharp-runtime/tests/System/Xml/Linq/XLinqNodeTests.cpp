// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Covers the System.Xml.Linq node hierarchy: XObject, XNode, XContainer, XText, XCData,
// XComment, XProcessingInstruction, XDocumentType, XStreamingElement,
// XNodeDocumentOrderComparer, XNodeEqualityComparer, and the Extensions free functions. Also
// covers real (non-stub) XElement::Parse/Load and XDocument::Parse/Load round-tripping mixed
// content through tinyxml2.
#include <gtest/gtest.h>
#include "System/InvalidOperationException.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/Linq/Extensions.hpp"
#include "System/Xml/Linq/XAttribute.hpp"
#include "System/Xml/Linq/XCData.hpp"
#include "System/Xml/Linq/XComment.hpp"
#include "System/Xml/Linq/XContainer.hpp"
#include "System/Xml/Linq/XDocument.hpp"
#include "System/Xml/Linq/XDocumentType.hpp"
#include "System/Xml/Linq/XElement.hpp"
#include "System/Xml/Linq/XNode.hpp"
#include "System/Xml/Linq/XNodeDocumentOrderComparer.hpp"
#include "System/Xml/Linq/XNodeEqualityComparer.hpp"
#include "System/Xml/Linq/XObject.hpp"
#include "System/Xml/Linq/XProcessingInstruction.hpp"
#include "System/Xml/Linq/XStreamingElement.hpp"
#include "System/Xml/Linq/XText.hpp"

using System::InvalidOperationException;
using System::Xml::XmlException;
using System::Xml::XmlNodeType;
using System::Xml::XmlWriter;
using System::Xml::Linq::XAttribute;
using System::Xml::Linq::XCData;
using System::Xml::Linq::XComment;
using System::Xml::Linq::XDeclaration;
using System::Xml::Linq::XDocument;
using System::Xml::Linq::XDocumentType;
using System::Xml::Linq::XElement;
using System::Xml::Linq::XName;
using System::Xml::Linq::XNode;
using System::Xml::Linq::XNodeDocumentOrderComparer;
using System::Xml::Linq::XNodeEqualityComparer;
using System::Xml::Linq::XProcessingInstruction;
using System::Xml::Linq::XStreamingElement;
using System::Xml::Linq::XText;

// ===========================================================================
// XObject
// ===========================================================================

TEST(XObjectTests, GetParentProperty_NullForDetached) {
    XElement e("a");
    EXPECT_EQ(e.getParentProperty(), nullptr);
}

TEST(XObjectTests, GetParentProperty_NullWhenParentIsDocument) {
    auto root = std::make_shared<XElement>("root");
    XDocument doc(root);
    // XObject::Parent only returns an XElement parent (matches .NET: `parent as XElement`).
    EXPECT_EQ(root->getParentProperty(), nullptr);
}

TEST(XObjectTests, GetParentProperty_ReturnsOwningElement) {
    auto parent = std::make_shared<XElement>("parent");
    auto child = std::make_shared<XElement>("child");
    parent->Add(child);
    EXPECT_EQ(child->getParentProperty(), parent.get());
}

TEST(XObjectTests, GetDocumentProperty_ReturnsOwningDocument) {
    auto root = std::make_shared<XElement>("root");
    auto child = std::make_shared<XElement>("child");
    root->Add(child);
    XDocument doc(root);
    EXPECT_EQ(child->getDocumentProperty(), &doc);
    EXPECT_EQ(root->getDocumentProperty(), &doc);
}

TEST(XObjectTests, GetDocumentProperty_NullForStandaloneElement) {
    XElement e("a");
    EXPECT_EQ(e.getDocumentProperty(), nullptr);
}

TEST(XObjectTests, ChangedChangingEventAccessors_DoNotThrow) {
    XElement e("a");
    EXPECT_NO_THROW(e.add_Changed([](void*, const auto&) {}));
    EXPECT_NO_THROW(e.add_Changing([](void*, const auto&) {}));
}

// ===========================================================================
// XNode navigation / mutation
// ===========================================================================

TEST(XNodeTests, NextPreviousNode_Siblings) {
    auto root = std::make_shared<XElement>("root");
    auto a = std::make_shared<XElement>("a");
    auto b = std::make_shared<XComment>("mid");
    auto c = std::make_shared<XElement>("c");
    root->Add(a);
    root->Add(b);
    root->Add(c);

    EXPECT_EQ(a->getNextNodeProperty(), b);
    EXPECT_EQ(b->getNextNodeProperty(), c);
    EXPECT_EQ(c->getNextNodeProperty(), nullptr);
    EXPECT_EQ(a->getPreviousNodeProperty(), nullptr);
    EXPECT_EQ(b->getPreviousNodeProperty(), a);
    EXPECT_EQ(c->getPreviousNodeProperty(), b);
}

TEST(XNodeTests, Remove_DetachesFromParent) {
    auto root = std::make_shared<XElement>("root");
    auto child = std::make_shared<XElement>("child");
    root->Add(child);
    child->Remove();
    EXPECT_EQ(child->getParentProperty(), nullptr);
    EXPECT_TRUE(root->Elements().empty());
}

TEST(XNodeTests, Remove_NoParent_Throws) {
    XElement e("a");
    EXPECT_THROW(e.Remove(), InvalidOperationException);
}

TEST(XNodeTests, ReplaceWith_SwapsNode) {
    auto root = std::make_shared<XElement>("root");
    auto a = std::make_shared<XElement>("a");
    auto b = std::make_shared<XElement>("b");
    root->Add(a);
    auto replacement = std::make_shared<XElement>("replacement");
    a->ReplaceWith(replacement);
    ASSERT_EQ(root->Elements().size(), 1u);
    EXPECT_EQ(root->Elements()[0]->getNameProperty().getLocalNameProperty(), "replacement");
    (void)b;
}

TEST(XNodeTests, NodesBeforeAfterSelf) {
    auto root = std::make_shared<XElement>("root");
    auto a = std::make_shared<XElement>("a");
    auto b = std::make_shared<XElement>("b");
    auto c = std::make_shared<XElement>("c");
    root->Add(a);
    root->Add(b);
    root->Add(c);
    EXPECT_EQ(b->NodesBeforeSelf().size(), 1u);
    EXPECT_EQ(b->NodesAfterSelf().size(), 1u);
    EXPECT_EQ(a->NodesBeforeSelf().size(), 0u);
    EXPECT_EQ(c->NodesAfterSelf().size(), 0u);
}

TEST(XNodeTests, CompareDocumentOrder_Siblings) {
    auto root = std::make_shared<XElement>("root");
    auto a = std::make_shared<XElement>("a");
    auto b = std::make_shared<XElement>("b");
    root->Add(a);
    root->Add(b);
    EXPECT_EQ(XNode::CompareDocumentOrder(a.get(), b.get()), -1);
    EXPECT_EQ(XNode::CompareDocumentOrder(b.get(), a.get()), 1);
    EXPECT_EQ(XNode::CompareDocumentOrder(a.get(), a.get()), 0);
}

TEST(XNodeTests, CompareDocumentOrder_NoCommonAncestor_Throws) {
    XElement a("a");
    XElement b("b");
    EXPECT_THROW(XNode::CompareDocumentOrder(&a, &b), InvalidOperationException);
}

TEST(XNodeTests, CompareDocumentOrder_NestedDepths) {
    auto root = std::make_shared<XElement>("root");
    auto child = std::make_shared<XElement>("child");
    auto grandchild = std::make_shared<XElement>("grandchild");
    root->Add(child);
    child->Add(grandchild);
    EXPECT_EQ(XNode::CompareDocumentOrder(root.get(), grandchild.get()), -1);
    EXPECT_EQ(XNode::CompareDocumentOrder(grandchild.get(), root.get()), 1);
}

TEST(XNodeTests, DeepEquals_TextNodes) {
    XText a("hello");
    XText b("hello");
    XText c("world");
    EXPECT_TRUE(XNode::DeepEquals(&a, &b));
    EXPECT_FALSE(XNode::DeepEquals(&a, &c));
}

TEST(XNodeTests, DeepEquals_DifferentTypesNeverEqual) {
    XText t("x");
    XComment c("x");
    EXPECT_FALSE(XNode::DeepEquals(&t, &c));
}

TEST(XNodeTests, DeepEquals_NullHandling) {
    XText a("x");
    EXPECT_TRUE(XNode::DeepEquals(nullptr, nullptr));
    EXPECT_FALSE(XNode::DeepEquals(&a, nullptr));
    EXPECT_FALSE(XNode::DeepEquals(nullptr, &a));
}

TEST(XNodeTests, DeepEquals_Elements_IgnoresCommentsAndPIs) {
    XElement a("root");
    a.Add(std::make_shared<XElement>("child"));
    a.Add(std::make_shared<XComment>("note"));
    XElement b("root");
    b.Add(std::make_shared<XElement>("child"));
    EXPECT_TRUE(XNode::DeepEquals(&a, &b));
}

// Regression test for a wave-3 audit finding: DeepEqualsCore compared attributes via
// name-lookup (order-independent, like an unordered set), but real .NET's AttributesEqual
// walks both attribute lists in parallel by *position* -- verified against XElement.cs.
// Two elements with the same attributes in a different order are NOT deep-equal.
TEST(XNodeTests, DeepEquals_Elements_AttributeOrderMatters) {
    XElement a("root");
    a.Add(std::make_shared<XAttribute>("x", "1"));
    a.Add(std::make_shared<XAttribute>("y", "2"));
    XElement b("root");
    b.Add(std::make_shared<XAttribute>("y", "2"));
    b.Add(std::make_shared<XAttribute>("x", "1"));
    EXPECT_FALSE(XNode::DeepEquals(&a, &b));

    XElement c("root");
    c.Add(std::make_shared<XAttribute>("x", "1"));
    c.Add(std::make_shared<XAttribute>("y", "2"));
    EXPECT_TRUE(XNode::DeepEquals(&a, &c));
}

// Regression test for a wave-3 audit finding: XElement had no ValidateNode override at all
// (inherited XContainer's no-op default), so an XDocument or XDocumentType could be added as
// a child element -- a structurally invalid tree real .NET rejects. Verified against
// XElement.cs's ValidateNode.
TEST(XElementTests, Add_XDocument_ThrowsArgumentException) {
    XElement parent("root");
    auto doc = std::make_shared<XDocument>();
    EXPECT_THROW(parent.Add(doc), System::ArgumentException);
}

TEST(XElementTests, Add_XDocumentType_ThrowsArgumentException) {
    XElement parent("root");
    auto dt = std::make_shared<XDocumentType>("html", "", "", "");
    EXPECT_THROW(parent.Add(dt), System::ArgumentException);
}

TEST(XNodeTests, ToString_NoArgs_IsIndentedByDefault) {
    auto root = std::make_shared<XElement>("root");
    root->Add(std::make_shared<XElement>("child"));
    std::string s = root->ToString();
    EXPECT_NE(s.find('\n'), std::string::npos);
}

// ===========================================================================
// XContainer
// ===========================================================================

TEST(XContainerTests, FirstNodeLastNode) {
    auto root = std::make_shared<XElement>("root");
    auto a = std::make_shared<XElement>("a");
    auto b = std::make_shared<XElement>("b");
    root->Add(a);
    root->Add(b);
    EXPECT_EQ(root->getFirstNodeProperty(), a);
    EXPECT_EQ(root->getLastNodeProperty(), b);
}

TEST(XContainerTests, AddFirst_InsertsAtFront) {
    auto root = std::make_shared<XElement>("root");
    root->Add(std::make_shared<XElement>("b"));
    root->AddFirst(std::make_shared<XElement>("a"));
    auto els = root->Elements();
    ASSERT_EQ(els.size(), 2u);
    EXPECT_EQ(els[0]->getNameProperty().getLocalNameProperty(), "a");
    EXPECT_EQ(els[1]->getNameProperty().getLocalNameProperty(), "b");
}

TEST(XContainerTests, DescendantNodes_IncludesAllLevels) {
    auto root = std::make_shared<XElement>("root");
    auto child = std::make_shared<XElement>("child");
    child->Add(std::make_shared<XText>("leaf"));
    root->Add(child);
    auto nodes = root->DescendantNodes();
    ASSERT_EQ(nodes.size(), 2u); // child element + its text
}

TEST(XContainerTests, Add_SelfAsChild_ThrowsInsteadOfCreatingCycle) {
    // Regression: InsertNodeAt previously had no cycle guard -- inserting a node into its own
    // subtree created a permanent shared_ptr reference cycle (the container ends up holding,
    // transitively, a shared_ptr back to itself) and would stack-overflow any recursive
    // traversal (Nodes()/ToString()/etc.).
    auto root = std::make_shared<XElement>("root");
    EXPECT_THROW(root->Add(root), System::InvalidOperationException);
}

TEST(XContainerTests, Add_AncestorAsChild_ThrowsInsteadOfCreatingCycle) {
    auto root = std::make_shared<XElement>("root");
    auto child = std::make_shared<XElement>("child");
    root->Add(child);
    EXPECT_THROW(child->Add(root), System::InvalidOperationException);
}

TEST(XContainerTests, RemoveNodes_ClearsChildrenAndParent) {
    auto root = std::make_shared<XElement>("root");
    auto child = std::make_shared<XElement>("child");
    root->Add(child);
    root->RemoveNodes();
    EXPECT_TRUE(root->Nodes().empty());
    EXPECT_EQ(child->getParentProperty(), nullptr);
}

TEST(XContainerTests, Add_MovesNodeFromOldParent) {
    auto parentA = std::make_shared<XElement>("a");
    auto parentB = std::make_shared<XElement>("b");
    auto child = std::make_shared<XElement>("child");
    parentA->Add(child);
    parentB->Add(child);
    EXPECT_TRUE(parentA->Elements().empty());
    ASSERT_EQ(parentB->Elements().size(), 1u);
    EXPECT_EQ(child->getParentProperty(), parentB.get());
}

// ===========================================================================
// XText / XCData / XComment / XProcessingInstruction / XDocumentType
// ===========================================================================

TEST(XTextTests, ValueGetSet) {
    XText t("hello");
    EXPECT_EQ(t.getValueProperty(), "hello");
    t.setValueProperty("world");
    EXPECT_EQ(t.getValueProperty(), "world");
    EXPECT_EQ(t.getNodeTypeProperty(), XmlNodeType::Text);
}

TEST(XTextTests, SerializeTo_EscapesReservedChars) {
    XText t("a < b & c > d");
    EXPECT_EQ(t.ToString(), "a &lt; b &amp; c &gt; d");
}

TEST(XCDataTests, NodeTypeAndSerialization) {
    XCData cd("<raw> & stuff");
    EXPECT_EQ(cd.getNodeTypeProperty(), XmlNodeType::CDATA);
    EXPECT_EQ(cd.ToString(), "<![CDATA[<raw> & stuff]]>");
}

TEST(XCDataTests, IsAXText) {
    auto cd = std::make_shared<XCData>("hi");
    std::shared_ptr<XText> asText = cd; // must upcast cleanly
    EXPECT_EQ(asText->getValueProperty(), "hi");
}

TEST(XCommentTests, ValueAndSerialization) {
    XComment c("a comment");
    EXPECT_EQ(c.getValueProperty(), "a comment");
    EXPECT_EQ(c.getNodeTypeProperty(), XmlNodeType::Comment);
    EXPECT_EQ(c.ToString(), "<!--a comment-->");
}

TEST(XProcessingInstructionTests, TargetAndData) {
    XProcessingInstruction pi("xml-stylesheet", "type=\"text/xsl\" href=\"style.xsl\"");
    EXPECT_EQ(pi.getTargetProperty(), "xml-stylesheet");
    EXPECT_EQ(pi.getDataProperty(), "type=\"text/xsl\" href=\"style.xsl\"");
    EXPECT_EQ(pi.getNodeTypeProperty(), XmlNodeType::ProcessingInstruction);
    EXPECT_EQ(pi.ToString(), "<?xml-stylesheet type=\"text/xsl\" href=\"style.xsl\"?>");
}

TEST(XDocumentTypeTests, FieldsAndSerialization) {
    XDocumentType dt("html", "", "", "");
    EXPECT_EQ(dt.getNameProperty(), "html");
    EXPECT_EQ(dt.getNodeTypeProperty(), XmlNodeType::DocumentType);
    EXPECT_EQ(dt.ToString(), "<!DOCTYPE html>");
}

TEST(XDocumentTypeTests, PublicAndSystemId) {
    XDocumentType dt("html", "-//W3C//DTD XHTML 1.0//EN", "xhtml1.dtd", "");
    EXPECT_EQ(dt.ToString(), "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0//EN\" \"xhtml1.dtd\">");
}

// Real .NET's XDeclaration.ToString() omits each of version/encoding/standalone individually
// when unset (null) -- it does NOT default-fill version to "1.0" the way an earlier version of
// this port's ToString() did (inconsistent with how encoding/standalone were already handled).
TEST(XDeclarationTests, ToString_AllFieldsSet) {
    XDeclaration decl("1.0", "utf-8", "yes");
    EXPECT_EQ(decl.ToString(), "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>");
}

TEST(XDeclarationTests, ToString_EmptyVersion_OmitsVersionAttribute) {
    XDeclaration decl("", "utf-8", "");
    EXPECT_EQ(decl.ToString(), "<?xml encoding=\"utf-8\"?>");
}

TEST(XDeclarationTests, ToString_AllFieldsEmpty_JustXmlPI) {
    XDeclaration decl("", "", "");
    EXPECT_EQ(decl.ToString(), "<?xml?>");
}

// ===========================================================================
// WriteTo(XmlWriter&)
// ===========================================================================

TEST(WriteToTests, XElement_WritesStartAndEndTags) {
    XElement e("root");
    e.Add(std::make_shared<XAttribute>("id", "1"));
    e.Add(std::make_shared<XElement>("child"));
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    e.WriteTo(*w);
    std::string out = w->ToString();
    EXPECT_NE(out.find("<root"), std::string::npos);
    EXPECT_NE(out.find("id=\"1\""), std::string::npos);
    EXPECT_NE(out.find("<child"), std::string::npos);
}

TEST(WriteToTests, XElement_NamespacedAttribute_DoesNotEmitClarkNotation) {
    // Regression: XElement::WriteTo() previously wrote XName::ToString()'s Clark notation
    // ("{namespace}local") directly as the attribute *name* -- '{'/'}' are not legal in an XML
    // Name production, so this produced literally malformed, unparseable XML for any
    // namespaced attribute instead of just a namespace-fidelity gap.
    XElement e("root");
    e.Add(std::make_shared<XAttribute>(XName("http://example.com/ns", "kind"), "rare"));
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    e.WriteTo(*w);
    std::string out = w->ToString();
    EXPECT_EQ(out.find('{'), std::string::npos);
    EXPECT_EQ(out.find('}'), std::string::npos);
    EXPECT_NE(out.find("kind=\"rare\""), std::string::npos);
}

TEST(WriteToTests, XComment_WritesComment) {
    XComment c("hi");
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    XElement wrapper("root");
    w->WriteStartElement("root");
    c.WriteTo(*w);
    w->WriteEndElement();
    EXPECT_NE(w->ToString().find("hi"), std::string::npos);
}

TEST(WriteToTests, XProcessingInstruction_WritesPI) {
    XProcessingInstruction pi("target", "data");
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    w->WriteStartElement("root");
    pi.WriteTo(*w);
    w->WriteEndElement();
    std::string out = w->ToString();
    EXPECT_NE(out.find("<?target data?>"), std::string::npos);
}

TEST(WriteToTests, XDocumentType_WritesDocType) {
    XDocumentType dt("root", "", "", "");
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    dt.WriteTo(*w);
    EXPECT_NE(w->ToString().find("<!DOCTYPE root>"), std::string::npos);
}

// Regression test for a wave-3 audit finding: XDocument::WriteTo() only wrote a declaration
// when an explicit XDeclaration had been set, and never called WriteEndDocument() at all.
// Verified against XDocument.cs's WriteTo(): real .NET always calls WriteStartDocument()/
// WriteEndDocument() as a matched pair, regardless of whether an XDeclaration was set.
TEST(WriteToTests, XDocument_NoExplicitDeclaration_StillWritesXmlDeclaration) {
    XDocument doc(std::make_shared<XElement>("root"));
    std::unique_ptr<XmlWriter> w(XmlWriter::CreateToString());
    doc.WriteTo(*w);
    EXPECT_NE(w->ToString().find("<?xml"), std::string::npos);
    EXPECT_NE(w->ToString().find("<root"), std::string::npos);
}

// ===========================================================================
// XElement.Value with mixed content
// ===========================================================================

TEST(XElementValueTests, ConcatenatesMixedTextContent) {
    auto e = std::make_shared<XElement>("p");
    e->Add(std::string("Hello "));
    e->Add(std::make_shared<XElement>("b"));
    e->Elements()[0]->setValueProperty("world");
    e->Add(std::string("!"));
    EXPECT_EQ(e->getValueProperty(), "Hello world!");
}

// ===========================================================================
// XElement::Parse / XDocument::Parse round-tripping mixed node types
// ===========================================================================

TEST(XLinqParseTests, ParsesCommentsCDataTextAndElements) {
    auto root = XElement::Parse(
        "<root><!--a note--><data><![CDATA[raw <data>]]></data><p>plain text</p></root>");
    ASSERT_NE(root, nullptr);
    auto nodes = root->Nodes();
    ASSERT_EQ(nodes.size(), 3u);
    EXPECT_EQ(nodes[0]->getNodeTypeProperty(), XmlNodeType::Comment);
    EXPECT_EQ(static_cast<XComment*>(nodes[0].get())->getValueProperty(), "a note");

    auto dataEl = root->Element("data");
    ASSERT_NE(dataEl, nullptr);
    auto dataNodes = dataEl->Nodes();
    ASSERT_EQ(dataNodes.size(), 1u);
    EXPECT_EQ(dataNodes[0]->getNodeTypeProperty(), XmlNodeType::CDATA);
    EXPECT_EQ(static_cast<XCData*>(dataNodes[0].get())->getValueProperty(), "raw <data>");

    auto pEl = root->Element("p");
    ASSERT_NE(pEl, nullptr);
    EXPECT_EQ(pEl->getValueProperty(), "plain text");
}

TEST(XLinqParseTests, ParsesProcessingInstructionAndDocType) {
    auto doc = XDocument::Parse(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<?xml-stylesheet type=\"text/xsl\" href=\"a.xsl\"?>"
        "<root/>");
    ASSERT_NE(doc->getDeclarationProperty(), nullptr);
    EXPECT_EQ(doc->getDeclarationProperty()->getEncodingProperty(), "utf-8");

    bool foundPi = false;
    for (auto& n : doc->Nodes()) {
        if (n->getNodeTypeProperty() == XmlNodeType::ProcessingInstruction) {
            auto* pi = static_cast<XProcessingInstruction*>(n.get());
            EXPECT_EQ(pi->getTargetProperty(), "xml-stylesheet");
            foundPi = true;
        }
    }
    EXPECT_TRUE(foundPi);
    ASSERT_NE(doc->getRootProperty(), nullptr);
}

TEST(XLinqParseTests, DefaultParse_TrimsInsignificantWhitespace) {
    auto root = XElement::Parse("<root>\n  <a/>\n  <b/>\n</root>");
    // Whitespace-only text nodes between elements are trimmed by default (LoadOptions::None).
    // (The vendored tinyxml2 parser doesn't even surface pure-whitespace-only runs adjacent to
    // element tags as text nodes in the first place — verified directly against tinyxml2 — so
    // this also holds vacuously; see the PreserveWhitespace test below for the one case that
    // *is* actually under this converter's control: whitespace embedded in real mixed content.)
    EXPECT_EQ(root->Nodes().size(), 2u);
}

TEST(XLinqParseTests, PreserveWhitespace_DoesNotAffectMixedContentText) {
    // LoadOptions::PreserveWhitespace only changes whether *pure*-whitespace-only text nodes are
    // dropped (see IsAllWhitespace() in XDocument.cpp) — text that mixes whitespace with real
    // content is never trimmed, with or without the option. tinyxml2 itself never surfaces
    // pure-whitespace-only runs immediately adjacent to element tags as separate text nodes at
    // all (verified directly against the vendored parser), so LoadOptions::PreserveWhitespace
    // has no observable effect for that specific case — a documented, inherited limitation of
    // the parsing backend, not new here (see XmlDocument::getPreserveWhitespaceProperty()'s
    // doc-comment for the same caveat at the classic-DOM layer).
    auto withDefault = XElement::Parse("<p>  hello  <b/>  world  </p>");
    auto withPreserve = XElement::Parse("<p>  hello  <b/>  world  </p>", System::Xml::Linq::LoadOptions::PreserveWhitespace);
    EXPECT_EQ(withDefault->getValueProperty(), "  hello    world  ");
    EXPECT_EQ(withPreserve->getValueProperty(), "  hello    world  ");
}

TEST(XLinqParseTests, RoundTrip_ParseThenToStringThenParseAgain_Equal) {
    auto original = XElement::Parse("<root a=\"1\"><child>text</child></root>");
    auto roundTripped = XElement::Parse(original->ToString());
    EXPECT_TRUE(XNode::DeepEquals(original.get(), roundTripped.get()));
}

TEST(XLinqParseTests, MalformedXml_Throws) {
    EXPECT_THROW(XElement::Parse("<a><b></a>"), XmlException);
}

// ===========================================================================
// XDocument structural constraints
// ===========================================================================

TEST(XDocumentStructureTests, SecondRootElement_Throws) {
    XDocument doc(std::make_shared<XElement>("root"));
    EXPECT_THROW(doc.Add(std::static_pointer_cast<XNode>(std::make_shared<XElement>("other"))), InvalidOperationException);
}

// Regression test: real .NET's XDocument.ValidateNode throws ArgumentException (not
// InvalidOperationException) for non-whitespace text -- it's a fundamentally wrong node
// *kind* for this container, not a conflict with the document's current state. Verified
// against XDocument.cs's ValidateString.
TEST(XDocumentStructureTests, NonWhitespaceTextNode_ThrowsArgumentException) {
    XDocument doc;
    EXPECT_THROW(doc.Add(std::static_pointer_cast<XNode>(std::make_shared<XText>("stray"))), System::ArgumentException);
}

// Regression test for a wave-3 audit finding: the previous ValidateNode unconditionally
// rejected every top-level XText, including whitespace-only text (e.g. indentation between
// top-level nodes), which real .NET's ValidateString explicitly allows.
TEST(XDocumentStructureTests, WhitespaceOnlyTextNode_DoesNotThrow) {
    XDocument doc;
    EXPECT_NO_THROW(doc.Add(std::static_pointer_cast<XNode>(std::make_shared<XText>("  \t\r\n  "))));
}

TEST(XDocumentStructureTests, CDataNode_ThrowsArgumentException) {
    XDocument doc;
    EXPECT_THROW(doc.Add(std::static_pointer_cast<XNode>(std::make_shared<XCData>("cdata"))), System::ArgumentException);
}

TEST(XDocumentStructureTests, NestedXDocument_ThrowsArgumentException) {
    XDocument doc;
    EXPECT_THROW(doc.Add(std::static_pointer_cast<XNode>(std::make_shared<XDocument>())), System::ArgumentException);
}

TEST(XDocumentStructureTests, CommentsAndPIsAllowedAtTopLevel) {
    XDocument doc;
    EXPECT_NO_THROW(doc.Add(std::static_pointer_cast<XNode>(std::make_shared<XComment>("c"))));
    EXPECT_NO_THROW(doc.Add(std::static_pointer_cast<XNode>(std::make_shared<XProcessingInstruction>("t", "d"))));
}

TEST(XDocumentStructureTests, SetRootProperty_ReplacesExistingRoot) {
    XDocument doc(std::make_shared<XElement>("first"));
    doc.setRootProperty(std::make_shared<XElement>("second"));
    ASSERT_NE(doc.getRootProperty(), nullptr);
    EXPECT_EQ(doc.getRootProperty()->getNameProperty().getLocalNameProperty(), "second");
}

// ===========================================================================
// XStreamingElement
// ===========================================================================

TEST(XStreamingElementTests, WritesTextAttributeAndChildElement) {
    XStreamingElement se("root");
    se.Add(std::make_shared<XAttribute>("id", "1"));
    se.Add(std::string("hello"));
    se.Add(std::static_pointer_cast<XNode>(std::make_shared<XElement>("child")));
    std::string s = se.ToString();
    EXPECT_NE(s.find("<root"), std::string::npos);
    EXPECT_NE(s.find("id=\"1\""), std::string::npos);
    EXPECT_NE(s.find("hello"), std::string::npos);
    EXPECT_NE(s.find("<child"), std::string::npos);
}

TEST(XStreamingElementTests, NestedStreamingElement) {
    auto inner = std::make_shared<XStreamingElement>(XName("inner"));
    inner->Add(std::string("val"));
    XStreamingElement outer("outer");
    outer.Add(inner);
    std::string s = outer.ToString();
    EXPECT_NE(s.find("<inner"), std::string::npos);
    EXPECT_NE(s.find("val"), std::string::npos);
}

// ===========================================================================
// XNodeDocumentOrderComparer / XNodeEqualityComparer
// ===========================================================================

TEST(XNodeDocumentOrderComparerTests, ComparesLikeCompareDocumentOrder) {
    auto root = std::make_shared<XElement>("root");
    auto a = std::make_shared<XElement>("a");
    auto b = std::make_shared<XElement>("b");
    root->Add(a);
    root->Add(b);
    XNodeDocumentOrderComparer cmp;
    EXPECT_EQ(cmp.Compare(a.get(), b.get()), -1);
    EXPECT_TRUE(cmp(a.get(), b.get()));
    EXPECT_FALSE(cmp(b.get(), a.get()));
}

TEST(XNodeEqualityComparerTests, ComparesByValue) {
    XText a("x");
    XText b("x");
    XNodeEqualityComparer cmp;
    EXPECT_TRUE(cmp.Equals(&a, &b));
    EXPECT_EQ(cmp.GetHashCode(&a), cmp.GetHashCode(&b));
}

// ===========================================================================
// Extensions
// ===========================================================================

TEST(ExtensionsTests, Elements_OverMultipleContainers) {
    auto e1 = std::make_shared<XElement>("a");
    e1->Add(std::make_shared<XElement>("x"));
    auto e2 = std::make_shared<XElement>("b");
    e2->Add(std::make_shared<XElement>("y"));
    std::vector<std::shared_ptr<XElement>> source{e1, e2};
    auto result = System::Xml::Linq::Extensions::Elements(source);
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0]->getNameProperty().getLocalNameProperty(), "x");
    EXPECT_EQ(result[1]->getNameProperty().getLocalNameProperty(), "y");
}

TEST(ExtensionsTests, Attributes_OverMultipleElements) {
    auto e1 = std::make_shared<XElement>("a");
    e1->Add(std::make_shared<XAttribute>("k1", "v1"));
    auto e2 = std::make_shared<XElement>("b");
    e2->Add(std::make_shared<XAttribute>("k2", "v2"));
    std::vector<std::shared_ptr<XElement>> source{e1, e2};
    auto result = System::Xml::Linq::Extensions::Attributes(source);
    ASSERT_EQ(result.size(), 2u);
}

TEST(ExtensionsTests, Descendants_WithNameFilter) {
    auto root = std::make_shared<XElement>("root");
    auto child = std::make_shared<XElement>("child");
    child->Add(std::make_shared<XElement>("target"));
    root->Add(child);
    std::vector<std::shared_ptr<XElement>> source{root};
    auto result = System::Xml::Linq::Extensions::Descendants(source, XName("target"));
    EXPECT_EQ(result.size(), 1u);
}

TEST(ExtensionsTests, Remove_RemovesAllNodesInSnapshot) {
    auto root = std::make_shared<XElement>("root");
    auto a = std::make_shared<XElement>("a");
    auto b = std::make_shared<XElement>("b");
    root->Add(a);
    root->Add(b);
    std::vector<std::shared_ptr<XNode>> toRemove{a, b};
    System::Xml::Linq::Extensions::Remove(toRemove);
    EXPECT_TRUE(root->Elements().empty());
}

TEST(ExtensionsTests, InDocumentOrder_SortsNodes) {
    auto root = std::make_shared<XElement>("root");
    auto a = std::make_shared<XElement>("a");
    auto b = std::make_shared<XElement>("b");
    auto c = std::make_shared<XElement>("c");
    root->Add(a);
    root->Add(b);
    root->Add(c);
    std::vector<std::shared_ptr<XNode>> shuffled{c, a, b};
    auto sorted = System::Xml::Linq::Extensions::InDocumentOrder(shuffled);
    ASSERT_EQ(sorted.size(), 3u);
    EXPECT_EQ(sorted[0], a);
    EXPECT_EQ(sorted[1], b);
    EXPECT_EQ(sorted[2], c);
}

TEST(ExtensionsTests, Ancestors_ReturnsParentChain) {
    auto root = std::make_shared<XElement>("root");
    auto mid = std::make_shared<XElement>("mid");
    auto leaf = std::make_shared<XElement>("leaf");
    root->Add(mid);
    mid->Add(leaf);
    std::vector<std::shared_ptr<XNode>> source{std::static_pointer_cast<XNode>(leaf)};
    auto ancestors = System::Xml::Linq::Extensions::Ancestors(source);
    ASSERT_EQ(ancestors.size(), 2u);
    EXPECT_EQ(ancestors[0], mid.get());
    EXPECT_EQ(ancestors[1], root.get());
}

// Real .NET's Extensions.cs also exposes Attributes(source, name), Ancestors(source, name), and
// AncestorsAndSelf(source[, name]) -- all four were missing from this port's Extensions.hpp
// despite AncestorsAndSelf being explicitly listed as in-scope in the file's own class
// doc-comment. Added to close that gap.

TEST(ExtensionsTests, Attributes_WithNameFilter) {
    auto e1 = std::make_shared<XElement>("a");
    e1->Add(std::make_shared<XAttribute>("k1", "v1"));
    e1->Add(std::make_shared<XAttribute>("k2", "x"));
    auto e2 = std::make_shared<XElement>("b");
    e2->Add(std::make_shared<XAttribute>("k1", "v2"));
    std::vector<std::shared_ptr<XElement>> source{e1, e2};
    auto result = System::Xml::Linq::Extensions::Attributes(source, XName("k1"));
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0]->getValueProperty(), "v1");
    EXPECT_EQ(result[1]->getValueProperty(), "v2");
}

TEST(ExtensionsTests, Ancestors_WithNameFilter) {
    auto root = std::make_shared<XElement>("root");
    auto mid = std::make_shared<XElement>("mid");
    auto leaf = std::make_shared<XElement>("leaf");
    root->Add(mid);
    mid->Add(leaf);
    std::vector<std::shared_ptr<XNode>> source{std::static_pointer_cast<XNode>(leaf)};
    auto ancestors = System::Xml::Linq::Extensions::Ancestors(source, XName("root"));
    ASSERT_EQ(ancestors.size(), 1u);
    EXPECT_EQ(ancestors[0], root.get());
}

TEST(ExtensionsTests, AncestorsAndSelf_IncludesSourceElement) {
    auto root = std::make_shared<XElement>("root");
    auto mid = std::make_shared<XElement>("mid");
    root->Add(mid);
    std::vector<std::shared_ptr<XElement>> source{mid};
    auto result = System::Xml::Linq::Extensions::AncestorsAndSelf(source);
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], mid.get());
    EXPECT_EQ(result[1], root.get());
}

TEST(ExtensionsTests, AncestorsAndSelf_WithNameFilter) {
    auto root = std::make_shared<XElement>("root");
    auto mid = std::make_shared<XElement>("mid");
    root->Add(mid);
    std::vector<std::shared_ptr<XElement>> source{mid};
    auto result = System::Xml::Linq::Extensions::AncestorsAndSelf(source, XName("root"));
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], root.get());
}
