// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <vector>
#include "System/Xml/Linq/XName.hpp"
#include "System/Xml/Linq/XNode.hpp"

namespace System::Xml::Linq {

    class XElement;

    /**
     * @brief Represents a node that can contain other nodes. The two classes that derive from
     * XContainer are XElement and XDocument.
     *
     * C++ counterpart of .NET System.Xml.Linq.XContainer.
     *
     * @note Storage deviates from .NET's internal circular-linked-list representation on
     * purpose: children are kept in an ordered `std::vector<std::shared_ptr<XNode>>` instead.
     * Same public API/semantics, simpler C++ implementation (explicitly authorized — see NEXT.md).
     *
     * @note Re-adding a node that is already attached elsewhere in a tree *moves* it (detaches
     * it from its current parent, then attaches it here) rather than cloning it as real .NET
     * does. This is a deliberate, documented simplification: it avoids needing a full
     * deep-clone virtual dispatch across every node type, and "move" is arguably the more
     * useful behavior for a mutable in-memory game-data tree. Not applicable to attributes,
     * where the same move behavior is implemented directly on XElement.
     */
    class XContainer : public XNode {
        friend class XNode;

    protected:
        std::vector<std::shared_ptr<XNode>> children_;

        XContainer() = default;

        /** @brief Sets @p obj's owning container. Available to subclasses (e.g. XElement, for attributes). */
        static void AdoptObject(XObject& obj, XContainer* parent) { obj.parent_ = parent; }

        /**
         * @brief Structural validation hook run before a node is inserted, so subclasses
         * (XDocument, XElement) can enforce constraints (XDocument: single root element,
         * single doctype, no free text; XElement: an XDocument/XDocumentType may not be
         * added as a child element). Default: no restrictions.
         * @throws System::Exception (the specific type is subclass-defined -- XDocument
         * throws System::InvalidOperationException, XElement throws System::ArgumentException,
         * matching their respective .NET sources) if @p node may not be added to this container.
         */
        virtual void ValidateNode(const XNode& node) const { (void)node; }

        /** @brief Detaches @p n from this container (used by XNode::Remove() and internally). */
        void RemoveNode(XNode* n);

        /** @brief Inserts @p n at position @p index (0 = first child), adopting/moving it as Add() does. */
        void InsertNodeAt(size_t index, const std::shared_ptr<XNode>& n);

    public:
        /** @return The first child node, or nullptr if this container has no children. */
        [[nodiscard]] std::shared_ptr<XNode> getFirstNodeProperty() const { return children_.empty() ? nullptr : children_.front(); }
        /** @return The last child node, or nullptr if this container has no children. */
        [[nodiscard]] std::shared_ptr<XNode> getLastNodeProperty() const { return children_.empty() ? nullptr : children_.back(); }

        /**
         * @brief Adds @p node as the last child of this container.
         *
         * If @p node already has a parent, it is moved here (see class doc-comment).
         * A no-op if @p node is nullptr.
         */
        void Add(std::shared_ptr<XNode> node);
        /** @brief Adds each node in @p nodes as a child, in order. */
        void Add(const std::vector<std::shared_ptr<XNode>>& nodes);

        /** @brief Adds @p node as the first child of this container. A no-op if @p node is nullptr. */
        void AddFirst(std::shared_ptr<XNode> node);
        /** @brief Adds each node in @p nodes as the first children, in order. */
        void AddFirst(const std::vector<std::shared_ptr<XNode>>& nodes);

        /** @return All child nodes of this container, in document order. */
        [[nodiscard]] std::vector<std::shared_ptr<XNode>> Nodes() const { return children_; }

        /** @return All descendant nodes (children, grandchildren, ...; elements and leaf nodes), in document order. */
        [[nodiscard]] std::vector<std::shared_ptr<XNode>> DescendantNodes() const;

        /** @return The first child element named @p name, or nullptr. */
        [[nodiscard]] std::shared_ptr<XElement> Element(const XName& name) const;

        /** @return All child elements of this container, in document order. */
        [[nodiscard]] std::vector<std::shared_ptr<XElement>> Elements() const;
        /** @return All child elements of this container named @p name, in document order. */
        [[nodiscard]] std::vector<std::shared_ptr<XElement>> Elements(const XName& name) const;

        /** @return All descendant elements (not including this container itself), in document order. */
        [[nodiscard]] std::vector<std::shared_ptr<XElement>> Descendants() const;
        /** @return All descendant elements named @p name (not including this container itself), in document order. */
        [[nodiscard]] std::vector<std::shared_ptr<XElement>> Descendants(const XName& name) const;

        /** @brief Removes all child nodes of this container (does not remove attributes; see XElement::RemoveAttributes). */
        void RemoveNodes();

    protected:
        void CollectDescendantNodes(std::vector<std::shared_ptr<XNode>>& out) const;
        void CollectDescendants(const XName* nameFilter, std::vector<std::shared_ptr<XElement>>& out) const;
    };

} // namespace System::Xml::Linq
