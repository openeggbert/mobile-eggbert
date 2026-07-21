// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "XPathAstInternal.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <unordered_map>

#include "System/Diagnostics/UnreachableException.hpp"
#include "System/Double.hpp"
#include "System/Xml/XPath/XPathException.hpp"

using namespace System::Xml::XPath::Internal;

// ===========================================================================================
// Lexer / Parser (recursive descent over the subset grammar documented on XPathNavigator).
// ===========================================================================================

namespace System::Xml::XPath {
    namespace {

        enum class TokKind {
            End, Slash, SlashSlash, Dot, DotDot, At, Comma, LParen, RParen, LBracket, RBracket,
            Star, Pipe, Plus, Minus, Eq, Ne, Lt, Le, Gt, Ge, Colon, Name, Number, String,
        };

        struct Token {
            TokKind kind = TokKind::End;
            std::string text;
            double num = 0.0;
        };

        class Lexer {
            std::string src_;
            size_t pos_ = 0;

        public:
            explicit Lexer(std::string src) : src_(std::move(src)) {}

            Token Next() {
                SkipSpace();
                if (pos_ >= src_.size()) return {TokKind::End, "", 0};
                char c = src_[pos_];
                switch (c) {
                    case '/':
                        ++pos_;
                        if (pos_ < src_.size() && src_[pos_] == '/') { ++pos_; return {TokKind::SlashSlash, "//", 0}; }
                        return {TokKind::Slash, "/", 0};
                    case '.':
                        if (pos_ + 1 < src_.size() && src_[pos_ + 1] == '.') { pos_ += 2; return {TokKind::DotDot, "..", 0}; }
                        if (pos_ + 1 < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_ + 1]))) return LexNumber();
                        ++pos_; return {TokKind::Dot, ".", 0};
                    case '@': ++pos_; return {TokKind::At, "@", 0};
                    case ',': ++pos_; return {TokKind::Comma, ",", 0};
                    case '(': ++pos_; return {TokKind::LParen, "(", 0};
                    case ')': ++pos_; return {TokKind::RParen, ")", 0};
                    case '[': ++pos_; return {TokKind::LBracket, "[", 0};
                    case ']': ++pos_; return {TokKind::RBracket, "]", 0};
                    case '*': ++pos_; return {TokKind::Star, "*", 0};
                    case '|': ++pos_; return {TokKind::Pipe, "|", 0};
                    case '+': ++pos_; return {TokKind::Plus, "+", 0};
                    case '-': ++pos_; return {TokKind::Minus, "-", 0};
                    case '=': ++pos_; return {TokKind::Eq, "=", 0};
                    case ':': ++pos_; return {TokKind::Colon, ":", 0};
                    case '!':
                        if (pos_ + 1 < src_.size() && src_[pos_ + 1] == '=') { pos_ += 2; return {TokKind::Ne, "!=", 0}; }
                        throw XPathException("Unexpected '!' in XPath expression (expected '!=').");
                    case '<':
                        ++pos_;
                        if (pos_ < src_.size() && src_[pos_] == '=') { ++pos_; return {TokKind::Le, "<=", 0}; }
                        return {TokKind::Lt, "<", 0};
                    case '>':
                        ++pos_;
                        if (pos_ < src_.size() && src_[pos_] == '=') { ++pos_; return {TokKind::Ge, ">=", 0}; }
                        return {TokKind::Gt, ">", 0};
                    case '\'': case '"':
                        return LexString(c);
                    default:
                        if (std::isdigit(static_cast<unsigned char>(c))) return LexNumber();
                        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') return LexName();
                        throw XPathException(std::string("Unexpected character '") + c + "' in XPath expression.");
                }
            }

        private:
            void SkipSpace() { while (pos_ < src_.size() && std::isspace(static_cast<unsigned char>(src_[pos_]))) ++pos_; }

            Token LexNumber() {
                size_t start = pos_;
                while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) ++pos_;
                if (pos_ < src_.size() && src_[pos_] == '.') {
                    ++pos_;
                    while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) ++pos_;
                }
                std::string text = src_.substr(start, pos_ - start);
                Token t{TokKind::Number, text, 0};
                t.num = std::strtod(text.c_str(), nullptr);
                return t;
            }

            Token LexString(char quote) {
                ++pos_;
                size_t start = pos_;
                while (pos_ < src_.size() && src_[pos_] != quote) ++pos_;
                if (pos_ >= src_.size())
                    throw XPathException("Unterminated string literal in XPath expression.");
                std::string text = src_.substr(start, pos_ - start);
                ++pos_;
                return {TokKind::String, text, 0};
            }

            Token LexName() {
                size_t start = pos_;
                while (pos_ < src_.size() &&
                       (std::isalnum(static_cast<unsigned char>(src_[pos_])) || src_[pos_] == '_' ||
                        src_[pos_] == '-' || src_[pos_] == '.'))
                    ++pos_;
                return {TokKind::Name, src_.substr(start, pos_ - start), 0};
            }
        };

        struct FunctionSpec {
            int minArgs;
            int maxArgs; // -1 = unbounded
            XPathResultType returnType;
        };

        const std::unordered_map<std::string, FunctionSpec>& FunctionTable() {
            static const std::unordered_map<std::string, FunctionSpec> table = {
                {"last", {0, 0, XPathResultType::Number}},
                {"position", {0, 0, XPathResultType::Number}},
                {"count", {1, 1, XPathResultType::Number}},
                {"name", {0, 1, XPathResultType::String}},
                {"local-name", {0, 1, XPathResultType::String}},
                {"namespace-uri", {0, 1, XPathResultType::String}},
                {"not", {1, 1, XPathResultType::Boolean}},
                {"boolean", {1, 1, XPathResultType::Boolean}},
                {"string", {0, 1, XPathResultType::String}},
                {"number", {0, 1, XPathResultType::Number}},
                {"concat", {2, -1, XPathResultType::String}},
                {"starts-with", {2, 2, XPathResultType::Boolean}},
                {"contains", {2, 2, XPathResultType::Boolean}},
                {"string-length", {0, 1, XPathResultType::Number}},
                {"normalize-space", {0, 1, XPathResultType::String}},
                {"true", {0, 0, XPathResultType::Boolean}},
                {"false", {0, 0, XPathResultType::Boolean}},
            };
            return table;
        }

        bool IsReservedNodeTypeName(const std::string& s) {
            return s == "comment" || s == "text" || s == "processing-instruction" || s == "node";
        }

        // This is a hand-written recursive-descent parser with no inherent bound on nesting: '(',
        // function-call arguments, and predicate '[' all recurse back through ParseExpr(), and
        // repeated unary minus recurses through ParseUnary() directly. Without a depth guard, an
        // adversarial or accidentally-malformed input (e.g. thousands of nested parens, or a long
        // run of unary minuses) drives unbounded C++ call-stack recursion -- a stack overflow
        // (undefined behavior / hard crash), not a catchable XPathException. Real .NET's own
        // XPath engine is immune to this class of issue since it isn't a naive recursive-descent
        // parser over arbitrary user text the same way; this port's simpler grammar-subset parser
        // needed its own guard. kMaxParseDepth is generous for any realistic XPath expression
        // (deepest real-world expressions rarely exceed a few dozen levels) while still bounding
        // the C++ call stack well within its typical default limit.
        constexpr int kMaxParseDepth = 500;

        class Parser {
            Lexer lexer_;
            Token cur_;
            int depth_ = 0;

            struct DepthGuard {
                Parser& p;
                explicit DepthGuard(Parser& parser) : p(parser) {
                    if (++p.depth_ > kMaxParseDepth)
                        throw XPathException("XPath expression is too deeply nested.");
                }
                ~DepthGuard() { --p.depth_; }
            };

        public:
            explicit Parser(const std::string& src) : lexer_(src) { cur_ = lexer_.Next(); }

            std::shared_ptr<AstNode> ParseTop() {
                auto e = ParseExpr();
                if (cur_.kind != TokKind::End)
                    throw XPathException("Unexpected trailing content in XPath expression near '" + cur_.text + "'.");
                return e;
            }

        private:
            void Advance() { cur_ = lexer_.Next(); }
            [[nodiscard]] bool At(TokKind k) const { return cur_.kind == k; }
            void Expect(TokKind k, const char* what) {
                if (cur_.kind != k) throw XPathException(std::string("Expected ") + what + " in XPath expression.");
                Advance();
            }

            static std::shared_ptr<AstNode> MakeBin(BinOp op, std::shared_ptr<AstNode> l, std::shared_ptr<AstNode> r) {
                auto n = std::make_shared<AstNode>();
                n->kind = AstKind::BinaryOp;
                n->binOp = op;
                n->lhs = std::move(l);
                n->rhs = std::move(r);
                return n;
            }

            std::shared_ptr<AstNode> ParseExpr() { DepthGuard guard(*this); return ParseOr(); }

            std::shared_ptr<AstNode> ParseOr() {
                auto l = ParseAnd();
                while (At(TokKind::Name) && cur_.text == "or") { Advance(); l = MakeBin(BinOp::Or, l, ParseAnd()); }
                return l;
            }
            std::shared_ptr<AstNode> ParseAnd() {
                auto l = ParseEquality();
                while (At(TokKind::Name) && cur_.text == "and") { Advance(); l = MakeBin(BinOp::And, l, ParseEquality()); }
                return l;
            }
            std::shared_ptr<AstNode> ParseEquality() {
                auto l = ParseRelational();
                for (;;) {
                    if (At(TokKind::Eq)) { Advance(); l = MakeBin(BinOp::Eq, l, ParseRelational()); }
                    else if (At(TokKind::Ne)) { Advance(); l = MakeBin(BinOp::Ne, l, ParseRelational()); }
                    else break;
                }
                return l;
            }
            std::shared_ptr<AstNode> ParseRelational() {
                auto l = ParseAdditive();
                for (;;) {
                    if (At(TokKind::Lt)) { Advance(); l = MakeBin(BinOp::Lt, l, ParseAdditive()); }
                    else if (At(TokKind::Le)) { Advance(); l = MakeBin(BinOp::Le, l, ParseAdditive()); }
                    else if (At(TokKind::Gt)) { Advance(); l = MakeBin(BinOp::Gt, l, ParseAdditive()); }
                    else if (At(TokKind::Ge)) { Advance(); l = MakeBin(BinOp::Ge, l, ParseAdditive()); }
                    else break;
                }
                return l;
            }
            std::shared_ptr<AstNode> ParseAdditive() {
                auto l = ParseMultiplicative();
                for (;;) {
                    if (At(TokKind::Plus)) { Advance(); l = MakeBin(BinOp::Add, l, ParseMultiplicative()); }
                    else if (At(TokKind::Minus)) { Advance(); l = MakeBin(BinOp::Sub, l, ParseMultiplicative()); }
                    else break;
                }
                return l;
            }
            std::shared_ptr<AstNode> ParseMultiplicative() {
                auto l = ParseUnary();
                for (;;) {
                    if (At(TokKind::Star)) { Advance(); l = MakeBin(BinOp::Mul, l, ParseUnary()); }
                    else if (At(TokKind::Name) && cur_.text == "div") { Advance(); l = MakeBin(BinOp::Div, l, ParseUnary()); }
                    else if (At(TokKind::Name) && cur_.text == "mod") { Advance(); l = MakeBin(BinOp::Mod, l, ParseUnary()); }
                    else break;
                }
                return l;
            }
            std::shared_ptr<AstNode> ParseUnary() {
                if (At(TokKind::Minus)) {
                    DepthGuard guard(*this);
                    Advance();
                    auto n = std::make_shared<AstNode>();
                    n->kind = AstKind::UnaryMinus;
                    n->operand = ParseUnary();
                    return n;
                }
                return ParseUnion();
            }
            std::shared_ptr<AstNode> ParseUnion() {
                auto l = ParseValueExpr();
                while (At(TokKind::Pipe)) { Advance(); l = MakeBin(BinOp::Union, l, ParseValueExpr()); }
                return l;
            }

            bool LooksLikeLocationPathStart() {
                switch (cur_.kind) {
                    case TokKind::Slash: case TokKind::SlashSlash: case TokKind::Dot: case TokKind::DotDot:
                    case TokKind::At: case TokKind::Star:
                        return true;
                    case TokKind::Name: {
                        Lexer savedLexer = lexer_;
                        Token savedCur = cur_;
                        Advance();
                        bool followedByLParen = At(TokKind::LParen);
                        lexer_ = savedLexer;
                        cur_ = savedCur;
                        if (followedByLParen) return IsReservedNodeTypeName(savedCur.text);
                        return true;
                    }
                    default:
                        return false;
                }
            }

            std::shared_ptr<AstNode> ParseValueExpr() {
                if (LooksLikeLocationPathStart()) return ParseLocationPath();
                return ParsePrimaryExpr();
            }

            std::shared_ptr<AstNode> ParsePrimaryExpr() {
                if (At(TokKind::LParen)) {
                    Advance();
                    auto e = ParseExpr();
                    Expect(TokKind::RParen, "')'");
                    return e;
                }
                if (At(TokKind::String)) {
                    auto n = std::make_shared<AstNode>();
                    n->kind = AstKind::StringLiteral;
                    n->stringValue = cur_.text;
                    Advance();
                    return n;
                }
                if (At(TokKind::Number)) {
                    auto n = std::make_shared<AstNode>();
                    n->kind = AstKind::NumberLiteral;
                    n->numberValue = cur_.num;
                    Advance();
                    return n;
                }
                if (At(TokKind::Name)) return ParseFunctionCall();
                throw XPathException("Expected an expression near '" + cur_.text + "' in XPath expression.");
            }

            std::shared_ptr<AstNode> ParseFunctionCall() {
                std::string name = cur_.text;
                Advance();
                Expect(TokKind::LParen, "'('");
                auto n = std::make_shared<AstNode>();
                n->kind = AstKind::FunctionCall;
                n->functionName = name;
                if (!At(TokKind::RParen)) {
                    n->args.push_back(ParseExpr());
                    while (At(TokKind::Comma)) { Advance(); n->args.push_back(ParseExpr()); }
                }
                Expect(TokKind::RParen, "')'");
                ValidateFunctionCall(name, n->args.size());
                return n;
            }

            static void ValidateFunctionCall(const std::string& name, size_t argc) {
                auto it = FunctionTable().find(name);
                if (it == FunctionTable().end())
                    throw XPathException(
                        "Unsupported or unknown XPath function: " + name +
                        "() (this runtime implements a practical XPath 1.0 subset; see XPathNavigator's class doc-comment for the supported function list).");
                const FunctionSpec& spec = it->second;
                auto argcI = static_cast<int>(argc);
                if (argcI < spec.minArgs || (spec.maxArgs >= 0 && argcI > spec.maxArgs))
                    throw XPathException("Wrong number of arguments to XPath function " + name + "().");
            }

            static StepNode MakeDescendantOrSelfStep() {
                StepNode s;
                s.axis = Axis::DescendantOrSelf;
                s.test.kind = NodeTestKind::KindTest;
                s.test.kindType = KindTestType::AnyNode;
                return s;
            }

            [[nodiscard]] bool CanStartStep() const {
                switch (cur_.kind) {
                    case TokKind::Dot: case TokKind::DotDot: case TokKind::At: case TokKind::Star: case TokKind::Name:
                        return true;
                    default:
                        return false;
                }
            }

            std::shared_ptr<AstNode> ParseLocationPath() {
                auto n = std::make_shared<AstNode>();
                n->kind = AstKind::LocationPath;
                if (At(TokKind::SlashSlash)) {
                    Advance();
                    n->absolute = true;
                    n->steps.push_back(MakeDescendantOrSelfStep());
                    ParseRelativeLocationPathInto(n->steps);
                } else if (At(TokKind::Slash)) {
                    Advance();
                    n->absolute = true;
                    if (CanStartStep()) ParseRelativeLocationPathInto(n->steps);
                } else {
                    n->absolute = false;
                    ParseRelativeLocationPathInto(n->steps);
                }
                return n;
            }

            void ParseRelativeLocationPathInto(std::vector<StepNode>& steps) {
                ParseStepInto(steps);
                for (;;) {
                    if (At(TokKind::SlashSlash)) { Advance(); steps.push_back(MakeDescendantOrSelfStep()); ParseStepInto(steps); }
                    else if (At(TokKind::Slash)) { Advance(); ParseStepInto(steps); }
                    else break;
                }
            }

            void ParseStepInto(std::vector<StepNode>& steps) {
                if (At(TokKind::Dot)) {
                    Advance();
                    StepNode s;
                    s.axis = Axis::SelfNode;
                    s.test.kind = NodeTestKind::KindTest;
                    s.test.kindType = KindTestType::AnyNode;
                    steps.push_back(std::move(s));
                    return;
                }
                if (At(TokKind::DotDot)) {
                    Advance();
                    StepNode s;
                    s.axis = Axis::ParentNode;
                    s.test.kind = NodeTestKind::KindTest;
                    s.test.kindType = KindTestType::AnyNode;
                    steps.push_back(std::move(s));
                    return;
                }
                StepNode s;
                s.axis = Axis::Child;
                if (At(TokKind::At)) { Advance(); s.axis = Axis::Attribute; }
                s.test = ParseNodeTest();
                while (At(TokKind::LBracket)) {
                    Advance();
                    s.predicates.push_back(ParseExpr());
                    Expect(TokKind::RBracket, "']'");
                }
                steps.push_back(std::move(s));
            }

            NodeTest ParseNodeTest() {
                NodeTest t;
                if (At(TokKind::Star)) { Advance(); t.kind = NodeTestKind::NameTest; t.wildcardAll = true; return t; }
                if (!At(TokKind::Name))
                    throw XPathException("Expected a node test in XPath expression near '" + cur_.text + "'.");
                std::string text = cur_.text;
                Advance();
                if (At(TokKind::LParen) && IsReservedNodeTypeName(text)) {
                    Advance();
                    t.kind = NodeTestKind::KindTest;
                    if (text == "comment") t.kindType = KindTestType::Comment;
                    else if (text == "text") t.kindType = KindTestType::Text;
                    else if (text == "node") t.kindType = KindTestType::AnyNode;
                    else {
                        t.kindType = KindTestType::ProcessingInstruction;
                        if (At(TokKind::String)) { t.piTarget = cur_.text; t.piTargetSpecified = true; Advance(); }
                    }
                    Expect(TokKind::RParen, "')'");
                    return t;
                }
                t.kind = NodeTestKind::NameTest;
                if (At(TokKind::Colon)) {
                    Advance();
                    if (At(TokKind::Star)) { t.wildcardLocalOnly = true; t.prefix = text; Advance(); return t; }
                    if (!At(TokKind::Name)) throw XPathException("Expected a name after ':' in XPath expression.");
                    t.prefix = text;
                    t.localName = cur_.text;
                    Advance();
                    return t;
                }
                t.localName = text;
                return t;
            }
        };

    } // namespace
} // namespace System::Xml::XPath

