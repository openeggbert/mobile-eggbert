// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>

#include "System/Xml/WhitespaceHandling.hpp"
#include "System/Xml/XmlReader.hpp"

namespace System::Xml {

    /**
     * @brief Represents a reader that provides fast, non-cached, forward-only access to XML data.
     *
     * C++ counterpart of .NET System.Xml.XmlTextReader. In .NET, XmlTextReader is a concrete
     * subclass of the abstract XmlReader; this runtime's XmlReader is already a single concrete
     * tinyxml2-backed implementation (not an extensible abstract hierarchy), so XmlTextReader is
     * implemented via composition — it owns an XmlReader and forwards to it — rather than
     * inheritance. Namespaces/WhitespaceHandling/Normalization are stored for API-name
     * compatibility but not currently consulted by the underlying reader (documented, not silent).
     */
    class XmlTextReader {
        std::unique_ptr<XmlReader> inner_;

    public:
        /** @brief Opens @p url (a file path or raw XML text, per XmlReader::Create's own heuristic). */
        explicit XmlTextReader(const std::string& url) : inner_(XmlReader::Create(url)) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const { return inner_->getNodeTypeProperty(); }
        [[nodiscard]] std::string getNameProperty() const { return inner_->getNameProperty(); }
        [[nodiscard]] std::string getValueProperty() const { return inner_->getValueProperty(); }
        [[nodiscard]] bool getIsEmptyElementProperty() const { return inner_->getIsEmptyElementProperty(); }
        [[nodiscard]] ReadState getReadStateProperty() const { return inner_->getReadStateProperty(); }

        /** @brief Whether namespace support is enabled. Stored for API compatibility; not currently consulted. */
        bool getNamespacesProperty() const { return namespaces_; }
        void setNamespacesProperty(bool value) { namespaces_ = value; }

        /** @brief How whitespace nodes are handled. Stored for API compatibility; not currently consulted. */
        [[nodiscard]] System::Xml::WhitespaceHandling getWhitespaceHandlingProperty() const { return whitespaceHandling_; }
        void setWhitespaceHandlingProperty(System::Xml::WhitespaceHandling value) { whitespaceHandling_ = value; }

        /** @brief Whether attribute value normalization is applied. Stored for API compatibility; not currently consulted. */
        [[nodiscard]] bool getNormalizationProperty() const { return normalization_; }
        void setNormalizationProperty(bool value) { normalization_ = value; }

        bool Read() { return inner_->Read(); }
        bool MoveToElement() { return inner_->MoveToElement(); }
        bool MoveToNextAttribute() { return inner_->MoveToNextAttribute(); }
        [[nodiscard]] std::string GetAttribute(const std::string& name) const { return inner_->GetAttribute(name); }
        std::string ReadElementContentAsString() { return inner_->ReadElementContentAsString(); }
        void ReadStartElement() { inner_->ReadStartElement(); }
        void ReadEndElement() { inner_->ReadEndElement(); }
        void Close() { inner_->Close(); }

    private:
        bool namespaces_ = true;
        System::Xml::WhitespaceHandling whitespaceHandling_ = System::Xml::WhitespaceHandling::All;
        bool normalization_ = false;
    };

} // namespace System::Xml
