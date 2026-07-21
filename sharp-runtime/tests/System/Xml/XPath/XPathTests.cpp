// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Coverage for the System.Xml.XPath practical subset (XPathNavigator/XPathDocument/
// XPathExpression/XPathNodeIterator) built over the classic XmlDocument DOM. See
// XPathNavigator's class doc-comment for exactly which XPath 1.0 constructs are supported.
#include <gtest/gtest.h>
#include <memory>

#include "System/Xml/XPath/XPathDocument.hpp"
#include "System/Xml/XPath/XPathException.hpp"
#include "System/Xml/XPath/XPathExpression.hpp"
#include "System/Xml/XPath/XPathNavigator.hpp"
#include "System/Xml/XmlConvert.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"

using namespace System::Xml;
using namespace System::Xml::XPath;

namespace {

    // A real, non-trivial parsed XML fixture (bookstore catalog), reused across most tests —
    // exercises the parser/evaluator against actual tinyxml2-parsed markup, not a synthetic
    // programmatically-built tree.
    constexpr const char* kCatalogXml = R"(<?xml version="1.0"?>
<?target some data?>
<catalog xmlns:ns="urn:example:ns">
  <book id="bk101" available="true">
    <title>XML Developer's Guide</title>
    <author>Gambardella, Matthew</author>
    <price>44.95</price>
  </book>
  <book id="bk102" available="false">
    <title>Midnight Rain</title>
    <author>Ralls, Kim</author>
    <price>5.95</price>
  </book>
  <book id="bk103" available="true">
    <title>Maeve Ascendant</title>
    <author>Corets, Eva</author>
    <price>5.95</price>
  </book>
  <ns:special ns:kind="rare">A namespaced element</ns:special>
  <!-- a comment node -->
</catalog>
)";

    std::unique_ptr<XmlDocument> LoadCatalog() {
        auto doc = std::make_unique<XmlDocument>();
        doc->LoadXml(kCatalogXml);
        return doc;
    }

    // XmlNode::SelectSingleNode() (unwraps back to XmlNode*) is a different overload from
    // XPathNavigator::SelectSingleNode() (returns XPathNavigator*); these tests want the latter
    // (to read Value/Name/GetAttribute with XPath semantics), so go through CreateNavigator()
    // explicitly rather than calling SelectSingleNode() directly on an XmlNode/XmlElement*.
    std::unique_ptr<XPathNavigator> SelectNav(XmlNode* node, const std::string& xpath) {
        std::unique_ptr<XPathNavigator> nav(node->CreateNavigator());
        return std::unique_ptr<XPathNavigator>(nav->SelectSingleNode(xpath));
    }

} // namespace

// ===========================================================================
// Basic navigation / position properties
// ===========================================================================

TEST(XPathNavigatorTests, CreateNavigator_FromDocument_PositionedAtRoot) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_EQ(nav->getNodeTypeProperty(), XPathNodeType::Root);
    EXPECT_EQ(nav->getNameProperty(), "");
}

TEST(XPathNavigatorTests, CreateNavigator_FromElement_PositionedAtThatElement) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator());
    EXPECT_EQ(nav->getNodeTypeProperty(), XPathNodeType::Element);
    EXPECT_EQ(nav->getNameProperty(), "catalog");
}

TEST(XPathNavigatorTests, MoveToFirstChild_ThenMoveToNext_WalksSiblings) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator()); // catalog
    ASSERT_TRUE(nav->MoveToFirstChild()); // first book
    EXPECT_EQ(nav->getNameProperty(), "book");
    ASSERT_TRUE(nav->MoveToNext());
    EXPECT_EQ(nav->GetAttribute("id", ""), "bk102");
}

TEST(XPathNavigatorTests, MoveToParent_ReturnsToOwningElement) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator()); // catalog
    nav->MoveToFirstChild();
    ASSERT_TRUE(nav->MoveToParent());
    EXPECT_EQ(nav->getNameProperty(), "catalog");
}

TEST(XPathNavigatorTests, MoveToPrevious_WalksBackward) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator()); // catalog
    nav->MoveToFirstChild();
    nav->MoveToNext();
    ASSERT_TRUE(nav->MoveToPrevious());
    EXPECT_EQ(nav->GetAttribute("id", ""), "bk101");
}

