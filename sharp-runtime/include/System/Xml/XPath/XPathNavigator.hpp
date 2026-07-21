// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/IXmlNamespaceResolver.hpp"
#include "System/Xml/XmlNamespaceScope.hpp"
#include "System/Xml/XmlNodeOrder.hpp"
#include "System/Xml/XPath/IXPathNavigable.hpp"
#include "System/Xml/XPath/XPathEvaluateResult.hpp"
#include "System/Xml/XPath/XPathExpression.hpp"
#include "System/Xml/XPath/XPathItem.hpp"
#include "System/Xml/XPath/XPathNamespaceScope.hpp"
#include "System/Xml/XPath/XPathNodeIterator.hpp"
#include "System/Xml/XPath/XPathNodeType.hpp"

namespace System::Xml::XPath {

    /**
     * @brief Provides a read-only cursor for navigating XML data per the XPath 1.0 data model,
     * and for compiling/evaluating a practical subset of XPath 1.0 expressions against it.
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathNavigator, scoped down to what this runtime
     * calls a "practical subset" (see CLAUDE.md's parity philosophy). Concretely:
     *
     * __Axes supported__: child (default), descendant-or-self (abbreviated `//` only — no
     * `descendant::`/`ancestor::`/`following::`/etc. explicit axis syntax), attribute (`@`),
     * self (`.`), parent (`..`). The `namespace::` axis is NOT reachable from a compiled
     * expression, but IS available directly via MoveToFirstNamespace()/MoveToNextNamespace().
     *
     * __Node tests supported__: `*`, `prefix:*`, `name`, `prefix:name` (prefix compared as a raw
     * string against the node's Prefix — this runtime does not resolve prefixes to namespace
     * URIs for XPath matching), `text()`, `comment()`, `processing-instruction()`,
     * `processing-instruction('target')`, `node()`.
     *
     * __Predicates supported__: any expression; `[N]` and `[last()]`-style numeric predicates
     * use XPath's proximity-position rule (a bare numeric result tests position() == N), any
     * other result is converted to boolean.
     *
     * __Operators supported__: `|` (node-set union), `or`, `and`, `=`, `!=`, `<`, `<=`, `>`,
     * `>=`, `+`, `-`, `*`, `div`, `mod`, unary `-`.
     *
     * __Functions supported__: `last()`, `position()`, `count(node-set)`, `name(node-set?)`,
     * `local-name(node-set?)`, `namespace-uri(node-set?)`, `not(boolean)`, `boolean(object)`,
     * `string(object?)`, `number(object?)`, `concat(string, string, string*)`,
     * `starts-with(string, string)`, `contains(string, string)`, `string-length(string?)`,
     * `normalize-space(string?)`, `true()`, `false()`.
     *
     * __NOT supported__ (throws XPathException at Compile() time, not a silent wrong answer):
     * explicit `axis::` syntax, variable references (`$x`), `substring()`/`substring-before()`/
     * `substring-after()`/`translate()`/`sum()`/`floor()`/`ceiling()`/`round()`/`lang()`/`id()`/
     * `key()`/`document()`, and general `FilterExpr '/' RelativeLocationPath` composition (e.g.
     * `(a|b)/c`, `id('x')/y`) — only a plain (possibly union-of-)LocationPath or a plain
     * non-path Expr is accepted, not a mix of the two.
     *
     * __NOT supported__ (omitted from the API surface entirely, not throwing stubs): the
     * editable-navigator API (CanEdit/PrependChild/AppendChild/InsertBefore/InsertAfter/
     * ReplaceSelf/DeleteSelf/OuterXml-setter/etc. from .NET 3.5's IXPathEditable extensions),
     * MoveTo(XPathNavigator)/MoveToId (cross-navigator position transplantation / ID lookup),
     * MoveToFollowing/MoveToChild/MoveToNext(name-or-type)/MoveToFirst/SelectChildren/
     * SelectAncestors/SelectDescendants/Matches/ReadSubtree/WriteSubtree/XmlLang/SchemaInfo/
     * CheckValidity/TypedValue/ValueType/XmlType/ValueAs*.
     *
     * SetContext(IXmlNamespaceResolver) on XPathExpression (for resolving namespace prefixes
     * used inside a compiled expression against an external context) is likewise not
     * implemented — see XPathExpression's class doc-comment.
     */
    class XPathNavigator : public XPathItem, public IXPathNavigable, public System::Xml::IXmlNamespaceResolver {
    public:
        ~XPathNavigator() override = default;

