// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XNode.hpp"
#include <algorithm>
#include <sstream>
#include "System/InvalidOperationException.hpp"
#include "System/Xml/Linq/XContainer.hpp"

namespace System::Xml::Linq {

    std::string XNode::EscapeText(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    std::string XNode::EscapeAttributeValue(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    std::shared_ptr<XNode> XNode::getNextNodeProperty() const {
        if (parent_ == nullptr) return nullptr;
        const auto& siblings = parent_->children_;
        for (size_t i = 0; i < siblings.size(); ++i) {
            if (siblings[i].get() == this) {
                return (i + 1 < siblings.size()) ? siblings[i + 1] : nullptr;
            }
        }
        return nullptr;
    }

    std::shared_ptr<XNode> XNode::getPreviousNodeProperty() const {
        if (parent_ == nullptr) return nullptr;
        const auto& siblings = parent_->children_;
        for (size_t i = 0; i < siblings.size(); ++i) {
            if (siblings[i].get() == this) {
                return (i > 0) ? siblings[i - 1] : nullptr;
            }
        }
        return nullptr;
    }

    void XNode::Remove() {
        if (parent_ == nullptr) throw System::InvalidOperationException("The parent is missing.");
        parent_->RemoveNode(this);
    }

    void XNode::ReplaceWith(std::shared_ptr<XNode> replacement) {
        std::vector<std::shared_ptr<XNode>> v;
        if (replacement) v.push_back(std::move(replacement));
        ReplaceWith(v);
    }

    void XNode::ReplaceWith(const std::vector<std::shared_ptr<XNode>>& replacements) {
        if (parent_ == nullptr) throw System::InvalidOperationException("The parent is missing.");
        XContainer* c = parent_;
        auto& siblings = c->children_;
        auto it = std::find_if(siblings.begin(), siblings.end(),
                                [this](const std::shared_ptr<XNode>& n) { return n.get() == this; });
        size_t index = static_cast<size_t>(it - siblings.begin());
        c->RemoveNode(this);
        for (const auto& n : replacements) {
            if (!n) continue;
            c->InsertNodeAt(index, n);
            ++index;
        }
    }

    std::vector<std::shared_ptr<XNode>> XNode::NodesBeforeSelf() const {
        std::vector<std::shared_ptr<XNode>> result;
        if (parent_ == nullptr) return result;
        const auto& siblings = parent_->children_;
        for (const auto& n : siblings) {
            if (n.get() == this) break;
            result.push_back(n);
        }
        return result;
    }

    std::vector<std::shared_ptr<XNode>> XNode::NodesAfterSelf() const {
        std::vector<std::shared_ptr<XNode>> result;
        if (parent_ == nullptr) return result;
        const auto& siblings = parent_->children_;
        bool found = false;
        for (const auto& n : siblings) {
            if (found) result.push_back(n);
            else if (n.get() == this) found = true;
        }
        return result;
    }

    SharpRuntime::intcs XNode::CompareDocumentOrder(const XNode* n1, const XNode* n2) {
        if (n1 == n2) return 0;
        if (n1 == nullptr) return -1;
        if (n2 == nullptr) return 1;
        if (n1->parent_ != n2->parent_) {
            SharpRuntime::intcs height = 0;
            const XNode* p1 = n1;
            while (p1->parent_ != nullptr) { p1 = p1->parent_; ++height; }
            const XNode* p2 = n2;
            while (p2->parent_ != nullptr) { p2 = p2->parent_; --height; }
            if (p1 != static_cast<const XNode*>(p2)) {
                throw System::InvalidOperationException("A common ancestor is missing.");
            }
            if (height < 0) {
                do { n2 = n2->parent_; ++height; } while (height != 0);
                if (n1 == n2) return -1;
            } else if (height > 0) {
                do { n1 = n1->parent_; --height; } while (height != 0);
                if (n1 == n2) return 1;
            }
            while (n1->parent_ != n2->parent_) {
                n1 = n1->parent_;
                n2 = n2->parent_;
            }
        } else if (n1->parent_ == nullptr) {
            throw System::InvalidOperationException("A common ancestor is missing.");
        }
        const auto& siblings = n1->parent_->children_;
        for (const auto& n : siblings) {
            if (n.get() == n1) return -1;
            if (n.get() == n2) return 1;
        }
        return 0; // unreachable if the tree is consistent
    }

    bool XNode::DeepEquals(const XNode* n1, const XNode* n2) {
        if (n1 == n2) return true;
        if (n1 == nullptr || n2 == nullptr) return false;
        if (n1->getNodeTypeProperty() != n2->getNodeTypeProperty()) return false;
        return n1->DeepEqualsCore(*n2);
    }

    std::string XNode::ToString(SaveOptions options) const {
        std::ostringstream oss;
        bool indent = (options & SaveOptions::DisableFormatting) == SaveOptions::None;
        SerializeTo(oss, 0, indent);
        return oss.str();
    }

} // namespace System::Xml::Linq