TEST(XPathNavigatorTests, Value_OnElement_IsConcatenatedDescendantText) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(SelectNav(doc->getDocumentElementProperty(), "book[1]/title"));
    ASSERT_NE(nav, nullptr);
    EXPECT_EQ(nav->getValueProperty(), "XML Developer's Guide");
}

TEST(XPathNavigatorTests, IsEmptyElement_TrueForChildlessElement_FalseOtherwise) {
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    auto* empty = doc.CreateElement("empty");
    root->AppendChild(empty);
    auto* full = doc.CreateElement("full");
    full->setInnerTextProperty("x");
    root->AppendChild(full);

    std::unique_ptr<XPathNavigator> emptyNav(empty->CreateNavigator());
    std::unique_ptr<XPathNavigator> fullNav(full->CreateNavigator());
    EXPECT_TRUE(emptyNav->getIsEmptyElementProperty());
    EXPECT_FALSE(fullNav->getIsEmptyElementProperty());
}

// ===========================================================================
// Attribute axis
// ===========================================================================

TEST(XPathNavigatorTests, MoveToFirstAttribute_ThenNext_WalksAttributes) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    ASSERT_NE(nav, nullptr);
    ASSERT_TRUE(nav->MoveToFirstAttribute());
    EXPECT_EQ(nav->getLocalNameProperty(), "id");
    ASSERT_TRUE(nav->MoveToNextAttribute());
    EXPECT_EQ(nav->getLocalNameProperty(), "available");
    EXPECT_FALSE(nav->MoveToNextAttribute());
}

TEST(XPathNavigatorTests, HasAttributes_TrueOnlyWhenAttributesPresent) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> withAttrs(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    std::unique_ptr<XPathNavigator> withoutAttrs(SelectNav(doc->getDocumentElementProperty(), "book[1]/title"));
    EXPECT_TRUE(withAttrs->getHasAttributesProperty());
    EXPECT_FALSE(withoutAttrs->getHasAttributesProperty());
}

TEST(XPathNavigatorTests, GetAttribute_DoesNotDisturbCurrentPosition) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    ASSERT_EQ(nav->GetAttribute("id", ""), "bk101");
    EXPECT_EQ(nav->getNodeTypeProperty(), XPathNodeType::Element);
    EXPECT_EQ(nav->getNameProperty(), "book");
}

TEST(XPathNavigatorTests, MoveToAttribute_FindsMatchByLocalName) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    ASSERT_TRUE(nav->MoveToAttribute("available", ""));
    EXPECT_EQ(nav->getValueProperty(), "true");
}

TEST(XPathNavigatorTests, AttributeAxis_ExcludesXmlnsDeclarations) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> root(doc->getDocumentElementProperty()->CreateNavigator()); // catalog (has xmlns:ns)
    EXPECT_FALSE(root->getHasAttributesProperty());
}

// ===========================================================================
// Namespace axis
// ===========================================================================

TEST(XPathNavigatorTests, MoveToFirstNamespace_Local_FindsDeclaredPrefix) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator()); // catalog
    ASSERT_TRUE(nav->MoveToFirstNamespace(XPathNamespaceScope::Local));
    EXPECT_EQ(nav->getLocalNameProperty(), "ns");
    EXPECT_EQ(nav->getValueProperty(), "urn:example:ns");
    EXPECT_FALSE(nav->MoveToNextNamespace(XPathNamespaceScope::Local));
}

TEST(XPathNavigatorTests, MoveToFirstNamespace_All_IncludesImplicitXmlPrefix) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator()); // catalog
    ASSERT_TRUE(nav->MoveToFirstNamespace(XPathNamespaceScope::All));
    bool sawXml = false, sawNs = false;
    do {
        if (nav->getLocalNameProperty() == "xml") sawXml = true;
        if (nav->getLocalNameProperty() == "ns") sawNs = true;
    } while (nav->MoveToNextNamespace(XPathNamespaceScope::All));
    EXPECT_TRUE(sawXml);
    EXPECT_TRUE(sawNs);
}

