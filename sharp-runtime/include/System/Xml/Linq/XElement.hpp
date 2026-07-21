// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "System/Xml/Linq/LoadOptions.hpp"
#include "System/Xml/Linq/SaveOptions.hpp"
#include "System/Xml/Linq/XAttribute.hpp"
#include "System/Xml/Linq/XContainer.hpp"
#include "System/Xml/Linq/XName.hpp"

namespace System::Xml {
    class XmlWriter;
}

namespace System::Xml::Linq {

    /**
     * @brief Represents an XML element with a name, attributes, and content (a mix of child
     * elements, text, CDATA, comments, and processing instructions).
     *
     * C++ counterpart of .NET System.Xml.Linq.XElement.
     */
    class XElement : public XContainer {
        XName name_;
        std::vector<std::shared_ptr<XAttribute>> attributes_;

        void RelinkAttributes();

    public:
        /** Constructs an empty element with the given XName (plain strings convert implicitly via XName). */
        explicit XElement(const XName& name) : name_(name) {}

        /** Constructs an element with an XName and an initial text value (a single XText child; empty strings add no child). */
        XElement(const XName& name, const std::string& value) : name_(name) {
            if (!value.empty()) setValueProperty(value);
        }

        using XContainer::Add;
        using XContainer::AddFirst;

        [[nodiscard]] System::Xml::XmlNodeType getNodeTypeProperty() const override { return System::Xml::XmlNodeType::Element; }

        /** @return The qualified name of the element. */
        [[nodiscard]] const XName& getNameProperty() const { return name_; }
        /** @brief Sets the qualified name of the element. */
        void setNameProperty(const XName& name) { name_ = name; }

        /**
         * @return The text contents of this element. If there is text content interspersed with
         * nodes (mixed content), the text is concatenated (matches .NET's XElement.Value getter).
         */
        [[nodiscard]] std::string getValueProperty() const;
        /** @brief Replaces all content of this element with a single text node containing @p v (a no-op child-wise if @p v is empty). */
        void setValueProperty(const std::string& v);

        // --- Attributes ----------------------------------------------------------------

        /**
         * @brief Appends @p attr to this element's attributes.
         * If @p attr already belongs to another element, it is moved here (see XContainer's doc-comment).
         * @throws System::InvalidOperationException if this element already has an attribute with the same name.
         */
        void Add(std::shared_ptr<XAttribute> attr);

        /** @brief Appends a text child containing @p text. Convenience for `Add(std::make_shared<XText>(text))`. */
        void Add(const std::string& text);

        /** Returns the first attribute matching @p name, or nullptr. */
        [[nodiscard]] std::shared_ptr<XAttribute> Attribute(const XName& name) const {
            for (auto& a : attributes_)
                if (a->getNameProperty() == name) return a;
            return nullptr;
        }

        /** @return All attributes of this element, in document order. */
        [[nodiscard]] const std::vector<std::shared_ptr<XAttribute>>& getAttributesProperty() const { return attributes_; }
        /** @return All attributes of this element, in document order (.NET-style method name; same content as getAttributesProperty()). */
        [[nodiscard]] std::vector<std::shared_ptr<XAttribute>> Attributes() const { return attributes_; }

        /** @return The first attribute of this element, or nullptr. */
        [[nodiscard]] std::shared_ptr<XAttribute> getFirstAttributeProperty() const { return attributes_.empty() ? nullptr : attributes_.front(); }
        /** @return The last attribute of this element, or nullptr. */
        [[nodiscard]] std::shared_ptr<XAttribute> getLastAttributeProperty() const { return attributes_.empty() ? nullptr : attributes_.back(); }
        /** @return true if this element has at least one attribute. */
        [[nodiscard]] bool getHasAttributesProperty() const { return !attributes_.empty(); }
        /** @return true if this element has at least one child element. */
        [[nodiscard]] bool getHasElementsProperty() const;
        /** @return true if this element has no content (no attributes are required to be empty — matches .NET, which defines IsEmpty in terms of content only). */
        [[nodiscard]] bool getIsEmptyProperty() const { return children_.empty(); }

        /** @brief Removes all attributes from this element. */
        void RemoveAttributes();
        /** @brief Detaches @p attr from this element (used internally by XAttribute::Remove()). No-op if @p attr does not belong to this element. */
        void RemoveAttribute(XAttribute* attr);

        /** @brief Removes all attributes and content from this element. */
        void RemoveAll();

        /** Returns the value of the attribute named @p name, or std::nullopt if absent. */
        [[nodiscard]] std::optional<std::string> getAttributeValue(const std::string& name) const {
            auto a = Attribute(name);
            if (a) return a->getValueProperty();
            return std::nullopt;
        }

        // --- Serialization ---------------------------------------------------------------

        void WriteTo(System::Xml::XmlWriter& writer) const override;

        /** @brief Saves this element to @p fileName (formatted, unless @p options disables it). */
        void Save(const std::string& fileName, SaveOptions options = SaveOptions::None) const;
        /** @brief Writes this element to @p writer (no start/end-document wrapping). */
        void Save(System::Xml::XmlWriter& writer) const;

        /**
         * @brief Parses @p xml into an XElement.
         * @param xml XML source text; must contain exactly one root element.
         * @param options Controls whitespace handling (SetBaseUri/SetLineInfo have no effect — see LoadOptions).
         * Only affects text nodes that mix whitespace with real content — the vendored tinyxml2
         * parser never surfaces pure-whitespace-only runs immediately adjacent to element tags as
         * text nodes at all (verified directly), so LoadOptions::PreserveWhitespace has no
         * observable effect for that specific case (a limitation inherited from the parsing
         * backend, already documented at the classic-DOM layer on XmlDocument::PreserveWhitespace).
         * @throws System::Xml::XmlException on malformed XML.
         */
        [[nodiscard]] static std::shared_ptr<XElement> Parse(const std::string& xml, LoadOptions options = LoadOptions::None);

        /**
         * @brief Loads and parses the XML file at @p filePath into an XElement.
         * @throws System::Xml::XmlException if the file is missing or malformed.
         */
        [[nodiscard]] static std::shared_ptr<XElement> Load(const std::string& filePath, LoadOptions options = LoadOptions::None);

        [[nodiscard]] SharpRuntime::intcs GetDeepHashCode() const override;

    protected:
        void SerializeTo(std::ostream& os, int depth, bool indent) const override;
        [[nodiscard]] bool DeepEqualsCore(const XNode& other) const override;

        /**
         * @brief Rejects adding an XDocument or XDocumentType as a child element.
         * @throws System::ArgumentException if @p node is an XDocument or XDocumentType.
         */
        void ValidateNode(const XNode& node) const override;

    private:
        void AppendTextValue(std::string& sb) const;
    };

} // namespace System::Xml::Linq