std::shared_ptr<AstNode> System::Xml::XPath::Internal::ParseXPath(const std::string& xpath) {
    System::Xml::XPath::Parser p(xpath);
    return p.ParseTop();
}

// ===========================================================================================
// Value coercion (XPath 1.0 §3.4 / §4 conversion rules).
// ===========================================================================================

namespace System::Xml::XPath {
    namespace {

        double ParseXPathNumberLiteralString(const std::string& raw) {
            size_t b = raw.find_first_not_of(" \t\n\r");
            if (b == std::string::npos) return std::numeric_limits<double>::quiet_NaN();
            size_t e = raw.find_last_not_of(" \t\n\r");
            std::string s = raw.substr(b, e - b + 1);

            // XPath 1.0's number() string-to-number conversion (§4.2) is an optional leading
            // '-' followed by a Number per the §3.4 production `Digits ('.' Digits?)? | '.'
            // Digits` -- no exponent, no "inf"/"nan" spellings. std::from_chars with
            // chars_format::fixed rejects exponents (e.g. "1e2") as intended, but per the C++
            // standard it still recognizes "nan"/"inf"/"infinity" (case-insensitively, with an
            // optional sign) as valid floating-point spellings regardless of chars_format --
            // that restriction only governs the numeric-digit grammar, not these special
            // tokens. Left unchecked, number("Infinity") would silently produce +Infinity
            // instead of the NaN the spec requires for any string outside its Number grammar.
            // Reject any character outside [-0-9.] up front so those spellings can never reach
            // from_chars at all.
            for (char c : s) {
                if (!(std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == '-'))
                    return std::numeric_limits<double>::quiet_NaN();
            }

            double result = 0.0;
            const char* begin = s.data();
            const char* end = s.data() + s.size();
            auto [ptr, ec] = std::from_chars(begin, end, result, std::chars_format::fixed);
            if (ec != std::errc{} || ptr != end) return std::numeric_limits<double>::quiet_NaN();
            return result;
        }