TEST(XPathNavigatorTests, GetNamespacesInScope_ExcludeXml_OmitsXmlPrefix) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator()); // catalog
    auto ns = nav->GetNamespacesInScope(System::Xml::XmlNamespaceScope::ExcludeXml);
    EXPECT_EQ(ns.count("xml"), 0u);
    ASSERT_EQ(ns.count("ns"), 1u);
    EXPECT_EQ(ns.at("ns"), "urn:example:ns");
}

TEST(XPathNavigatorTests, LookupNamespace_ResolvesDeclaredPrefix) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator()); // catalog
    auto uri = nav->LookupNamespace("ns");
    ASSERT_TRUE(uri.has_value());
    EXPECT_EQ(*uri, "urn:example:ns");
}

// ===========================================================================
// Comparison: IsSamePosition / ComparePosition
// ===========================================================================

TEST(XPathNavigatorTests, IsSamePosition_SameNode_True_DifferentNode_False) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> a(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    std::unique_ptr<XPathNavigator> b(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    std::unique_ptr<XPathNavigator> c(SelectNav(doc->getDocumentElementProperty(), "book[2]"));
    EXPECT_TRUE(a->IsSamePosition(*b));
    EXPECT_FALSE(a->IsSamePosition(*c));
}

TEST(XPathNavigatorTests, ComparePosition_ReflectsDocumentOrder) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> first(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    std::unique_ptr<XPathNavigator> second(SelectNav(doc->getDocumentElementProperty(), "book[2]"));
    EXPECT_EQ(first->ComparePosition(second.get()), System::Xml::XmlNodeOrder::Before);
    EXPECT_EQ(second->ComparePosition(first.get()), System::Xml::XmlNodeOrder::After);
    EXPECT_EQ(first->ComparePosition(first.get()), System::Xml::XmlNodeOrder::Same);
}

TEST(XPathNavigatorTests, IsDescendant_TrueForNestedNode) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> root(doc->CreateNavigator());
    std::unique_ptr<XPathNavigator> title(SelectNav(doc->getDocumentElementProperty(), "book[1]/title"));
    EXPECT_TRUE(root->IsDescendant(title.get()));
    EXPECT_FALSE(title->IsDescendant(root.get()));
}

// ===========================================================================
// Select(): axes, wildcards, self/parent, predicates
// ===========================================================================

TEST(XPathSelectTests, ChildAxis_SimplePath) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("catalog/book"));
    EXPECT_EQ(it->getCountProperty(), 3);
}

TEST(XPathSelectTests, DescendantAxis_DoubleSlash_FindsAllTitles) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("//title"));
    ASSERT_EQ(it->getCountProperty(), 3);
    ASSERT_TRUE(it->MoveNext());
    EXPECT_EQ(it->getCurrentProperty()->getValueProperty(), "XML Developer's Guide");
}

TEST(XPathSelectTests, AttributeAxis_AtName_And_PathThenAt) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->getDocumentElementProperty()->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> ids(nav->Select("book/@id"));
    ASSERT_EQ(ids->getCountProperty(), 3);
    ASSERT_TRUE(ids->MoveNext());
    EXPECT_EQ(ids->getCurrentProperty()->getValueProperty(), "bk101");
}

TEST(XPathSelectTests, Wildcard_Star_MatchesAllChildElements) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> book(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    std::unique_ptr<XPathNodeIterator> it(book->Select("*"));
    EXPECT_EQ(it->getCountProperty(), 3); // title, author, price
}

TEST(XPathSelectTests, Wildcard_AtStar_MatchesAllAttributes) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> book(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    std::unique_ptr<XPathNodeIterator> it(book->Select("@*"));
    EXPECT_EQ(it->getCountProperty(), 2); // id, available
}

TEST(XPathSelectTests, SelfStep_Dot_ReturnsCurrentNodeOnly) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> book(SelectNav(doc->getDocumentElementProperty(), "book[1]"));
    std::unique_ptr<XPathNodeIterator> it(book->Select("."));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_TRUE(it->getCurrentProperty()->IsSamePosition(*book));
}

TEST(XPathSelectTests, ParentStep_DotDot_ReturnsParentElement) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> title(SelectNav(doc->getDocumentElementProperty(), "book[1]/title"));
    std::unique_ptr<XPathNodeIterator> it(title->Select(".."));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->getNameProperty(), "book");
}

