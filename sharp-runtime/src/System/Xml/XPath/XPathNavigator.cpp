// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XPath/XPathNavigator.hpp"

#include <algorithm>
#include <memory>

#include "System/Xml/XPath/XPathException.hpp"
#include "XPathAstInternal.hpp"

namespace System::Xml::XPath {

    // =======================================================================================
    // Simple defaults
    // =======================================================================================

    bool XPathNavigator::getHasAttributesProperty() const {
        std::unique_ptr<XPathNavigator> clone(Clone());
        return clone->MoveToFirstAttribute();
    }

    bool XPathNavigator::getHasChildrenProperty() const {
        std::unique_ptr<XPathNavigator> clone(Clone());
        return clone->MoveToFirstChild();
    }

    void XPathNavigator::MoveToRoot() {
        while (MoveToParent()) {}
    }

    bool XPathNavigator::MoveToAttribute(const std::string& localName, const std::string& namespaceURI) {
        if (MoveToFirstAttribute()) {
            do {
                if (localName == getLocalNameProperty() && namespaceURI == getNamespaceURIProperty()) return true;
            } while (MoveToNextAttribute());
            (void)MoveToParent();
        }
        return false;
    }

    bool XPathNavigator::MoveToNamespace(const std::string& name) {
        if (MoveToFirstNamespace(XPathNamespaceScope::All)) {
            do {
                if (name == getLocalNameProperty()) return true;
            } while (MoveToNextNamespace(XPathNamespaceScope::All));
            (void)MoveToParent();
        }
        return false;
    }

    std::string XPathNavigator::GetAttribute(const std::string& localName, const std::string& namespaceURI) {
        std::unique_ptr<XPathNavigator> clone(Clone());
        if (!clone->MoveToAttribute(localName, namespaceURI)) return {};
        return clone->getValueProperty();
    }

    std::string XPathNavigator::GetNamespace(const std::string& name) {
        std::unique_ptr<XPathNavigator> clone(Clone());
        if (!clone->MoveToNamespace(name)) {
            if (name == "xml") return "http://www.w3.org/XML/1998/namespace";
            if (name == "xmlns") return "http://www.w3.org/2000/xmlns/";
            return {};
        }
        return clone->getValueProperty();
    }

    // =======================================================================================
    // Comparison (ports .NET XPathNavigator's generic default ComparePosition/IsDescendant
    // algorithm, which works purely in terms of Clone/MoveToParent/IsSamePosition).
    // =======================================================================================

    bool XPathNavigator::IsDescendant(const XPathNavigator* nav) const {
        if (!nav) return false;
        std::unique_ptr<XPathNavigator> clone(nav->Clone());
        while (clone->MoveToParent())
            if (clone->IsSamePosition(*this)) return true;
        return false;
    }

    namespace {

        SharpRuntime::intcs GetDepth(XPathNavigator& nav) {
            SharpRuntime::intcs depth = 0;
            while (nav.MoveToParent()) ++depth;
            return depth;
        }

        // XPath-order comparison for namespaces, attributes, and other siblings sharing a parent.
        System::Xml::XmlNodeOrder CompareSiblings(XPathNavigator& n1, XPathNavigator& n2) {
            int cmp = 0;
            switch (n1.getNodeTypeProperty()) {
                case XPathNodeType::Namespace: break;
                case XPathNodeType::Attribute: cmp += 1; break;
                default: cmp += 2; break;
            }
            switch (n2.getNodeTypeProperty()) {
                case XPathNodeType::Namespace:
                    if (cmp == 0) {
                        while (n1.MoveToNextNamespace())
                            if (n1.IsSamePosition(n2)) return System::Xml::XmlNodeOrder::Before;
                    }
                    break;
                case XPathNodeType::Attribute:
                    cmp -= 1;
                    if (cmp == 0) {
                        while (n1.MoveToNextAttribute())
                            if (n1.IsSamePosition(n2)) return System::Xml::XmlNodeOrder::Before;
                    }
                    break;
                default:
                    cmp -= 2;
                    if (cmp == 0) {
                        while (n1.MoveToNext())
                            if (n1.IsSamePosition(n2)) return System::Xml::XmlNodeOrder::Before;
                    }
                    break;
            }
            return cmp < 0 ? System::Xml::XmlNodeOrder::Before : System::Xml::XmlNodeOrder::After;
        }

    } // namespace