        std::string FormatXPathNumber(double value) {
            if (std::isnan(value)) return "NaN";
            if (std::isinf(value)) return value > 0 ? "Infinity" : "-Infinity";
            if (value == 0.0) return "0";
            return System::Double::ToString(value);
        }

        std::string NormalizeSpace(const std::string& s) {
            std::string result;
            bool inSpace = true;
            for (char c : s) {
                bool isWs = (c == ' ' || c == '\t' || c == '\n' || c == '\r');
                if (isWs) {
                    if (!inSpace) { result += ' '; inSpace = true; }
                } else {
                    result += c;
                    inSpace = false;
                }
            }
            if (!result.empty() && result.back() == ' ') result.pop_back();
            return result;
        }

    } // namespace
} // namespace System::Xml::XPath

std::string System::Xml::XPath::Internal::NodeStringValue(XPathNavigator& nav) {
    return nav.getValueProperty();
}

double System::Xml::XPath::Internal::ToNumber(const XPathValue& v) {
    switch (v.kind) {
        case XPathResultType::Number:
            return v.number;
        case XPathResultType::Boolean:
            return v.boolean ? 1.0 : 0.0;
        case XPathResultType::NodeSet:
            if (v.nodes.empty()) return std::numeric_limits<double>::quiet_NaN();
            return ParseXPathNumberLiteralString(NodeStringValue(*v.nodes.front()));
        default: // String (and the unused Navigator/Any/Error tags)
            return ParseXPathNumberLiteralString(v.text);
    }
}