TEST(XPathSelectTests, PositionalPredicate_SelectsNthPerParent) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    // //title should mean "any title anywhere" but [1] on a per-step positional predicate over
    // book/title (each book has exactly one title) still exercises real per-context-node
    // positional semantics via the deeper case below.
    std::unique_ptr<XPathNodeIterator> it(nav->Select("catalog/book[2]"));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk102");
}

TEST(XPathSelectTests, PositionalPredicate_LastFunction) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("catalog/book[last()]"));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk103");
}

TEST(XPathSelectTests, EqualityPredicate_OnAttribute) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("catalog/book[@id='bk102']"));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    // SelectSingleNode returns a caller-owned XPathNavigator (an internal Clone()) -- wrap it
    // instead of using it inline, confirmed as a real AddressSanitizer-caught leak otherwise
    // (2026-07-14).
    std::unique_ptr<XPathNavigator> title(it->getCurrentProperty()->SelectSingleNode("title"));
    EXPECT_EQ(title->getValueProperty(), "Midnight Rain");
}

TEST(XPathSelectTests, EqualityPredicate_OnChildElementValue) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("catalog/book[price='5.95']"));
    EXPECT_EQ(it->getCountProperty(), 2);
}

TEST(XPathSelectTests, DescendantThenPredicate_NestedPerContextSemantics) {
    // Regression-style check that predicates apply per-context-node (proximity position within
    // each parent's own child list), not against the globally merged node-set.
    XmlDocument doc;
    auto* root = doc.CreateElement("root");
    doc.AppendChild(root);
    for (int g = 0; g < 2; ++g) {
        auto* group = doc.CreateElement("group");
        root->AppendChild(group);
        for (int i = 0; i < 3; ++i) {
            auto* item = doc.CreateElement("item");
            item->setInnerTextProperty("g" + std::to_string(g) + "-" + std::to_string(i));
            group->AppendChild(item);
        }
    }
    std::unique_ptr<XPathNavigator> nav(root->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("//group/item[1]"));
    ASSERT_EQ(it->getCountProperty(), 2); // first item of EACH group, not just the first overall
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->getValueProperty(), "g0-0");
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->getValueProperty(), "g1-0");
}

// ===========================================================================
// Node-type tests: text()/comment()/processing-instruction()/node()
// ===========================================================================

TEST(XPathSelectTests, TextNodeTest_SelectsTextChildren) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> title(SelectNav(doc->getDocumentElementProperty(), "book[1]/title"));
    std::unique_ptr<XPathNodeIterator> it(title->Select("text()"));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->getValueProperty(), "XML Developer's Guide");
}

TEST(XPathSelectTests, CommentNodeTest_SelectsComments) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("//comment()"));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->getValueProperty(), " a comment node ");
}

TEST(XPathSelectTests, ProcessingInstructionNodeTest_SelectsPI) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("//processing-instruction()"));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->getNameProperty(), "target");
}

TEST(XPathSelectTests, NodeNodeTest_MatchesAllChildNodeKinds) {
    XmlDocument doc;
    auto* el = doc.CreateElement("mix");
    doc.AppendChild(el);
    el->AppendChild(doc.CreateTextNode("text"));
    el->AppendChild(doc.CreateComment("c"));
    el->AppendChild(doc.CreateElement("child"));
    std::unique_ptr<XPathNavigator> nav(el->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("node()"));
    EXPECT_EQ(it->getCountProperty(), 3);
}

// ===========================================================================
// SelectSingleNode / Evaluate
// ===========================================================================

TEST(XPathEvaluateTests, SelectSingleNode_ReturnsFirstMatch_NullWhenNone) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNavigator> found(nav->SelectSingleNode("//book[@id='bk103']/author"));
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getValueProperty(), "Corets, Eva");
    std::unique_ptr<XPathNavigator> notFound(nav->SelectSingleNode("//book[@id='does-not-exist']"));
    EXPECT_EQ(notFound, nullptr);
}

