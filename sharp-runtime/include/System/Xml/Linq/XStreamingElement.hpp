// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <any>
#include <memory>
#include <string>
#include <vector>
#include "System/Xml/Linq/SaveOptions.hpp"
#include "System/Xml/Linq/XAttribute.hpp"
#include "System/Xml/Linq/XName.hpp"
#include "System/Xml/Linq/XNode.hpp"

namespace System::Xml {
    class XmlWriter;
}

namespace System::Xml::Linq {

    /**
     * @brief A write-only element: content is written out via WriteTo()/Save()/ToString()
     * without ever materializing a full XElement tree. Not part of the node hierarchy (not an
     * XNode/XContainer, cannot be added as a child of an XElement/XDocument the way an XElement
     * can — matches .NET, where XStreamingElement derives directly from object).
     *
     * C++ counterpart of .NET System.Xml.Linq.XStreamingElement.
     *
     * @note Real .NET's "streaming" behavior comes from C# iterator (`yield return`) laziness:
     * an `IEnumerable` passed as content is only enumerated at write time, so a query over a
     * huge/lazy sequence never has to be materialized into memory. C++ has no exact analogue
     * without hand-rolled generators/coroutines, which is out of scope here. This port stores
     * added content eagerly in a `std::vector<std::any>` and simply defers *building an XElement
     * tree* (the other half of "streaming") — it still never allocates an XContainer/XElement
     * for itself. Content items are limited to: `std::string`, `std::shared_ptr<XAttribute>`,
     * `std::shared_ptr<XNode>` (any concrete node type, via implicit upcast), and
     * `std::shared_ptr<XStreamingElement>` (nested streaming elements) — a deliberately scoped
     * subset of real .NET's fully-dynamic `object?` content model.
     */
    class XStreamingElement {
        XName name_;
        std::vector<std::any> content_;

        void WriteContent(System::Xml::XmlWriter& writer, const std::vector<std::any>& items) const;

    public:
        /** @brief Creates an XStreamingElement with the given name and no content. */
        explicit XStreamingElement(const XName& name) : name_(name) {}

        /** @return The name of this streaming element. */
        [[nodiscard]] const XName& getNameProperty() const { return name_; }
        /** @brief Sets the name of this streaming element. */
        void setNameProperty(const XName& name) { name_ = name; }

        /** @brief Adds a text content item. */
        void Add(const std::string& text);
        /** @brief Adds an attribute. */
        void Add(std::shared_ptr<XAttribute> attr);
        /** @brief Adds a node (XElement, XComment, XCData, XText, XProcessingInstruction, or XDocumentType). */
        void Add(std::shared_ptr<XNode> node);
        /** @brief Adds a nested streaming element. */
        void Add(std::shared_ptr<XStreamingElement> nested);

        /** @brief Writes this streaming element (and its content) to @p writer. */
        void WriteTo(System::Xml::XmlWriter& writer) const;

        /** @brief Saves this streaming element to @p fileName. SaveOptions is accepted for API compatibility but has no effect (see class doc-comment on the underlying XmlWriter). */
        void Save(const std::string& fileName, SaveOptions options = SaveOptions::None) const;
        /** @brief Writes this streaming element to @p writer, wrapped in WriteStartDocument()/WriteEndDocument(). */
        void Save(System::Xml::XmlWriter& writer) const;

        /** @return The XML text representation of this streaming element. */
        [[nodiscard]] std::string ToString() const { return ToString(SaveOptions::None); }
        /** @return The XML text representation of this streaming element. @param options Accepted for API compatibility; has no effect (see class doc-comment). */
        [[nodiscard]] std::string ToString(SaveOptions options) const;
    };

} // namespace System::Xml::Linq
