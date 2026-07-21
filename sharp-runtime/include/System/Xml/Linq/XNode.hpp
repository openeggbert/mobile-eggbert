// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/Linq/SaveOptions.hpp"
#include "System/Xml/Linq/XObject.hpp"

namespace System::Xml {
    class XmlWriter;
}

namespace System::Xml::Linq {

    /**
     * @brief Represents nodes (elements, comments, document type, processing instruction,
     * and text nodes) in the XML tree. Note that an XAttribute is not an XNode (it derives
     * from XObject directly), matching .NET.
     *
     * C++ counterpart of .NET System.Xml.Linq.XNode.
     *
     * @note Internal representation deviates from .NET on purpose: real .NET stores a container's
     * children as a circular singly-linked list rooted at the last child (`XContainer.content` /
     * `XNode.next`). This port stores children as an ordered `std::vector<std::shared_ptr<XNode>>`
     * on XContainer instead (an explicitly authorized deviation — see the task notes / NEXT.md) —
     * same observable public API and document-order semantics, simpler C++ implementation.
     *
     * @note `CreateReader()` (building an XmlReader that streams this subtree) is out of scope —
     * this port's XmlReader is tinyxml2-backed and not wired to read directly from an in-memory
     * XNode tree (only from text/files). Not part of the ported surface.
     */
    class XNode : public XObject {
        friend class XContainer;

    protected:
        XNode() = default;

        /** @brief Escapes '&', '<', '>' for use in XML text content. Shared by XText and XElement's writer. */
        static std::string EscapeText(const std::string& s);
        /** @brief Escapes '&', '<', '>', '"' for use in a double-quoted XML attribute value. */
        static std::string EscapeAttributeValue(const std::string& s);

    public:
        /**
         * @brief Writes this node (and, for containers, its descendants) to @p os as XML text.
         * @param os Output stream.
         * @param depth Current nesting depth (used for indentation).
         * @param indent If true, pretty-print with newlines/indentation.
         *
         * C++-only implementation seam (not part of .NET's public surface; public rather than
         * protected because XElement/XDocument must invoke it polymorphically on arbitrary
         * sibling/child node instances, which protected access wouldn't allow through a
         * base-typed pointer) used by ToString()/Save() so that SaveOptions::DisableFormatting
         * can be honored exactly, independent of System::Xml::XmlWriter's fixed (always-indented)
         * tinyxml2-backed formatting.
         */
        virtual void SerializeTo(std::ostream& os, int depth, bool indent) const = 0;

        /**
         * @return The next sibling node of this node, or nullptr if this node has no parent or
         * is the last child.
         */
        [[nodiscard]] std::shared_ptr<XNode> getNextNodeProperty() const;

        /**
         * @return The previous sibling node of this node, or nullptr if this node has no parent
         * or is the first child.
         */
        [[nodiscard]] std::shared_ptr<XNode> getPreviousNodeProperty() const;

        /** @brief Removes this node from its parent container. @throws System::InvalidOperationException if this node has no parent. */
        void Remove();

        /** @brief Replaces this node with @p replacement (which may be nullptr, meaning "just remove this node"). @throws System::InvalidOperationException if this node has no parent. */
        void ReplaceWith(std::shared_ptr<XNode> replacement);

        /** @brief Replaces this node with @p replacements. @throws System::InvalidOperationException if this node has no parent. */
        void ReplaceWith(const std::vector<std::shared_ptr<XNode>>& replacements);

        /** @return The sibling nodes before this node, in document order. Empty if this node has no parent. */
        [[nodiscard]] std::vector<std::shared_ptr<XNode>> NodesBeforeSelf() const;

        /** @return The sibling nodes after this node, in document order. Empty if this node has no parent. */
        [[nodiscard]] std::vector<std::shared_ptr<XNode>> NodesAfterSelf() const;

        /**
         * @brief Compares two nodes to determine their relative XML document order.
         * @return 0 if equal, -1 if @p n1 is before @p n2, 1 if @p n1 is after @p n2.
         * @throws System::InvalidOperationException if the two nodes do not share a common ancestor.
         */
        [[nodiscard]] static SharpRuntime::intcs CompareDocumentOrder(const XNode* n1, const XNode* n2);

        /** @return true if this node appears after @p node in document order. */
        [[nodiscard]] bool IsAfter(const XNode* node) const { return CompareDocumentOrder(this, node) > 0; }
        /** @return true if this node appears before @p node in document order. */
        [[nodiscard]] bool IsBefore(const XNode* node) const { return CompareDocumentOrder(this, node) < 0; }

        /**
         * @brief Compares the values of two nodes, including the values of all descendant nodes.
         * A null node equals only another null node. Nodes of different NodeTypes are never equal.
         */
        [[nodiscard]] static bool DeepEquals(const XNode* n1, const XNode* n2);

        /** @return A value-based hash code for this node's content (see XNodeEqualityComparer). */
        [[nodiscard]] virtual SharpRuntime::intcs GetDeepHashCode() const = 0;

        /** @return The formatted XML text representation (SaveOptions::None — indented). */
        [[nodiscard]] std::string ToString() const { return ToString(SaveOptions::None); }

        /**
         * @return The XML text representation of this node.
         * @param options If SaveOptions::DisableFormatting is set, the output is not indented.
         * SaveOptions::OmitDuplicateNamespaces has no effect (this port does not implement namespace-declaration deduplication).
         */
        [[nodiscard]] std::string ToString(SaveOptions options) const;

        /** @brief Writes this node to @p writer. */
        virtual void WriteTo(System::Xml::XmlWriter& writer) const = 0;

    protected:
        /** @brief Compares this node's value against @p other's, assuming both share the same NodeType. */
        [[nodiscard]] virtual bool DeepEqualsCore(const XNode& other) const = 0;
    };

} // namespace System::Xml::Linq
