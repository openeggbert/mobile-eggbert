// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <sstream>
#include <string>
#include "System/Xml/Linq/LoadOptions.hpp"
#include "System/Xml/Linq/SaveOptions.hpp"
#include "System/Xml/Linq/XContainer.hpp"
#include "System/Xml/Linq/XDocumentType.hpp"
#include "System/Xml/Linq/XElement.hpp"

namespace System::Xml {
    class XmlWriter;
}

namespace System::Xml::Linq {

    /** Represents the XML declaration (<?xml version="..." encoding="..." standalone="..."?>). Not an XNode, matching .NET. */
    class XDeclaration {
        std::string version_;
        std::string encoding_;
        std::string standalone_;
    public:
        /** Constructs an XML declaration with the given version, encoding, and standalone value. */
        XDeclaration(const std::string& version, const std::string& encoding, const std::string& standalone)
            : version_(version), encoding_(encoding), standalone_(standalone) {}

        /** @return The XML version string (e.g. "1.0"). */
        [[nodiscard]] const std::string& getVersionProperty()    const { return version_; }

        /** @return The encoding name (e.g. "utf-8"). */
        [[nodiscard]] const std::string& getEncodingProperty()   const { return encoding_; }

        /** @return The standalone declaration value (e.g. "yes" or "no"). */
        [[nodiscard]] const std::string& getStandaloneProperty() const { return standalone_; }

        /**
         * @return The serialised processing instruction string.
         *
         * Matches real .NET's XDeclaration.ToString() exactly: each of version/encoding/
         * standalone is omitted individually when unset (empty string here stands in for
         * .NET's nullable-string default, consistent with encoding/standalone's existing
         * handling below) -- version is NOT defaulted to "1.0" when unset, since real .NET
         * doesn't default-fill it either.
         */
        [[nodiscard]] std::string ToString() const {
            std::string s = "<?xml";
            if (!version_.empty()) s += " version=\"" + version_ + "\"";
            if (!encoding_.empty()) s += " encoding=\"" + encoding_ + "\"";
            if (!standalone_.empty()) s += " standalone=\"" + standalone_ + "\"";
            s += "?>";
            return s;
        }
    };

    /**
     * @brief Represents an XML document: an optional declaration, an optional document type,
     * at most one root element, and any number of top-level comments/processing instructions.
     *
     * C++ counterpart of .NET System.Xml.Linq.XDocument.
     */
    class XDocument : public XContainer {
        std::shared_ptr<XDeclaration> declaration_;

    public:
        /** Default constructor — creates an empty document. */
        XDocument() = default;

        /** Constructs a document with only a root element. */
        explicit XDocument(std::shared_ptr<XElement> root) { if (root) Add(std::move(root)); }

        /** Constructs a document with an XML declaration and a root element. */
        XDocument(std::shared_ptr<XDeclaration> decl, std::shared_ptr<XElement> root)
            : declaration_(std::move(decl)) { if (root) Add(std::move(root)); }

        [[nodiscard]] System::Xml::XmlNodeType getNodeTypeProperty() const override { return System::Xml::XmlNodeType::Document; }

        /** @return The XML declaration, or nullptr if absent. */
        [[nodiscard]] std::shared_ptr<XDeclaration> getDeclarationProperty() const { return declaration_; }
        /** Sets the XML declaration. */
        void setDeclarationProperty(std::shared_ptr<XDeclaration> d) { declaration_ = std::move(d); }

        /** @return The root element, or nullptr if this document has none. */
        [[nodiscard]] std::shared_ptr<XElement> getRootProperty() const;
        /** @brief Replaces (or sets) the root element. If a root already exists it is first removed. */
        void setRootProperty(std::shared_ptr<XElement> root);

        /** @return The document type declaration, or nullptr if absent. */
        [[nodiscard]] std::shared_ptr<XDocumentType> getDocumentTypeProperty() const;

        /** Returns the root element if its name matches @p name, otherwise nullptr (matches XContainer::Element, kept for API compatibility). */
        [[nodiscard]] std::shared_ptr<XElement> Element(const XName& name) const {
            auto root = getRootProperty();
            return (root && root->getNameProperty() == name) ? root : nullptr;
        }

        void WriteTo(System::Xml::XmlWriter& writer) const override;

        /** @brief Saves this document to @p filePath (formatted, unless @p options disables it). */
        void Save(const std::string& filePath, SaveOptions options = SaveOptions::None) const;
        /** @brief Writes this document to @p writer (emits WriteStartDocument() first if a declaration is present). */
        void Save(System::Xml::XmlWriter& writer) const;

        /**
         * @brief Parses @p xml into an XDocument.
         * @param options See XElement::Parse's doc-comment for a caveat on LoadOptions::PreserveWhitespace.
         * @throws System::Xml::XmlException on malformed XML.
         */
        [[nodiscard]] static std::shared_ptr<XDocument> Parse(const std::string& xml, LoadOptions options = LoadOptions::None);

        /**
         * @brief Loads and parses the XML file at @p filePath into an XDocument.
         * @throws System::Xml::XmlException if the file is missing or malformed.
         */
        [[nodiscard]] static std::shared_ptr<XDocument> Load(const std::string& filePath, LoadOptions options = LoadOptions::None);

        [[nodiscard]] SharpRuntime::intcs GetDeepHashCode() const override;

    protected:
        void ValidateNode(const XNode& node) const override;
        void SerializeTo(std::ostream& os, int depth, bool indent) const override;
        [[nodiscard]] bool DeepEqualsCore(const XNode& other) const override;
    };

} // namespace System::Xml::Linq
