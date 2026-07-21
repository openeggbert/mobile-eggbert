// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Private implementation detail of the XPath subset parser/evaluator (System.Xml.XPath). Not a
// public header: included only by .cpp files under src/System/Xml/XPath/. Defines the AST node
// type forward-declared as System::Xml::XPath::Internal::AstNode in the public
// XPathExpression.hpp, plus the recursive-descent parser and tree-walking evaluator.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/XPath/XPathNavigator.hpp"
#include "System/Xml/XPath/XPathResultType.hpp"

namespace System::Xml::XPath::Internal {

    using NodeSetT = std::vector<std::shared_ptr<XPathNavigator>>;

    /** @brief A dynamically-typed XPath value: exactly one of the fields below is meaningful, per `kind`. */
    struct XPathValue {
        XPathResultType kind = XPathResultType::String;
        double number = 0.0;
        std::string text;
        bool boolean = false;
        NodeSetT nodes;

        static XPathValue Num(double v) { XPathValue r; r.kind = XPathResultType::Number; r.number = v; return r; }
        static XPathValue Str(std::string v) { XPathValue r; r.kind = XPathResultType::String; r.text = std::move(v); return r; }
        static XPathValue Bool(bool v) { XPathValue r; r.kind = XPathResultType::Boolean; r.boolean = v; return r; }
        static XPathValue Nodes(NodeSetT v) { XPathValue r; r.kind = XPathResultType::NodeSet; r.nodes = std::move(v); return r; }
    };

    /** @brief The (context node, proximity position, context size) triple used to evaluate position()/last() and predicate truth values. */
    struct EvalContext {
        XPathNavigator* contextNode;
        SharpRuntime::intcs position;
        SharpRuntime::intcs size;
    };

    // ---- Value coercion, per XPath 1.0 §3.4/§4 -------------------------------------------

    /** @return The XPath "string-value" of a single node (concatenated descendant text for element/root, own text/data for others). */
    std::string NodeStringValue(XPathNavigator& nav);
    double ToNumber(const XPathValue& v);
    std::string ToStringValue(const XPathValue& v);
    bool ToBoolean(const XPathValue& v);
    /** @brief Sorts @p nodes into document order and removes duplicates (by IsSamePosition), in place. */
    void SortAndDedupe(NodeSetT& nodes);

    // ---- Location path model --------------------------------------------------------------

    enum class Axis { Child, Attribute, SelfNode, ParentNode, DescendantOrSelf };

    enum class NodeTestKind { NameTest, KindTest };
    enum class KindTestType { Comment, Text, ProcessingInstruction, AnyNode };

    struct NodeTest {
        NodeTestKind kind = NodeTestKind::KindTest;
        // NameTest fields:
        bool wildcardAll = false;         // '*'
        bool wildcardLocalOnly = false;   // 'prefix:*'
        std::string prefix;
        std::string localName;
        // KindTest fields:
        KindTestType kindType = KindTestType::AnyNode;
        std::string piTarget;
        bool piTargetSpecified = false;
    };

    bool NodeTestMatches(const NodeTest& test, Axis axis, XPathNavigator& nav);

    struct AstNode;

    struct StepNode {
        Axis axis = Axis::Child;
        NodeTest test;
        std::vector<std::shared_ptr<AstNode>> predicates;
    };

    enum class AstKind { LocationPath, BinaryOp, UnaryMinus, NumberLiteral, StringLiteral, FunctionCall };
    enum class BinOp { Or, And, Eq, Ne, Lt, Le, Gt, Ge, Add, Sub, Mul, Div, Mod, Union };

    /**
     * @brief One node of the parsed XPath expression tree. A single tagged struct (rather than a
     * class hierarchy) covers every node kind for simplicity; only the fields relevant to `kind`
     * are populated.
     */
    struct AstNode {
        AstKind kind;

        // LocationPath
        bool absolute = false;
        std::vector<StepNode> steps;

        // BinaryOp
        BinOp binOp{};
        std::shared_ptr<AstNode> lhs, rhs;

        // UnaryMinus
        std::shared_ptr<AstNode> operand;

        // NumberLiteral
        double numberValue = 0.0;

        // StringLiteral
        std::string stringValue;

        // FunctionCall
        std::string functionName;
        std::vector<std::shared_ptr<AstNode>> args;

        [[nodiscard]] XPathValue Evaluate(EvalContext& ctx) const;
        [[nodiscard]] XPathResultType InferReturnType() const;
    };

    /** @brief Parses @p xpath per the subset grammar documented on XPathNavigator. @throws XPathException on syntax error or unsupported construct. */
    std::shared_ptr<AstNode> ParseXPath(const std::string& xpath);

} // namespace System::Xml::XPath::Internal
