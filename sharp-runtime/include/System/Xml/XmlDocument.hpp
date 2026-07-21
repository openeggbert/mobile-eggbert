// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <tinyxml2/tinyxml2.h>

#include "System/Xml/XmlAttribute.hpp"
#include "System/Xml/XmlCDataSection.hpp"
#include "System/Xml/XmlComment.hpp"
#include "System/Xml/XmlDeclaration.hpp"
#include "System/Xml/XmlDocumentFragment.hpp"
#include "System/Xml/XmlDocumentType.hpp"
#include "System/Xml/XmlElement.hpp"
#include "System/Xml/XmlEntityReference.hpp"
#include "System/Xml/XmlImplementation.hpp"
#include "System/Xml/XmlNode.hpp"
#include "System/Xml/XmlNodeChangedEventArgs.hpp"
#include "System/Xml/XmlNodeList.hpp"
#include "System/Xml/XmlProcessingInstruction.hpp"
#include "System/Xml/XmlSignificantWhitespace.hpp"
#include "System/Xml/XmlText.hpp"
#include "System/Xml/XmlWhitespace.hpp"

namespace System::Xml {

    /**
     * @brief Represents an XML document; the root of the classic XmlDocument DOM API.
     *
     * C++ counterpart of .NET System.Xml.XmlDocument. Owns a tinyxml2::XMLDocument and caches
     * one wrapper object per native tinyxml2 node so navigation (ParentNode, FirstChild, etc.)
     * returns stable identities. Non-copyable/non-movable: it holds an internal self-pointer
     * (XmlNode::native_ points at its own tinyxml2::XMLDocument member) so its address must be
     * stable — construct it with `new`/on the stack and pass by pointer/reference, matching
     * other non-movable stream-like types in this runtime (e.g. FileStream).
     *
     * @note NodeInserting/NodeInserted/NodeRemoving/NodeRemoved/NodeChanging/NodeChanged are
     * exposed as settable std::function fields for API-name compatibility, but are not yet
     * wired into AppendChild/RemoveChild/etc. — a documented gap, not a silent one.
     */
    class XmlDocument : public XmlNode {
        tinyxml2::XMLDocument doc_;
        std::shared_ptr<XmlNameTable> nameTable_;
        std::unique_ptr<XmlImplementation> implementation_;
        std::unordered_map<tinyxml2::XMLNode*, std::unique_ptr<XmlNode>> nodeCache_;
        // Owns nodes created by factory methods with no native tinyxml2 backing to key nodeCache_
        // by (XmlAttribute, XmlEntityReference -- see those Create* methods' own comments): tracked
        // here until either the document is destroyed, or ReleaseUnattachedNode() transfers
        // ownership out (e.g. XmlElement::SetAttributeNode claims an attribute once attached).
        // Fixes a confirmed AddressSanitizer-caught leak (2026-07-14): CreateAttribute/
        // CreateEntityReference previously returned a bare `new`'d object with no owner at all
        // if it was never attached to anything.
        std::vector<std::unique_ptr<XmlNode>> unattachedNodes_;
        bool preserveWhitespace_ = false;
        // Scratch parent used to "detach" a node without destroying it (tinyxml2::Unlink is
        // private; InsertEndChild's documented move-if-already-parented semantics let us
        // achieve the same effect: moving a node here removes it from its real parent while
        // keeping it alive and reusable, matching .NET's RemoveChild contract).
        tinyxml2::XMLElement* detachedHolder_ = nullptr;

        XmlDocument(const XmlDocument&) = delete;
        XmlDocument& operator=(const XmlDocument&) = delete;
        XmlDocument(XmlDocument&&) = delete;
        XmlDocument& operator=(XmlDocument&&) = delete;

    public:
        XmlDocument();
        explicit XmlDocument(std::shared_ptr<XmlNameTable> nt);
        ~XmlDocument() override;

        /** @brief Resolves to this document itself (see XmlNode::GetDocument doc-comment). */
        [[nodiscard]] XmlDocument* GetDocument() const override { return const_cast<XmlDocument*>(this); }

        /** @brief Wraps a native tinyxml2 node in the appropriate XmlNode subclass, caching by identity. @return nullptr if @p native is nullptr. */
        XmlNode* WrapNode(tinyxml2::XMLNode* native);