TEST(XPathEvaluateTests, Evaluate_CountFunction_ReturnsNumber) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    XPathEvaluateResult result = nav->Evaluate("count(//book)");
    ASSERT_EQ(result.getResultTypeProperty(), XPathResultType::Number);
    EXPECT_DOUBLE_EQ(result.getNumberProperty(), 3.0);
}

TEST(XPathEvaluateTests, Evaluate_StringExpression_ReturnsString) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    XPathEvaluateResult result = nav->Evaluate("concat('a', 'b', 'c')");
    ASSERT_EQ(result.getResultTypeProperty(), XPathResultType::String);
    EXPECT_EQ(result.getStringProperty(), "abc");
}

TEST(XPathEvaluateTests, Evaluate_BooleanExpression_ReturnsBoolean) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    XPathEvaluateResult result = nav->Evaluate("count(//book) > 2");
    ASSERT_EQ(result.getResultTypeProperty(), XPathResultType::Boolean);
    EXPECT_TRUE(result.getBooleanProperty());
}

TEST(XPathEvaluateTests, Evaluate_NodeSet_ReturnsIterator) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    XPathEvaluateResult result = nav->Evaluate("//book");
    ASSERT_EQ(result.getResultTypeProperty(), XPathResultType::NodeSet);
    std::unique_ptr<XPathNodeIterator> it(result.releaseNodeIterator());
    ASSERT_NE(it, nullptr);
    EXPECT_EQ(it->getCountProperty(), 3);
}

// ===========================================================================
// Functions
// ===========================================================================

TEST(XPathFunctionTests, NameLocalNameFunctions) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_EQ(nav->Evaluate("name(//ns:special)").getStringProperty(), "ns:special");
    EXPECT_EQ(nav->Evaluate("local-name(//ns:special)").getStringProperty(), "special");
}

TEST(XPathFunctionTests, PositionFunction_InPredicate) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("catalog/book[position()=2]"));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk102");
}

TEST(XPathFunctionTests, NotBooleanTrueFalseFunctions) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_TRUE(nav->Evaluate("not(false())").getBooleanProperty());
    EXPECT_FALSE(nav->Evaluate("boolean(0)").getBooleanProperty());
    EXPECT_TRUE(nav->Evaluate("true()").getBooleanProperty());
}

TEST(XPathFunctionTests, StringNumberFunctions) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_EQ(nav->Evaluate("string(42)").getStringProperty(), "42");
    EXPECT_DOUBLE_EQ(nav->Evaluate("number('3.5')").getNumberProperty(), 3.5);
}

// Regression test for a wave-3 audit finding: number() accepted scientific/exponent
// notation (e.g. "1e2" -> 100), but XPath 1.0's Number production (section 3.4) is
// `Digits ('.' Digits?)? | '.' Digits` -- no exponent syntax at all. A string containing an
// exponent must evaluate to NaN, matching real XPath 1.0 (and .NET's XPathNavigator).
TEST(XPathFunctionTests, Number_ExponentNotation_YieldsNaN) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_TRUE(std::isnan(nav->Evaluate("number('1e2')").getNumberProperty()));
    EXPECT_TRUE(std::isnan(nav->Evaluate("number('1.5E-3')").getNumberProperty()));
}

// Regression test for a code-audit finding (ticket 236): std::from_chars(..., fixed) rejects
// exponent notation as intended, but the C++ standard still has it recognize "nan"/"inf"/
// "infinity" (case-insensitively) as valid floating-point spellings regardless of
// chars_format -- so number("Infinity") previously produced +Infinity instead of the NaN
// required by XPath 1.0's Number grammar, which has no such spellings at all.
TEST(XPathFunctionTests, Number_NanAndInfinitySpellings_YieldNaN) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_TRUE(std::isnan(nav->Evaluate("number('NaN')").getNumberProperty()));
    EXPECT_TRUE(std::isnan(nav->Evaluate("number('Infinity')").getNumberProperty()));
    EXPECT_TRUE(std::isnan(nav->Evaluate("number('-Infinity')").getNumberProperty()));
    EXPECT_TRUE(std::isnan(nav->Evaluate("number('inf')").getNumberProperty()));
}