    System::Xml::XmlNodeOrder XPathNavigator::ComparePosition(const XPathNavigator* nav) const {
        if (!nav) return System::Xml::XmlNodeOrder::Unknown;
        if (IsSamePosition(*nav)) return System::Xml::XmlNodeOrder::Same;

        std::unique_ptr<XPathNavigator> n1(Clone());
        std::unique_ptr<XPathNavigator> n2(nav->Clone());

        SharpRuntime::intcs depth1 = GetDepth(*std::unique_ptr<XPathNavigator>(n1->Clone()));
        SharpRuntime::intcs depth2 = GetDepth(*std::unique_ptr<XPathNavigator>(n2->Clone()));

        if (depth1 > depth2) {
            while (depth1 > depth2) { (void)n1->MoveToParent(); --depth1; }
            if (n1->IsSamePosition(*n2)) return System::Xml::XmlNodeOrder::After;
        }
        if (depth2 > depth1) {
            while (depth2 > depth1) { (void)n2->MoveToParent(); --depth2; }
            if (n1->IsSamePosition(*n2)) return System::Xml::XmlNodeOrder::Before;
        }

        std::unique_ptr<XPathNavigator> parent1(n1->Clone());
        std::unique_ptr<XPathNavigator> parent2(n2->Clone());
        for (;;) {
            if (!parent1->MoveToParent() || !parent2->MoveToParent()) return System::Xml::XmlNodeOrder::Unknown;
            if (parent1->IsSamePosition(*parent2)) return CompareSiblings(*n1, *n2);
            (void)n1->MoveToParent();
            (void)n2->MoveToParent();
        }
    }

    // =======================================================================================
    // IXmlNamespaceResolver
    // =======================================================================================

    std::optional<std::string> XPathNavigator::LookupNamespace(const std::string& prefix) const {
        std::unique_ptr<XPathNavigator> clone(Clone());
        if (clone->getNodeTypeProperty() != XPathNodeType::Element) {
            if (clone->MoveToParent()) return clone->LookupNamespace(prefix);
        } else if (clone->MoveToNamespace(prefix)) {
            return clone->getValueProperty();
        }
        if (prefix.empty()) return std::string();
        if (prefix == "xml") return std::string("http://www.w3.org/XML/1998/namespace");
        if (prefix == "xmlns") return std::string("http://www.w3.org/2000/xmlns/");
        return std::nullopt;
    }

    std::optional<std::string> XPathNavigator::LookupPrefix(const std::string& namespaceURI) const {
        std::unique_ptr<XPathNavigator> clone(Clone());
        if (clone->getNodeTypeProperty() != XPathNodeType::Element) {
            if (clone->MoveToParent()) return clone->LookupPrefix(namespaceURI);
        } else if (clone->MoveToFirstNamespace(XPathNamespaceScope::All)) {
            do {
                if (namespaceURI == clone->getValueProperty()) return clone->getLocalNameProperty();
            } while (clone->MoveToNextNamespace(XPathNamespaceScope::All));
        }
        std::optional<std::string> def = LookupNamespace("");
        if (def.has_value() && namespaceURI == *def) return std::string();
        if (namespaceURI == "http://www.w3.org/XML/1998/namespace") return std::string("xml");
        if (namespaceURI == "http://www.w3.org/2000/xmlns/") return std::string("xmlns");
        return std::nullopt;
    }

    std::map<std::string, std::string> XPathNavigator::GetNamespacesInScope(System::Xml::XmlNamespaceScope scope) const {
        // System::Xml::XmlNamespaceScope and System::Xml::XPath::XPathNamespaceScope declare the
        // same three members (All, ExcludeXml, Local) in the same order, matching real .NET's two
        // separate-but-identically-shaped enums; this cast relies on that.
        auto xpScope = static_cast<XPathNamespaceScope>(scope);

        std::unique_ptr<XPathNavigator> clone(Clone());
        XPathNodeType nt = clone->getNodeTypeProperty();
        if ((nt != XPathNodeType::Element && scope != System::Xml::XmlNamespaceScope::Local) ||
            nt == XPathNodeType::Attribute || nt == XPathNodeType::Namespace) {
            if (clone->MoveToParent()) return clone->GetNamespacesInScope(scope);
        }

        std::map<std::string, std::string> result;
        if (scope == System::Xml::XmlNamespaceScope::All) result["xml"] = "http://www.w3.org/XML/1998/namespace";
        if (clone->MoveToFirstNamespace(xpScope)) {
            do {
                std::string prefix = clone->getLocalNameProperty();
                std::string ns = clone->getValueProperty();
                if (!prefix.empty() || !ns.empty() || scope == System::Xml::XmlNamespaceScope::Local)
                    result[prefix] = ns;
            } while (clone->MoveToNextNamespace(xpScope));
        }
        return result;
    }

    // =======================================================================================
    // Compile / Select / Evaluate
    // =======================================================================================

    namespace {

        class XPathNodeSetIterator final : public XPathNodeIterator {
            Internal::NodeSetT nodes_;
            SharpRuntime::intcs index_ = -1;

        public:
            explicit XPathNodeSetIterator(Internal::NodeSetT nodes) : nodes_(std::move(nodes)) {}