std::string System::Xml::XPath::Internal::ToStringValue(const XPathValue& v) {
    switch (v.kind) {
        case XPathResultType::String:
            return v.text;
        case XPathResultType::Boolean:
            return v.boolean ? "true" : "false";
        case XPathResultType::Number:
            return FormatXPathNumber(v.number);
        case XPathResultType::NodeSet:
            return v.nodes.empty() ? std::string() : NodeStringValue(*v.nodes.front());
        default:
            return {};
    }
}

bool System::Xml::XPath::Internal::ToBoolean(const XPathValue& v) {
    switch (v.kind) {
        case XPathResultType::Boolean:
            return v.boolean;
        case XPathResultType::Number:
            return v.number != 0.0 && !std::isnan(v.number);
        case XPathResultType::String:
            return !v.text.empty();
        case XPathResultType::NodeSet:
            return !v.nodes.empty();
        default:
            return false;
    }
}

void System::Xml::XPath::Internal::SortAndDedupe(NodeSetT& nodes) {
    std::stable_sort(nodes.begin(), nodes.end(),
                      [](const std::shared_ptr<XPathNavigator>& a, const std::shared_ptr<XPathNavigator>& b) {
                          return a->ComparePosition(b.get()) == System::Xml::XmlNodeOrder::Before;
                      });
    nodes.erase(std::unique(nodes.begin(), nodes.end(),
                             [](const std::shared_ptr<XPathNavigator>& a, const std::shared_ptr<XPathNavigator>& b) {
                                 return a->IsSamePosition(*b);
                             }),
                nodes.end());
}