// The leading-minus and leading-decimal-point forms that XPath 1.0's number() conversion (as
// opposed to the Number *token* production used for in-expression literals) does allow must
// keep working after the character-whitelist fix above.
TEST(XPathFunctionTests, Number_LeadingMinusAndDecimalPoint_StillParse) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_DOUBLE_EQ(nav->Evaluate("number('-3.5')").getNumberProperty(), -3.5);
    EXPECT_DOUBLE_EQ(nav->Evaluate("number('.5')").getNumberProperty(), 0.5);
}

TEST(XPathFunctionTests, Number_PlainDecimal_StillParsesCorrectly) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_DOUBLE_EQ(nav->Evaluate("number('123')").getNumberProperty(), 123.0);
    EXPECT_DOUBLE_EQ(nav->Evaluate("number('.5')").getNumberProperty(), 0.5);
    EXPECT_DOUBLE_EQ(nav->Evaluate("number('-2.25')").getNumberProperty(), -2.25);
}

TEST(XPathFunctionTests, StartsWithContainsFunctions) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_TRUE(nav->Evaluate("starts-with('XML Developer', 'XML')").getBooleanProperty());
    EXPECT_TRUE(nav->Evaluate("contains('XML Developer', 'Devel')").getBooleanProperty());
    EXPECT_FALSE(nav->Evaluate("contains('XML Developer', 'zzz')").getBooleanProperty());
}

TEST(XPathFunctionTests, StringLengthAndNormalizeSpace) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_DOUBLE_EQ(nav->Evaluate("string-length('hello')").getNumberProperty(), 5.0);
    EXPECT_EQ(nav->Evaluate("normalize-space('  a   b  c ')").getStringProperty(), "a b c");
}

// ===========================================================================
// Operators
// ===========================================================================

TEST(XPathOperatorTests, ArithmeticOperators) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_DOUBLE_EQ(nav->Evaluate("2 + 3").getNumberProperty(), 5.0);
    EXPECT_DOUBLE_EQ(nav->Evaluate("5 - 2").getNumberProperty(), 3.0);
    EXPECT_DOUBLE_EQ(nav->Evaluate("4 * 2").getNumberProperty(), 8.0);
    EXPECT_DOUBLE_EQ(nav->Evaluate("7 div 2").getNumberProperty(), 3.5);
    EXPECT_DOUBLE_EQ(nav->Evaluate("7 mod 2").getNumberProperty(), 1.0);
    EXPECT_DOUBLE_EQ(nav->Evaluate("-5").getNumberProperty(), -5.0);
}

TEST(XPathOperatorTests, RelationalAndEqualityOperators) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_TRUE(nav->Evaluate("1 < 2").getBooleanProperty());
    EXPECT_TRUE(nav->Evaluate("2 <= 2").getBooleanProperty());
    EXPECT_TRUE(nav->Evaluate("3 > 2").getBooleanProperty());
    EXPECT_TRUE(nav->Evaluate("2 >= 2").getBooleanProperty());
    EXPECT_TRUE(nav->Evaluate("2 = 2").getBooleanProperty());
    EXPECT_TRUE(nav->Evaluate("2 != 3").getBooleanProperty());
}

TEST(XPathOperatorTests, RelationalOperator_NodeSetVsString_ComparesNumerically) {
    // Regression: per XPath 1.0 Section 3.4, <, <=, >, >= always convert to numbers, even when
    // one side is a node-set compared against a string literal -- lexicographically "44.95" <
    // "9" (since '4' < '9'), but numerically 44.95 > 9. The previous implementation used
    // lexicographic string comparison here, so //book[price > '9'] incorrectly matched zero
    // books instead of the one (bk101, price 44.95) that is numerically greater than 9.
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("//book[price > '9']"));
    ASSERT_EQ(it->getCountProperty(), 1);
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk101");
}

TEST(XPathOperatorTests, RelationalOperator_NodeSetVsNodeSet_ComparesNumerically) {
    // Regression: node-set vs node-set relational comparison must also be numeric, not
    // lexicographic -- "44.95" > "5.95" lexicographically is false ('4' < '5'), but numerically
    // 44.95 > 5.95 is true.
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_TRUE(nav->Evaluate("//book[@id='bk101']/price > //book[@id='bk102']/price").getBooleanProperty());
}

