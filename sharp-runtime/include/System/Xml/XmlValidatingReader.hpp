// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>

#include "System/Xml/ValidationType.hpp"
#include "System/Xml/XmlReader.hpp"

namespace System::Xml {

    /**
     * @brief A reader that provides schema validation support on top of an underlying XmlReader.
     *
     * C++ counterpart of .NET System.Xml.XmlValidatingReader — legacy/obsolete even in .NET
     * itself (superseded by XmlReaderSettings::ValidationType + XmlReader::Create). This runtime
     * performs no DTD/schema validation regardless of ValidationType (a documented practical-
     * subset limitation): all Read()-family calls simply delegate to the wrapped reader.
     */
    class XmlValidatingReader {
        XmlReader* inner_;

    public:
        /** @brief Wraps @p reader; this object does not take ownership. */
        explicit XmlValidatingReader(XmlReader* reader) : inner_(reader) {}

        /** @brief The type of validation to perform. Stored for API-name compatibility; validation is never actually performed. */
        [[nodiscard]] System::Xml::ValidationType getValidationTypeProperty() const { return validationType_; }
        void setValidationTypeProperty(System::Xml::ValidationType value) { validationType_ = value; }

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const { return inner_->getNodeTypeProperty(); }
        [[nodiscard]] std::string getNameProperty() const { return inner_->getNameProperty(); }
        [[nodiscard]] std::string getValueProperty() const { return inner_->getValueProperty(); }
        [[nodiscard]] bool getIsEmptyElementProperty() const { return inner_->getIsEmptyElementProperty(); }
        [[nodiscard]] ReadState getReadStateProperty() const { return inner_->getReadStateProperty(); }

        bool Read() { return inner_->Read(); }
        bool MoveToElement() { return inner_->MoveToElement(); }
        bool MoveToNextAttribute() { return inner_->MoveToNextAttribute(); }
        [[nodiscard]] std::string GetAttribute(const std::string& name) const { return inner_->GetAttribute(name); }
        std::string ReadElementContentAsString() { return inner_->ReadElementContentAsString(); }
        void ReadStartElement() { inner_->ReadStartElement(); }
        void ReadEndElement() { inner_->ReadEndElement(); }
        /** @brief No-op; this runtime performs no validation to resolve entities against. */
        void ResolveEntity() {}
        void Close() { inner_->Close(); }

    private:
        System::Xml::ValidationType validationType_ = System::Xml::ValidationType::Auto;
    };

} // namespace System::Xml