bool System::Xml::XPath::Internal::NodeTestMatches(const NodeTest& test, Axis axis, XPathNavigator& nav) {
    XPathNodeType nt = nav.getNodeTypeProperty();
    if (test.kind == NodeTestKind::KindTest) {
        switch (test.kindType) {
            case KindTestType::AnyNode:
                return true;
            case KindTestType::Comment:
                return nt == XPathNodeType::Comment;
            case KindTestType::Text:
                return nt == XPathNodeType::Text || nt == XPathNodeType::SignificantWhitespace || nt == XPathNodeType::Whitespace;
            case KindTestType::ProcessingInstruction:
                if (nt != XPathNodeType::ProcessingInstruction) return false;
                return !test.piTargetSpecified || nav.getNameProperty() == test.piTarget;
        }
        return false; // unreachable
    }
    // NameTest: restricted to the axis's principal node type.
    XPathNodeType principal = (axis == Axis::Attribute) ? XPathNodeType::Attribute : XPathNodeType::Element;
    if (nt != principal) return false;
    if (test.wildcardAll) return true;
    if (test.wildcardLocalOnly) return nav.getPrefixProperty() == test.prefix;
    if (!test.prefix.empty()) return nav.getPrefixProperty() == test.prefix && nav.getLocalNameProperty() == test.localName;
    return nav.getPrefixProperty().empty() && nav.getLocalNameProperty() == test.localName;
}

// ===========================================================================================
// Axis / location-path evaluation.
// ===========================================================================================

namespace System::Xml::XPath {
    namespace {

        void CollectDescendantOrSelf(XPathNavigator& nav, const NodeTest& test, NodeSetT& out) {
            if (NodeTestMatches(test, Axis::DescendantOrSelf, nav))
                out.push_back(std::shared_ptr<XPathNavigator>(nav.Clone()));
            std::shared_ptr<XPathNavigator> child(nav.Clone());
            if (child->MoveToFirstChild()) {
                do {
                    CollectDescendantOrSelf(*child, test, out);
                } while (child->MoveToNext());
            }
        }

