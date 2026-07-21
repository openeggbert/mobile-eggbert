// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XContainer.hpp"
#include <algorithm>
#include "System/InvalidOperationException.hpp"
#include "System/Xml/Linq/XElement.hpp"

namespace System::Xml::Linq {

    using System::Xml::XmlNodeType;

    void XContainer::RemoveNode(XNode* n) {
        auto it = std::find_if(children_.begin(), children_.end(),
                                [n](const std::shared_ptr<XNode>& c) { return c.get() == n; });
        if (it == children_.end()) return;
        AdoptObject(**it, nullptr);
        children_.erase(it);
    }

    void XContainer::InsertNodeAt(size_t index, const std::shared_ptr<XNode>& n) {
        if (!n) return;
        ValidateNode(*n);
        // Verified against XContainer.cs's AddNode(): real .NET detects "n is this or an
        // ancestor of this" and clones n instead of inserting the original (Add() has
        // copy-on-attach semantics whenever the node being added is already attached
        // somewhere). This port instead reparents nodes in place (moves rather than clones) as
        // its established, simpler design for the common case, so replicating .NET's full
        // clone-based Add() would be a much larger behavioral change than this specific bug
        // warrants. This guards against the one genuinely broken outcome instead: inserting a
        // node into its own subtree, which previously created a permanent shared_ptr reference
        // cycle (the container ends up holding, transitively, a shared_ptr back to itself) and
        // a stack overflow in any recursive traversal (Nodes()/ToString()/etc.).
        for (XContainer* ancestorOrSelf = this; ancestorOrSelf; ancestorOrSelf = ancestorOrSelf->parent_) {
            if (ancestorOrSelf == n.get()) {
                throw System::InvalidOperationException("Cannot add a node as a child of itself or of one of its own descendants.");
            }
        }
        if (n->parent_ != nullptr) {
            n->parent_->RemoveNode(n.get());
        }
        AdoptObject(*n, this);
        index = std::min(index, children_.size());
        children_.insert(children_.begin() + static_cast<std::ptrdiff_t>(index), n);
    }

    void XContainer::Add(std::shared_ptr<XNode> node) {
        if (!node) return;
        InsertNodeAt(children_.size(), node);
    }

    void XContainer::Add(const std::vector<std::shared_ptr<XNode>>& nodes) {
        for (const auto& n : nodes) Add(n);
    }

    void XContainer::AddFirst(std::shared_ptr<XNode> node) {
        if (!node) return;
        InsertNodeAt(0, node);
    }

    void XContainer::AddFirst(const std::vector<std::shared_ptr<XNode>>& nodes) {
        size_t index = 0;
        for (const auto& n : nodes) {
            if (!n) continue;
            InsertNodeAt(index, n);
            ++index;
        }
    }

    std::vector<std::shared_ptr<XNode>> XContainer::DescendantNodes() const {
        std::vector<std::shared_ptr<XNode>> result;
        CollectDescendantNodes(result);
        return result;
    }

    void XContainer::CollectDescendantNodes(std::vector<std::shared_ptr<XNode>>& out) const {
        for (const auto& child : children_) {
            out.push_back(child);
            if (child->getNodeTypeProperty() == XmlNodeType::Element) {
                static_cast<XElement*>(child.get())->CollectDescendantNodes(out);
            }
        }
    }

    std::shared_ptr<XElement> XContainer::Element(const XName& name) const {
        for (const auto& child : children_) {
            if (child->getNodeTypeProperty() == XmlNodeType::Element) {
                auto e = std::static_pointer_cast<XElement>(child);
                if (e->getNameProperty() == name) return e;
            }
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<XElement>> XContainer::Elements() const {
        std::vector<std::shared_ptr<XElement>> result;
        for (const auto& child : children_) {
            if (child->getNodeTypeProperty() == XmlNodeType::Element) {
                result.push_back(std::static_pointer_cast<XElement>(child));
            }
        }
        return result;
    }

    std::vector<std::shared_ptr<XElement>> XContainer::Elements(const XName& name) const {
        std::vector<std::shared_ptr<XElement>> result;
        for (const auto& child : children_) {
            if (child->getNodeTypeProperty() == XmlNodeType::Element) {
                auto e = std::static_pointer_cast<XElement>(child);
                if (e->getNameProperty() == name) result.push_back(e);
            }
        }
        return result;
    }

    std::vector<std::shared_ptr<XElement>> XContainer::Descendants() const {
        std::vector<std::shared_ptr<XElement>> result;
        CollectDescendants(nullptr, result);
        return result;
    }

    std::vector<std::shared_ptr<XElement>> XContainer::Descendants(const XName& name) const {
        std::vector<std::shared_ptr<XElement>> result;
        CollectDescendants(&name, result);
        return result;
    }

    void XContainer::CollectDescendants(const XName* nameFilter, std::vector<std::shared_ptr<XElement>>& out) const {
        for (const auto& child : children_) {
            if (child->getNodeTypeProperty() == XmlNodeType::Element) {
                auto e = std::static_pointer_cast<XElement>(child);
                if (nameFilter == nullptr || e->getNameProperty() == *nameFilter) out.push_back(e);
                e->CollectDescendants(nameFilter, out);
            }
        }
    }

    void XContainer::RemoveNodes() {
        for (const auto& child : children_) {
            AdoptObject(*child, nullptr);
        }
        children_.clear();
    }

} // namespace System::Xml::Linq