TEST(XPathOperatorTests, OrAndOperators) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_TRUE(nav->Evaluate("true() or false()").getBooleanProperty());
    EXPECT_FALSE(nav->Evaluate("true() and false()").getBooleanProperty());
}

TEST(XPathOperatorTests, UnionOperator_CombinesAndDedupesInDocumentOrder) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("//book[@id='bk101'] | //book[@id='bk103'] | //book[@id='bk101']"));
    ASSERT_EQ(it->getCountProperty(), 2); // duplicate bk101 branch collapses
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk101");
    it->MoveNext();
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk103");
}

// ===========================================================================
// XPathExpression: Compile / Clone / Expression / AddSort
// ===========================================================================

TEST(XPathExpressionTests, Compile_ExposesOriginalExpressionString) {
    XPathExpression expr = XPathExpression::Compile("//book[@id='bk101']");
    EXPECT_EQ(expr.getExpressionProperty(), "//book[@id='bk101']");
}

TEST(XPathExpressionTests, ReturnType_InferredPerExpressionShape) {
    EXPECT_EQ(XPathExpression::Compile("//book").getReturnTypeProperty(), XPathResultType::NodeSet);
    EXPECT_EQ(XPathExpression::Compile("count(//book)").getReturnTypeProperty(), XPathResultType::Number);
    EXPECT_EQ(XPathExpression::Compile("'hello'").getReturnTypeProperty(), XPathResultType::String);
    EXPECT_EQ(XPathExpression::Compile("1 = 1").getReturnTypeProperty(), XPathResultType::Boolean);
}

TEST(XPathExpressionTests, Clone_ProducesIndependentUsableExpression) {
    auto doc = LoadCatalog();
    XPathExpression expr = XPathExpression::Compile("//book");
    XPathExpression cloned = expr.Clone();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select(cloned));
    EXPECT_EQ(it->getCountProperty(), 3);
}

TEST(XPathExpressionTests, AddSort_AppliedByNavigatorSelect_OrdersByStringSortKeyDescending) {
    auto doc = LoadCatalog();
    XPathExpression expr = XPathExpression::Compile("//book");
    expr.AddSort("@id", XmlSortOrder::Descending, XmlCaseOrder::None, "", XmlDataType::Text);
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select(expr));
    ASSERT_TRUE(it->MoveNext());
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk103");
    ASSERT_TRUE(it->MoveNext());
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk102");
    ASSERT_TRUE(it->MoveNext());
    EXPECT_EQ(it->getCurrentProperty()->GetAttribute("id", ""), "bk101");
}

TEST(XPathExpressionTests, AddSort_NumericDataType_OrdersNumerically) {
    auto doc = LoadCatalog();
    XPathExpression expr = XPathExpression::Compile("//book");
    expr.AddSort("price", XmlSortOrder::Ascending, XmlCaseOrder::None, "", XmlDataType::Number);
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select(expr));
    ASSERT_TRUE(it->MoveNext());
    // See EqualityPredicate_OnAttribute above for why SelectSingleNode's result is wrapped.
    std::unique_ptr<XPathNavigator> price(it->getCurrentProperty()->SelectSingleNode("price"));
    EXPECT_DOUBLE_EQ(System::Xml::XmlConvert::ToDouble(price->getValueProperty()), 5.95);
}

// ===========================================================================
// XPathNodeIterator: Clone / Count / MoveNext independence
// ===========================================================================

TEST(XPathNodeIteratorTests, Clone_IsIndependentFromOriginalPosition) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("//book"));
    ASSERT_TRUE(it->MoveNext());
    ASSERT_TRUE(it->MoveNext());
    EXPECT_EQ(it->getCurrentPositionProperty(), 2);
    std::unique_ptr<XPathNodeIterator> clone(it->Clone());
    EXPECT_EQ(clone->getCurrentPositionProperty(), 2);
    ASSERT_TRUE(it->MoveNext());
    EXPECT_EQ(it->getCurrentPositionProperty(), 3);
    EXPECT_EQ(clone->getCurrentPositionProperty(), 2); // unaffected by the original's further advance
}

// ===========================================================================
// XPathDocument
// ===========================================================================