        NodeSetT EvaluateAxis(XPathNavigator& nav, Axis axis, const NodeTest& test) {
            NodeSetT result;
            switch (axis) {
                case Axis::Child: {
                    std::shared_ptr<XPathNavigator> cur(nav.Clone());
                    if (cur->MoveToFirstChild()) {
                        do {
                            if (NodeTestMatches(test, axis, *cur)) result.push_back(std::shared_ptr<XPathNavigator>(cur->Clone()));
                        } while (cur->MoveToNext());
                    }
                    break;
                }
                case Axis::Attribute: {
                    std::shared_ptr<XPathNavigator> cur(nav.Clone());
                    if (cur->MoveToFirstAttribute()) {
                        do {
                            if (NodeTestMatches(test, axis, *cur)) result.push_back(std::shared_ptr<XPathNavigator>(cur->Clone()));
                        } while (cur->MoveToNextAttribute());
                    }
                    break;
                }
                case Axis::SelfNode: {
                    std::shared_ptr<XPathNavigator> cur(nav.Clone());
                    if (NodeTestMatches(test, axis, *cur)) result.push_back(cur);
                    break;
                }
                case Axis::ParentNode: {
                    std::shared_ptr<XPathNavigator> cur(nav.Clone());
                    if (cur->MoveToParent() && NodeTestMatches(test, axis, *cur)) result.push_back(cur);
                    break;
                }
                case Axis::DescendantOrSelf: {
                    std::shared_ptr<XPathNavigator> self(nav.Clone());
                    CollectDescendantOrSelf(*self, test, result);
                    break;
                }
            }
            return result;
        }

        NodeSetT ApplyPredicate(const NodeSetT& nodes, const AstNode& pred) {
            NodeSetT result;
            auto size = static_cast<SharpRuntime::intcs>(nodes.size());
            for (SharpRuntime::intcs i = 0; i < size; ++i) {
                EvalContext innerCtx{nodes[static_cast<size_t>(i)].get(), i + 1, size};
                XPathValue v = pred.Evaluate(innerCtx);
                bool keep = (v.kind == XPathResultType::Number) ? (v.number == static_cast<double>(i + 1)) : ToBoolean(v);
                if (keep) result.push_back(nodes[static_cast<size_t>(i)]);
            }
            return result;
        }

        XPathValue EvaluateLocationPath(const AstNode& node, EvalContext& ctx) {
            NodeSetT current;
            std::shared_ptr<XPathNavigator> start(ctx.contextNode->Clone());
            if (node.absolute) start->MoveToRoot();
            current.push_back(start);
            for (const StepNode& step : node.steps) {
                NodeSetT next;
                for (auto& n : current) {
                    NodeSetT axisNodes = EvaluateAxis(*n, step.axis, step.test);
                    NodeSetT filtered = std::move(axisNodes);
                    for (auto& predAst : step.predicates) filtered = ApplyPredicate(filtered, *predAst);
                    for (auto& x : filtered) next.push_back(x);
                }
                SortAndDedupe(next);
                current = std::move(next);
            }
            return XPathValue::Nodes(std::move(current));
        }

        // ---- Comparisons (XPath 1.0 §3.4) -----------------------------------------------

        bool NumCompare(double a, double b, BinOp op) {
            switch (op) {
                case BinOp::Eq: return a == b;
                case BinOp::Ne: return a != b;
                case BinOp::Lt: return a < b;
                case BinOp::Le: return a <= b;
                case BinOp::Gt: return a > b;
                case BinOp::Ge: return a >= b;
                default: return false;
            }
        }
        bool StrCompare(const std::string& a, const std::string& b, BinOp op) {
            switch (op) {
                case BinOp::Eq: return a == b;
                case BinOp::Ne: return a != b;
                case BinOp::Lt: return a < b;
                case BinOp::Le: return a <= b;
                case BinOp::Gt: return a > b;
                case BinOp::Ge: return a >= b;
                default: return false;
            }
        }
        BinOp FlipOp(BinOp op) {
            switch (op) {
                case BinOp::Lt: return BinOp::Gt;
                case BinOp::Gt: return BinOp::Lt;
                case BinOp::Le: return BinOp::Ge;
                case BinOp::Ge: return BinOp::Le;
                default: return op; // Eq/Ne/others are symmetric or not applicable here
            }
        }

