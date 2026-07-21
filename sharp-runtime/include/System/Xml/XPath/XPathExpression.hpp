// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/XPath/XPathResultType.hpp"
#include "System/Xml/XPath/XmlCaseOrder.hpp"
#include "System/Xml/XPath/XmlDataType.hpp"
#include "System/Xml/XPath/XmlSortOrder.hpp"

namespace System::Xml::XPath {

    namespace Internal { struct AstNode; }

    /**
     * @brief A compiled XPath expression, as returned by XPathNavigator::Compile().
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathExpression. Unlike real .NET (an abstract
     * class backed by an internal query-tree implementation), this is a concrete class: it owns
     * a hand-written AST produced by this runtime's own recursive-descent XPath 1.0 subset
     * parser (see the XPathNavigator class doc-comment for exactly which grammar constructs are
     * supported). Compiling is cheap to repeat, but pre-compiling and reusing an XPathExpression
     * (e.g. across many XPathNavigator::Select(XPathExpression) calls) avoids re-parsing.
     *
     * @note SetContext(IXmlNamespaceResolver)/SetContext(XmlNamespaceManager), present in real
     * .NET for resolving namespace prefixes used in the expression against an external context,
     * are not implemented: this runtime's parser/evaluator matches namespace-prefixed names
     * (e.g. "ns:tag") against the DOM node's raw Prefix string, not a resolved namespace URI.
     * Documented gap, not a silent one.
     */
    class XPathExpression {
        std::string expression_;
        std::shared_ptr<Internal::AstNode> ast_;

        struct SortKey {
            std::string keyExpression;
            XmlSortOrder order;
            XmlCaseOrder caseOrder;
            std::string lang;
            XmlDataType dataType;
        };
        std::vector<SortKey> sortKeys_;

        XPathExpression(std::string expression, std::shared_ptr<Internal::AstNode> ast);

    public:
        ~XPathExpression();
        XPathExpression(const XPathExpression&) = default;
        XPathExpression& operator=(const XPathExpression&) = default;

        /** @brief Parses @p xpath into a new XPathExpression. @throws XPathException on a syntax error or use of an unsupported construct. */
        [[nodiscard]] static XPathExpression Compile(const std::string& xpath);

        /** @return The original XPath source string this expression was compiled from. */
        [[nodiscard]] const std::string& getExpressionProperty() const { return expression_; }

        /** @return The statically-inferred result type of this expression (best-effort; see class doc-comment). */
        [[nodiscard]] XPathResultType getReturnTypeProperty() const;

        /**
         * @brief Adds a sort key applied (in the order added, most-recently-added is the lowest
         * priority) when this expression is evaluated via XPathNavigator::Select(). Applied for
         * real by this runtime's Select() (not a stored-but-ignored stub).
         * @param sortKeyXPath An XPath expression, evaluated relative to each selected node, that
         * produces the sort key for that node.
         */
        void AddSort(const std::string& sortKeyXPath, XmlSortOrder order, XmlCaseOrder caseOrder,
                     const std::string& lang, XmlDataType dataType);

        /** @return A copy of this compiled expression (sharing the immutable parsed AST). */
        [[nodiscard]] XPathExpression Clone() const;

        /** @return The internal AST root; for use by XPathNavigator's evaluator only. */
        [[nodiscard]] const std::shared_ptr<Internal::AstNode>& getAstForEvaluator() const { return ast_; }
        /** @return The sort keys added via AddSort, in insertion order; for use by XPathNavigator's evaluator only. */
        [[nodiscard]] const std::vector<SortKey>& getSortKeysForEvaluator() const { return sortKeys_; }
    };

} // namespace System::Xml::XPath