TEST(XPathDocumentTests, LoadXml_ThenCreateNavigator_CanSelect) {
    XPathDocument xdoc;
    xdoc.LoadXml(kCatalogXml);
    std::unique_ptr<XPathNavigator> nav(xdoc.CreateNavigator());
    std::unique_ptr<XPathNodeIterator> it(nav->Select("//book"));
    EXPECT_EQ(it->getCountProperty(), 3);
}

// ===========================================================================
// XmlNode::SelectSingleNode / SelectNodes wired for real
// ===========================================================================

TEST(XmlNodeXPathTests, SelectSingleNode_ReturnsXmlNode) {
    auto doc = LoadCatalog();
    XmlNode* found = doc->getDocumentElementProperty()->SelectSingleNode("book[@id='bk102']/title");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getInnerTextProperty(), "Midnight Rain");
}

TEST(XmlNodeXPathTests, SelectNodes_ReturnsAllMatchesAsXmlNodeList) {
    auto doc = LoadCatalog();
    // SelectNodes returns a caller-owned XmlNodeList -- delete it explicitly, confirmed as a
    // real AddressSanitizer-caught leak otherwise (2026-07-14).
    XmlNodeList* list = doc->getDocumentElementProperty()->SelectNodes("book");
    ASSERT_NE(list, nullptr);
    EXPECT_EQ(list->getCountProperty(), 3);
    delete list;
}

// ===========================================================================
// Error handling: unsupported syntax / bad input
// ===========================================================================

TEST(XPathErrorTests, Compile_UnknownFunction_Throws) {
    EXPECT_THROW(XPathExpression::Compile("bogus-function()"), XPathException);
}

TEST(XPathErrorTests, Compile_VariableReference_Throws) {
    EXPECT_THROW(XPathExpression::Compile("$x"), XPathException);
}

TEST(XPathErrorTests, Compile_ExplicitAxisSyntax_Throws) {
    EXPECT_THROW(XPathExpression::Compile("child::book"), XPathException);
}

TEST(XPathErrorTests, Compile_UnterminatedString_Throws) {
    EXPECT_THROW(XPathExpression::Compile("book[@id='bk101]"), XPathException);
}

// The hand-written recursive-descent parser recurses through ParseExpr() for '(', function-call
// arguments, and predicate '[', and through ParseUnary() for repeated unary minus. Without a
// depth guard, a deeply nested expression drives unbounded C++ call-stack recursion (a stack
// overflow crash, not a catchable exception). These confirm the guard throws XPathException well
// before the process could ever crash, for each of the three recursive shapes.
TEST(XPathErrorTests, Compile_DeeplyNestedParens_ThrowsInsteadOfCrashing) {
    std::string expr(600, '(');
    expr += "1";
    expr.append(600, ')');
    EXPECT_THROW(XPathExpression::Compile(expr), XPathException);
}

TEST(XPathErrorTests, Compile_DeeplyNestedUnaryMinus_ThrowsInsteadOfCrashing) {
    std::string expr(600, '-');
    expr += "1";
    EXPECT_THROW(XPathExpression::Compile(expr), XPathException);
}

TEST(XPathErrorTests, Compile_DeeplyNestedFunctionCalls_ThrowsInsteadOfCrashing) {
    std::string expr;
    for (int i = 0; i < 600; ++i) expr += "not(";
    expr += "1";
    for (int i = 0; i < 600; ++i) expr += ")";
    EXPECT_THROW(XPathExpression::Compile(expr), XPathException);
}

// A moderately-nested expression (well under the 500-level guard threshold) must still compile
// successfully -- the guard must not be so tight it rejects realistic, legitimate expressions.
TEST(XPathErrorTests, Compile_ModeratelyNestedParens_StillCompiles) {
    std::string expr(50, '(');
    expr += "1";
    expr.append(50, ')');
    EXPECT_NO_THROW(XPathExpression::Compile(expr));
}

TEST(XPathErrorTests, Select_NonNodeSetExpression_Throws) {
    auto doc = LoadCatalog();
    std::unique_ptr<XPathNavigator> nav(doc->CreateNavigator());
    EXPECT_THROW(nav->Select("1 + 1"), XPathException);
}