        bool CompareValues(const XPathValue& a, const XPathValue& b, BinOp op) {
            bool aNs = a.kind == XPathResultType::NodeSet;
            bool bNs = b.kind == XPathResultType::NodeSet;
            // Per XPath 1.0 Section 3.4: = and != between node-set-involving operands compare
            // string-values lexicographically, but <, <=, >, >= ALWAYS convert to numbers
            // first (the same "neither object is a node-set and the comparison is one of <,
            // <=, >, >=" numeric-conversion rule applies to the node-set cases too -- this is
            // a well-known, universally-implemented XPath behavior, not specific to any one
            // engine). Node-set-vs-boolean and node-set-vs-number below already did this
            // correctly (via NumCompare); node-set-vs-node-set and node-set-vs-string
            // previously used StrCompare unconditionally, silently producing lexicographic
            // instead of numeric results for relational operators, e.g. `@count > 9` where
            // @count="10" incorrectly evaluated string "10" < "9".
            bool numeric = !(op == BinOp::Eq || op == BinOp::Ne);
            if (aNs && bNs) {
                for (auto& na : a.nodes)
                    for (auto& nb : b.nodes) {
                        if (numeric) {
                            if (NumCompare(ParseXPathNumberLiteralString(NodeStringValue(*na)),
                                           ParseXPathNumberLiteralString(NodeStringValue(*nb)), op)) return true;
                        } else {
                            if (StrCompare(NodeStringValue(*na), NodeStringValue(*nb), op)) return true;
                        }
                    }
                return false;
            }
            if (aNs || bNs) {
                const XPathValue& ns = aNs ? a : b;
                const XPathValue& other = aNs ? b : a;
                BinOp effOp = aNs ? op : FlipOp(op);
                if (other.kind == XPathResultType::Boolean) {
                    return NumCompare(ToBoolean(ns) ? 1.0 : 0.0, other.boolean ? 1.0 : 0.0, effOp);
                }
                if (other.kind == XPathResultType::Number) {
                    for (auto& n : ns.nodes)
                        if (NumCompare(ParseXPathNumberLiteralString(NodeStringValue(*n)), other.number, effOp)) return true;
                    return false;
                }
                for (auto& n : ns.nodes) {
                    if (numeric) {
                        if (NumCompare(ParseXPathNumberLiteralString(NodeStringValue(*n)),
                                       ParseXPathNumberLiteralString(other.text), effOp)) return true;
                    } else {
                        if (StrCompare(NodeStringValue(*n), other.text, effOp)) return true;
                    }
                }
                return false;
            }
            if (op == BinOp::Eq || op == BinOp::Ne) {
                if (a.kind == XPathResultType::Boolean || b.kind == XPathResultType::Boolean) {
                    bool av = ToBoolean(a), bv = ToBoolean(b);
                    return op == BinOp::Eq ? av == bv : av != bv;
                }
                if (a.kind == XPathResultType::Number || b.kind == XPathResultType::Number) {
                    double av = ToNumber(a), bv = ToNumber(b);
                    return op == BinOp::Eq ? av == bv : av != bv;
                }
                std::string as = ToStringValue(a), bs = ToStringValue(b);
                return op == BinOp::Eq ? as == bs : as != bs;
            }
            return NumCompare(ToNumber(a), ToNumber(b), op);
        }

        XPathValue EvaluateBinaryOp(const AstNode& node, EvalContext& ctx) {
            switch (node.binOp) {
                case BinOp::Or:
                    return XPathValue::Bool(ToBoolean(node.lhs->Evaluate(ctx)) || ToBoolean(node.rhs->Evaluate(ctx)));
                case BinOp::And:
                    return XPathValue::Bool(ToBoolean(node.lhs->Evaluate(ctx)) && ToBoolean(node.rhs->Evaluate(ctx)));
                case BinOp::Eq: case BinOp::Ne: case BinOp::Lt: case BinOp::Le: case BinOp::Gt: case BinOp::Ge: {
                    XPathValue a = node.lhs->Evaluate(ctx);
                    XPathValue b = node.rhs->Evaluate(ctx);
                    return XPathValue::Bool(CompareValues(a, b, node.binOp));
                }
                case BinOp::Add:
                    return XPathValue::Num(ToNumber(node.lhs->Evaluate(ctx)) + ToNumber(node.rhs->Evaluate(ctx)));
                case BinOp::Sub:
                    return XPathValue::Num(ToNumber(node.lhs->Evaluate(ctx)) - ToNumber(node.rhs->Evaluate(ctx)));
                case BinOp::Mul:
                    return XPathValue::Num(ToNumber(node.lhs->Evaluate(ctx)) * ToNumber(node.rhs->Evaluate(ctx)));
                case BinOp::Div:
                    return XPathValue::Num(ToNumber(node.lhs->Evaluate(ctx)) / ToNumber(node.rhs->Evaluate(ctx)));
                case BinOp::Mod:
                    return XPathValue::Num(std::fmod(ToNumber(node.lhs->Evaluate(ctx)), ToNumber(node.rhs->Evaluate(ctx))));
                case BinOp::Union: {
                    XPathValue a = node.lhs->Evaluate(ctx);
                    XPathValue b = node.rhs->Evaluate(ctx);
                    if (a.kind != XPathResultType::NodeSet || b.kind != XPathResultType::NodeSet)
                        throw XPathException("The '|' operator requires both operands to be node-sets.");
                    NodeSetT combined = a.nodes;
                    combined.insert(combined.end(), b.nodes.begin(), b.nodes.end());
                    SortAndDedupe(combined);
                    return XPathValue::Nodes(std::move(combined));
                }
            }
            throw System::Diagnostics::UnreachableException("unreachable BinOp");
        }