        [[nodiscard]] bool getIsNodeProperty() const override { return true; }

        // --- Position ------------------------------------------------------------------

        /** @return The XPath node type of the current position. */
        [[nodiscard]] virtual XPathNodeType getNodeTypeProperty() const = 0;
        /** @return The qualified name of the current node ("" for node types that have no name, e.g. text/comment/root). */
        [[nodiscard]] virtual std::string getNameProperty() const = 0;
        /** @return The local part of Name. */
        [[nodiscard]] virtual std::string getLocalNameProperty() const = 0;
        /** @return The namespace URI of the current node ("" if none/not applicable). */
        [[nodiscard]] virtual std::string getNamespaceURIProperty() const = 0;
        /** @return The namespace prefix of the current node ("" if none/not applicable). */
        [[nodiscard]] virtual std::string getPrefixProperty() const = 0;
        /** @return Always ""; this runtime does not track base URIs (matches XmlNode::getBaseURIProperty()). */
        [[nodiscard]] virtual std::string getBaseURIProperty() const = 0;
        /** @return true if the current node is an element with no children (see XmlElement::getIsEmptyProperty() for the same DOM-level approximation this reuses). */
        [[nodiscard]] virtual bool getIsEmptyElementProperty() const = 0;

        /** @return true if the current node has at least one attribute. Default: probes via MoveToFirstAttribute()/MoveToParent(). */
        [[nodiscard]] virtual bool getHasAttributesProperty() const;
        /** @return true if the current node has at least one child. Default: probes via MoveToFirstChild()/MoveToParent(). */
        [[nodiscard]] virtual bool getHasChildrenProperty() const;

        // --- Cloning ---------------------------------------------------------------------

        /** @return An independent copy of this navigator, positioned at the same point. Caller takes ownership. */
        [[nodiscard]] virtual XPathNavigator* Clone() const = 0;
        /** @brief IXPathNavigable::CreateNavigator(); returns Clone(). */
        [[nodiscard]] XPathNavigator* CreateNavigator() const override { return Clone(); }

        // --- Navigation ------------------------------------------------------------------

        /** @brief Moves to the root node of the document containing the current node. */
        virtual void MoveToRoot();
        /** @brief Moves to the parent of the current node. @return false (no move) if there is no parent. */
        [[nodiscard]] virtual bool MoveToParent() = 0;
        /** @brief Moves to the first child of the current node. @return false (no move) if there are no children. */
        [[nodiscard]] virtual bool MoveToFirstChild() = 0;
        /** @brief Moves to the next sibling of the current node. @return false (no move) if there is none. */
        [[nodiscard]] virtual bool MoveToNext() = 0;
        /** @brief Moves to the previous sibling of the current node. @return false (no move) if there is none. */
        [[nodiscard]] virtual bool MoveToPrevious() = 0;
        /** @brief Moves to the first attribute of the current node. @return false (no move) if there are none. */
        [[nodiscard]] virtual bool MoveToFirstAttribute() = 0;
        /** @brief Moves to the next attribute. @return false (no move) if there is none. */
        [[nodiscard]] virtual bool MoveToNextAttribute() = 0;
        /** @brief Moves to the attribute matching @p localName/@p namespaceURI. Default: linear scan via MoveToFirstAttribute()/MoveToNextAttribute(). @return false (no move) if not found. */
        [[nodiscard]] virtual bool MoveToAttribute(const std::string& localName, const std::string& namespaceURI);
        /** @brief Moves to the first namespace node in @p scope. @return false (no move) if there are none. */
        [[nodiscard]] virtual bool MoveToFirstNamespace(XPathNamespaceScope scope = XPathNamespaceScope::All) = 0;
        /** @brief Moves to the next namespace node in the same @p scope passed to MoveToFirstNamespace(). @return false (no move) if there is none. */
        [[nodiscard]] virtual bool MoveToNextNamespace(XPathNamespaceScope scope = XPathNamespaceScope::All) = 0;
        /** @brief Moves to the namespace node whose LocalName (prefix) is @p name. Default: linear scan over MoveToFirstNamespace(All)/MoveToNextNamespace(All). @return false (no move) if not found. */
        [[nodiscard]] virtual bool MoveToNamespace(const std::string& name);

