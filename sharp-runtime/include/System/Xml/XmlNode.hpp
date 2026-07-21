// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/XPath/IXPathNavigable.hpp"
#include "System/Xml/XmlNodeList.hpp"
#include "System/Xml/XmlNodeType.hpp"

namespace tinyxml2 { class XMLNode; }

namespace System::Xml {

    class XmlDocument;
    class XmlAttributeCollection;
    class XmlElement;
    class XmlWriter;

    /**
     * @brief Represents a single node in an XML document; the abstract base of the classic
     * XmlDocument DOM API.
     *
     * C++ counterpart of .NET System.Xml.XmlNode. Backed by a tinyxml2::XMLNode* owned by the
     * tinyxml2::XMLDocument inside this node's XmlDocument; node identity is stable because
     * XmlDocument caches one wrapper per native pointer (repeated navigation calls, e.g.
     * ParentNode() twice, return the same XmlNode*).
     *
     * @note Simplifications (practical-subset scope, documented rather than silent):
     * - NamespaceURI resolves "xmlns"/"xmlns:prefix" by walking ancestor elements; it does not
     *   implement the full DOM namespace-declaration algorithm (e.g. namespace undeclaration).
     * - BaseURI always returns "" (no base-URI tracking).
     * - IsReadOnly always returns false (no DTD-driven read-only nodes).
     * - ChildNodes returns a materialized snapshot list, not a live view.
     */
    class XmlNode : public XPath::IXPathNavigable {
    protected:
        tinyxml2::XMLNode* native_ = nullptr;
        XmlDocument* ownerDocument_ = nullptr;
        mutable std::unique_ptr<XmlNodeList> childNodesSnapshot_;