        XPathValue EvaluateFunctionCall(const AstNode& node, EvalContext& ctx) {
            const std::string& n = node.functionName;
            auto arg = [&](size_t i) { return node.args[i]->Evaluate(ctx); };

            if (n == "last") return XPathValue::Num(static_cast<double>(ctx.size));
            if (n == "position") return XPathValue::Num(static_cast<double>(ctx.position));
            if (n == "count") {
                XPathValue v = arg(0);
                if (v.kind != XPathResultType::NodeSet) throw XPathException("count() requires a node-set argument.");
                return XPathValue::Num(static_cast<double>(v.nodes.size()));
            }
            if (n == "name" || n == "local-name" || n == "namespace-uri") {
                XPathNavigator* target = ctx.contextNode;
                // Declared in this outer scope (not just the `if` below) because `target` may
                // point into v.nodes[0]; if v were scoped to the inner block, it would be
                // destroyed (freeing that node, since it may be the last owning shared_ptr)
                // before target is dereferenced below.
                XPathValue v;
                if (!node.args.empty()) {
                    v = arg(0);
                    if (v.kind != XPathResultType::NodeSet) throw XPathException(n + "() requires a node-set argument.");
                    if (v.nodes.empty()) return XPathValue::Str("");
                    target = v.nodes.front().get();
                }
                if (n == "name") return XPathValue::Str(target->getNameProperty());
                if (n == "local-name") return XPathValue::Str(target->getLocalNameProperty());
                return XPathValue::Str(target->getNamespaceURIProperty());
            }
            if (n == "not") return XPathValue::Bool(!ToBoolean(arg(0)));
            if (n == "boolean") return XPathValue::Bool(ToBoolean(arg(0)));
            if (n == "true") return XPathValue::Bool(true);
            if (n == "false") return XPathValue::Bool(false);
            if (n == "string") {
                if (node.args.empty()) return XPathValue::Str(NodeStringValue(*ctx.contextNode));
                return XPathValue::Str(ToStringValue(arg(0)));
            }
            if (n == "number") {
                if (node.args.empty()) return XPathValue::Num(ParseXPathNumberLiteralString(NodeStringValue(*ctx.contextNode)));
                return XPathValue::Num(ToNumber(arg(0)));
            }
            if (n == "concat") {
                std::string result;
                for (auto& a : node.args) result += ToStringValue(a->Evaluate(ctx));
                return XPathValue::Str(result);
            }
            if (n == "starts-with") {
                std::string s = ToStringValue(arg(0)), p = ToStringValue(arg(1));
                return XPathValue::Bool(s.size() >= p.size() && s.compare(0, p.size(), p) == 0);
            }
            if (n == "contains") {
                std::string s = ToStringValue(arg(0)), p = ToStringValue(arg(1));
                return XPathValue::Bool(s.find(p) != std::string::npos);
            }
            if (n == "string-length") {
                std::string s = node.args.empty() ? NodeStringValue(*ctx.contextNode) : ToStringValue(arg(0));
                return XPathValue::Num(static_cast<double>(s.size()));
            }
            if (n == "normalize-space") {
                std::string s = node.args.empty() ? NodeStringValue(*ctx.contextNode) : ToStringValue(arg(0));
                return XPathValue::Str(NormalizeSpace(s));
            }
            throw XPathException("Unknown XPath function: " + n + "().");
        }

    } // namespace
} // namespace System::Xml::XPath

XPathValue System::Xml::XPath::Internal::AstNode::Evaluate(EvalContext& ctx) const {
    switch (kind) {
        case AstKind::NumberLiteral: return XPathValue::Num(numberValue);
        case AstKind::StringLiteral: return XPathValue::Str(stringValue);
        case AstKind::UnaryMinus: return XPathValue::Num(-System::Xml::XPath::Internal::ToNumber(operand->Evaluate(ctx)));
        case AstKind::BinaryOp: return System::Xml::XPath::EvaluateBinaryOp(*this, ctx);
        case AstKind::FunctionCall: return System::Xml::XPath::EvaluateFunctionCall(*this, ctx);
        case AstKind::LocationPath: return System::Xml::XPath::EvaluateLocationPath(*this, ctx);
    }
    throw System::Diagnostics::UnreachableException("unreachable AstKind");
}

System::Xml::XPath::XPathResultType System::Xml::XPath::Internal::AstNode::InferReturnType() const {
    switch (kind) {
        case AstKind::NumberLiteral: return XPathResultType::Number;
        case AstKind::StringLiteral: return XPathResultType::String;
        case AstKind::UnaryMinus: return XPathResultType::Number;
        case AstKind::LocationPath: return XPathResultType::NodeSet;
        case AstKind::BinaryOp:
            switch (binOp) {
                case BinOp::Or: case BinOp::And: case BinOp::Eq: case BinOp::Ne:
                case BinOp::Lt: case BinOp::Le: case BinOp::Gt: case BinOp::Ge:
                    return XPathResultType::Boolean;
                case BinOp::Add: case BinOp::Sub: case BinOp::Mul: case BinOp::Div: case BinOp::Mod:
                    return XPathResultType::Number;
                case BinOp::Union:
                    return XPathResultType::NodeSet;
            }
            throw System::Diagnostics::UnreachableException("unreachable BinOp");
        case AstKind::FunctionCall: {
            auto it = System::Xml::XPath::FunctionTable().find(functionName);
            return it != System::Xml::XPath::FunctionTable().end() ? it->second.returnType : XPathResultType::Any;
        }
    }
    throw System::Diagnostics::UnreachableException("unreachable AstKind");
}