        /**
         * @brief Internal: transfers ownership of a node this document is holding in
         * unattachedNodes_ (see that member's own comment) to the caller, who is taking over its
         * lifetime (e.g. XmlElement::SetAttributeNode, once an attribute is actually attached).
         * No-op if @p node isn't currently tracked there.
         */
        void ReleaseUnattachedNode(XmlNode* node);
        /** @return The underlying tinyxml2 document, for interop with code that needs direct tinyxml2 access. */
        [[nodiscard]] tinyxml2::XMLDocument& getNativeDocument() { return doc_; }
        /** @brief Internal: purges cached wrappers for @p native and its descendants (call before a tinyxml2 delete). */
        void PurgeCache(tinyxml2::XMLNode* native);
        /** @brief Internal: detaches @p native from its current parent, keeping it alive/reusable. */
        void DetachNode(tinyxml2::XMLNode* native);
        /** @return true if @p native is (or is parented directly under) this document's internal detached-node holder. */
        [[nodiscard]] bool IsDetached(const tinyxml2::XMLNode* native) const;

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::Document; }
        [[nodiscard]] std::string getNameProperty() const override { return "#document"; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return "#document"; }

        /** @return The root element, or nullptr if the document is empty. */
        [[nodiscard]] XmlElement* getDocumentElementProperty() const;
        /** @return The document type declaration node, or nullptr if none. */
        [[nodiscard]] XmlDocumentType* getDocumentTypeProperty() const;
        /** @return The implementation object associated with this document. */
        [[nodiscard]] XmlImplementation& getImplementationProperty() const { return *implementation_; }
        /** @return The NameTable associated with this document. */
        [[nodiscard]] std::shared_ptr<XmlNameTable> getNameTableProperty() const { return nameTable_; }
        /** @return Whether insignificant whitespace is preserved. This runtime relies on tinyxml2's own whitespace handling; the flag is stored for API compatibility. */
        [[nodiscard]] bool getPreserveWhitespaceProperty() const { return preserveWhitespace_; }
        void setPreserveWhitespaceProperty(bool value) { preserveWhitespace_ = value; }

        // --- Node factories ------------------------------------------------------------

        /** @throws XmlException / System::ArgumentException if @p name is not a well-formed XML name. */
        [[nodiscard]] XmlAttribute* CreateAttribute(const std::string& name);
        /** @throws XmlException / System::ArgumentException if @p name is not a well-formed XML name. */
        [[nodiscard]] XmlElement* CreateElement(const std::string& name);
        [[nodiscard]] XmlText* CreateTextNode(const std::string& text);
        [[nodiscard]] XmlComment* CreateComment(const std::string& data);
        [[nodiscard]] XmlCDataSection* CreateCDataSection(const std::string& data);
        /** @throws System::ArgumentException if @p target is empty. */
        [[nodiscard]] XmlProcessingInstruction* CreateProcessingInstruction(const std::string& target, const std::string& data);
        [[nodiscard]] XmlDeclaration* CreateXmlDeclaration(const std::string& version, const std::string& encoding, const std::string& standalone);
        [[nodiscard]] XmlDocumentFragment* CreateDocumentFragment();
        [[nodiscard]] XmlDocumentType* CreateDocumentType(const std::string& name, const std::string& publicId,
                                                          const std::string& systemId, const std::string& internalSubset);
        /** @throws System::ArgumentException if @p name starts with '#'. */
        [[nodiscard]] XmlEntityReference* CreateEntityReference(const std::string& name);
        [[nodiscard]] XmlWhitespace* CreateWhitespace(const std::string& text);
        [[nodiscard]] XmlSignificantWhitespace* CreateSignificantWhitespace(const std::string& text);

        /** @return All descendant elements named @p name (or "*" for all), in document order. */
        [[nodiscard]] XmlNodeList* GetElementsByTagName(const std::string& name) const;
        /** @return The first descendant element whose "id" attribute equals @p elementId, or nullptr. Simplification: matches a literal attribute named "id", not a DTD-declared ID-type attribute. */
        [[nodiscard]] XmlElement* GetElementById(const std::string& elementId) const;

        // --- Load / Save ---------------------------------------------------------------

        /** @brief Loads and parses the XML document at @p filename. @throws XmlException on parse error. */
        void Load(const std::string& filename);
        /** @brief Parses @p xml as the document content. @throws XmlException on parse error. */
        void LoadXml(const std::string& xml);
        /** @brief Saves the document to @p filename. */
        void Save(const std::string& filename) const;
        /** @brief Writes the document to @p writer. */
        void Save(XmlWriter& writer) const;

        [[nodiscard]] std::string getInnerXmlProperty() const override;
        void setInnerXmlProperty(const std::string& xml) override;
        void WriteTo(XmlWriter& writer) const override;
        void WriteContentTo(XmlWriter& writer) const override;

        // --- Node-change notifications (see class doc-comment: not yet wired to call sites) ---
        XmlNodeChangedEventHandler NodeInserting;
        XmlNodeChangedEventHandler NodeInserted;
        XmlNodeChangedEventHandler NodeRemoving;
        XmlNodeChangedEventHandler NodeRemoved;
        XmlNodeChangedEventHandler NodeChanging;
        XmlNodeChangedEventHandler NodeChanged;
    };

} // namespace System::Xml