        XmlNode() = default;
        XmlNode(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : native_(native), ownerDocument_(ownerDocument) {}

        /**
         * @brief Resolves the document to use for node-wrapping/cache operations (WrapNode,
         * PurgeCache, DetachNode, IsDetached, getNativeDocument).
         *
         * C++ counterpart of .NET XmlNode's internal `Document` property: returns
         * ownerDocument_ for ordinary nodes, or `this` when this node IS the XmlDocument, whose
         * OwnerDocument is always nullptr (matching .NET's documented XmlDocument.OwnerDocument
         * == null contract). Every internal use of the owning document for node operations must
         * go through this, not ownerDocument_ directly, or those operations silently no-op when
         * called on the XmlDocument object itself.
         */
        [[nodiscard]] virtual XmlDocument* GetDocument() const { return ownerDocument_; }

        /**
         * @brief Removes only this node's children, never attributes.
         *
         * C++ counterpart of .NET XmlNode's internal RemoveAllChildren() (distinct from the
         * public, virtual RemoveAll(), which XmlElement overrides to also clear attributes).
         * InnerText/InnerXml setters must call this, not RemoveAll(), so that setting InnerText
         * on an XmlElement does not silently wipe its attributes too.
         */
        void RemoveAllChildren();

    public:
        virtual ~XmlNode();

        /** @return The underlying tinyxml2 node; for interop with code that needs direct tinyxml2 access. */
        [[nodiscard]] tinyxml2::XMLNode* getNativeNode() const { return native_; }

        /** @return The type of this node. */
        [[nodiscard]] virtual XmlNodeType getNodeTypeProperty() const = 0;

        /** @return The qualified name of the node (meaning varies per node type). */
        [[nodiscard]] virtual std::string getNameProperty() const;
        /** @return The part of Name after the last ':', or all of Name if there is none. */
        [[nodiscard]] virtual std::string getLocalNameProperty() const;
        /** @return The namespace URI in scope for this node's prefix, or "" if none. */
        [[nodiscard]] virtual std::string getNamespaceURIProperty() const;
        /** @return The part of Name before the last ':', or "" if there is none. */
        [[nodiscard]] virtual std::string getPrefixProperty() const;

        /** @return The text value of the node, or "" for node types that have no value. */
        [[nodiscard]] virtual std::string getValueProperty() const;
        /** @brief Sets the text value of the node. @throws System::InvalidOperationException for node types that have no settable value. */
        virtual void setValueProperty(const std::string& value);

        /** @return The concatenated text content of this node and all its descendants. */
        [[nodiscard]] virtual std::string getInnerTextProperty() const;
        /** @brief Replaces this node's children with a single text node containing @p text. */
        virtual void setInnerTextProperty(const std::string& text);

        /** @return The XML markup of this node's children (not including this node itself). */
        [[nodiscard]] virtual std::string getInnerXmlProperty() const;
        /** @brief Replaces this node's children by parsing @p xml as a sequence of child nodes. */
        virtual void setInnerXmlProperty(const std::string& xml);

        /** @return The XML markup representing this node and all its children. */
        [[nodiscard]] std::string getOuterXmlProperty() const;

        /** @return The parent node, or nullptr if none. */
        [[nodiscard]] XmlNode* getParentNodeProperty() const;
        /** @return The first child node, or nullptr if none. */
        [[nodiscard]] XmlNode* getFirstChildProperty() const;
        /** @return The last child node, or nullptr if none. */
        [[nodiscard]] XmlNode* getLastChildProperty() const;
        /** @return The previous sibling node, or nullptr if none. */
        [[nodiscard]] XmlNode* getPreviousSiblingProperty() const;
        /** @return The next sibling node, or nullptr if none. */
        [[nodiscard]] XmlNode* getNextSiblingProperty() const;
        /** @return A snapshot list of this node's children. */
        [[nodiscard]] XmlNodeList* getChildNodesProperty() const;
        /** @return This node's attributes, or nullptr for node types that don't have any (only XmlElement overrides this). */
        [[nodiscard]] virtual XmlAttributeCollection* getAttributesProperty() const { return nullptr; }
        /** @return The document that owns this node, or nullptr if this node is itself an XmlDocument. */
        [[nodiscard]] XmlDocument* getOwnerDocumentProperty() const { return ownerDocument_; }
        /** @return true if this node has at least one child. */
        [[nodiscard]] bool getHasChildNodesProperty() const;
        /** @return Always false; this runtime does not implement DTD-driven read-only nodes. */
        [[nodiscard]] bool getIsReadOnlyProperty() const { return false; }
        /** @return Always ""; this runtime does not track base URIs. */
        [[nodiscard]] virtual std::string getBaseURIProperty() const { return {}; }

        /** @brief Inserts @p newChild as the first child. @return @p newChild. */
        virtual XmlNode* PrependChild(XmlNode* newChild);
        /** @brief Inserts @p newChild as the last child. @return @p newChild. */
        virtual XmlNode* AppendChild(XmlNode* newChild);
        /** @brief Inserts @p newChild immediately before @p refChild (or at the end if @p refChild is nullptr). @return @p newChild. */
        virtual XmlNode* InsertBefore(XmlNode* newChild, XmlNode* refChild);
        /** @brief Inserts @p newChild immediately after @p refChild (or at the start if @p refChild is nullptr). @return @p newChild. */
        virtual XmlNode* InsertAfter(XmlNode* newChild, XmlNode* refChild);
        /** @brief Removes @p oldChild from this node's children. The removed node remains valid and reusable. @return @p oldChild. */
        virtual XmlNode* RemoveChild(XmlNode* oldChild);
        /** @brief Replaces @p oldChild with @p newChild. @return @p oldChild (the removed node). */
        virtual XmlNode* ReplaceChild(XmlNode* newChild, XmlNode* oldChild);
        /** @brief Removes all children (and, for XmlElement, all attributes). */
        virtual void RemoveAll();

        /** @brief Creates a copy of this node. @param deep If true, recursively clones children too. */
        [[nodiscard]] virtual XmlNode* CloneNode(bool deep) const;

        /** @brief Merges adjacent text nodes among this node's descendants. */
        virtual void Normalize();

        /** @return The first child XmlElement named @p name, or nullptr if none. */
        [[nodiscard]] XmlElement* Item(const std::string& name) const;

        /** @return Always false; DOM Level 2 feature testing is not implemented. */
        [[nodiscard]] bool Supports(const std::string& feature, const std::string& version) const;

        /** @brief Writes this node and its children to @p writer. */
        virtual void WriteTo(XmlWriter& writer) const;
        /** @brief Writes only this node's children to @p writer. */
        virtual void WriteContentTo(XmlWriter& writer) const;

        /**
         * @brief Creates an XPathNavigator (System::Xml::XPath::XmlDocumentNavigator) positioned
         * on this node. Caller takes ownership.
         *
         * C++ counterpart of .NET XmlNode.CreateNavigator() (via IXPathNavigable). Calling this
         * on an XmlAttribute repositions the navigator onto that attribute (attributes are not
         * part of the tinyxml2 node tree navigated by plain node pointers); this requires the
         * attribute to currently be attached to an element (@throws System::InvalidOperationException
         * if it is detached).
         */
        [[nodiscard]] XPath::XPathNavigator* CreateNavigator() const override;

        /** @return The first node matching @p xpath (document order), or nullptr. Caller takes ownership. Equivalent to `CreateNavigator()->SelectSingleNode(xpath)` unwrapped back to the underlying XmlNode. @throws XPath::XPathException on an XPath syntax error. */
        [[nodiscard]] XmlNode* SelectSingleNode(const std::string& xpath) const;
        /** @return All nodes matching @p xpath, in document order. Caller takes ownership of the returned list (the list, not the individual nodes, which remain owned by this document). @throws XPath::XPathException on an XPath syntax error. */
        [[nodiscard]] XmlNodeList* SelectNodes(const std::string& xpath) const;
    };

} // namespace System::Xml