        /** @return The value of attribute @p localName/@p namespaceURI, or "" if absent. Does not change the current position. */
        [[nodiscard]] virtual std::string GetAttribute(const std::string& localName, const std::string& namespaceURI);
        /** @return The URI of the in-scope namespace named @p name, or "" if absent. Does not change the current position. */
        [[nodiscard]] virtual std::string GetNamespace(const std::string& name);

        // --- Comparison --------------------------------------------------------------------

        /** @return true if @p other is positioned on the same node as this navigator. */
        [[nodiscard]] virtual bool IsSamePosition(const XPathNavigator& other) const = 0;
        /** @return true if @p nav is a (possibly indirect) descendant of this navigator's position. */
        [[nodiscard]] virtual bool IsDescendant(const XPathNavigator* nav) const;
        /** @return The document-order relationship between this navigator's position and @p nav's. */
        [[nodiscard]] virtual System::Xml::XmlNodeOrder ComparePosition(const XPathNavigator* nav) const;

        // --- IXmlNamespaceResolver -----------------------------------------------------------

        [[nodiscard]] std::optional<std::string> LookupNamespace(const std::string& prefix) const override;
        [[nodiscard]] std::optional<std::string> LookupPrefix(const std::string& namespaceURI) const override;
        [[nodiscard]] std::map<std::string, std::string> GetNamespacesInScope(System::Xml::XmlNamespaceScope scope) const override;

        // --- XPath compile / select / evaluate ------------------------------------------------

        /** @brief Compiles @p xpath into a reusable XPathExpression. @throws XPathException on a syntax error or use of an unsupported construct (see class doc-comment). */
        [[nodiscard]] virtual XPathExpression Compile(const std::string& xpath) const;
        /** @brief Compiles and evaluates @p xpath, positioned at this node. @return The first matching node, or nullptr. Caller takes ownership. */
        [[nodiscard]] XPathNavigator* SelectSingleNode(const std::string& xpath) const;
        /** @return The first node matched by @p expression, or nullptr. Caller takes ownership. */
        [[nodiscard]] XPathNavigator* SelectSingleNode(const XPathExpression& expression) const;
        /** @brief Compiles and evaluates @p xpath, positioned at this node. @return A node-set iterator over matches (any sort keys added to a pre-compiled XPathExpression are ignored here — compile once and use Select(XPathExpression) to apply sorting). Caller takes ownership. @throws XPathException if @p xpath does not evaluate to a node-set. */
        [[nodiscard]] XPathNodeIterator* Select(const std::string& xpath) const;
        /** @return A node-set iterator over the nodes matched by @p expr, in document order and sorted per any AddSort() keys. Caller takes ownership. @throws XPathException if @p expr does not evaluate to a node-set. */
        [[nodiscard]] XPathNodeIterator* Select(const XPathExpression& expr) const;
        /** @brief Compiles and evaluates @p xpath, positioned at this node. */
        [[nodiscard]] XPathEvaluateResult Evaluate(const std::string& xpath) const;
        /** @brief Evaluates @p expr, positioned at this node. */
        [[nodiscard]] XPathEvaluateResult Evaluate(const XPathExpression& expr) const;

        /** @return getValueProperty(). */
        [[nodiscard]] std::string ToString() const { return getValueProperty(); }

    protected:
        XPathNavigator() = default;
        XPathNavigator(const XPathNavigator&) = default;
    };

} // namespace System::Xml::XPath