            [[nodiscard]] XPathNodeIterator* Clone() const override {
                auto* c = new XPathNodeSetIterator(nodes_);
                c->index_ = index_;
                return c;
            }
            bool MoveNext() override {
                if (index_ + 1 >= static_cast<SharpRuntime::intcs>(nodes_.size())) return false;
                ++index_;
                return true;
            }
            [[nodiscard]] XPathNavigator* getCurrentProperty() const override {
                if (index_ < 0 || static_cast<size_t>(index_) >= nodes_.size()) return nullptr;
                return nodes_[static_cast<size_t>(index_)].get();
            }
            [[nodiscard]] SharpRuntime::intcs getCurrentPositionProperty() const override { return index_ + 1; }
            [[nodiscard]] SharpRuntime::intcs getCountProperty() const override {
                return static_cast<SharpRuntime::intcs>(nodes_.size());
            }
        };

        Internal::NodeSetT ApplySortKeys(Internal::NodeSetT nodes, const XPathExpression& expr) {
            const auto& keys = expr.getSortKeysForEvaluator();
            if (keys.empty() || nodes.size() < 2) return nodes;

            std::vector<std::shared_ptr<Internal::AstNode>> keyAsts;
            keyAsts.reserve(keys.size());
            for (auto& k : keys) keyAsts.push_back(Internal::ParseXPath(k.keyExpression));

            auto size = static_cast<SharpRuntime::intcs>(nodes.size());
            // Apply from last-added to first-added key so stable_sort makes the first-added key
            // the primary sort key (standard multi-key-stable-sort technique).
            for (size_t ki = keys.size(); ki-- > 0;) {
                const auto& spec = keys[ki];
                const std::shared_ptr<Internal::AstNode>& ast = keyAsts[ki];
                std::stable_sort(nodes.begin(), nodes.end(),
                                  [&](const std::shared_ptr<XPathNavigator>& a, const std::shared_ptr<XPathNavigator>& b) {
                                      Internal::EvalContext ca{a.get(), 1, size};
                                      Internal::EvalContext cb{b.get(), 1, size};
                                      Internal::XPathValue va = ast->Evaluate(ca);
                                      Internal::XPathValue vb = ast->Evaluate(cb);
                                      int cmp;
                                      if (spec.dataType == XmlDataType::Number) {
                                          double na = Internal::ToNumber(va), nb = Internal::ToNumber(vb);
                                          cmp = (na < nb) ? -1 : (na > nb ? 1 : 0);
                                      } else {
                                          cmp = Internal::ToStringValue(va).compare(Internal::ToStringValue(vb));
                                      }
                                      return spec.order == XmlSortOrder::Ascending ? (cmp < 0) : (cmp > 0);
                                  });
            }
            return nodes;
        }

    } // namespace

    XPathExpression XPathNavigator::Compile(const std::string& xpath) const {
        return XPathExpression::Compile(xpath);
    }

    XPathNodeIterator* XPathNavigator::Select(const std::string& xpath) const {
        return Select(Compile(xpath));
    }

    XPathNodeIterator* XPathNavigator::Select(const XPathExpression& expr) const {
        std::unique_ptr<XPathNavigator> ctxNav(Clone());
        Internal::EvalContext ctx{ctxNav.get(), 1, 1};
        Internal::XPathValue v = expr.getAstForEvaluator()->Evaluate(ctx);
        if (v.kind != XPathResultType::NodeSet)
            throw XPathException("XPath expression '" + expr.getExpressionProperty() + "' does not select a node-set.");
        return new XPathNodeSetIterator(ApplySortKeys(std::move(v.nodes), expr));
    }

    XPathNavigator* XPathNavigator::SelectSingleNode(const std::string& xpath) const {
        return SelectSingleNode(Compile(xpath));
    }

    XPathNavigator* XPathNavigator::SelectSingleNode(const XPathExpression& expression) const {
        std::unique_ptr<XPathNodeIterator> it(Select(expression));
        if (!it->MoveNext()) return nullptr;
        return it->getCurrentProperty()->Clone();
    }

    XPathEvaluateResult XPathNavigator::Evaluate(const std::string& xpath) const {
        return Evaluate(Compile(xpath));
    }

    XPathEvaluateResult XPathNavigator::Evaluate(const XPathExpression& expr) const {
        std::unique_ptr<XPathNavigator> ctxNav(Clone());
        Internal::EvalContext ctx{ctxNav.get(), 1, 1};
        Internal::XPathValue v = expr.getAstForEvaluator()->Evaluate(ctx);
        switch (v.kind) {
            case XPathResultType::Number: return XPathEvaluateResult::OfNumber(v.number);
            case XPathResultType::String: return XPathEvaluateResult::OfString(v.text);
            case XPathResultType::Boolean: return XPathEvaluateResult::OfBoolean(v.boolean);
            case XPathResultType::NodeSet:
                return XPathEvaluateResult::OfNodeSet(new XPathNodeSetIterator(ApplySortKeys(std::move(v.nodes), expr)));
            default:
                throw XPathException("Unexpected XPath evaluation result type.");
        }
    }

} // namespace System::Xml::XPath
