// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/Formatting.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml {

    /**
     * @brief Represents a writer that provides a fast, non-cached, forward-only way of
     * generating streams or files containing XML data.
     *
     * C++ counterpart of .NET System.Xml.XmlTextWriter. Implemented via composition (owns an
     * XmlWriter and forwards to it) for the same reason as XmlTextReader: this runtime's
     * XmlWriter is already a single concrete tinyxml2-backed implementation, not an extensible
     * abstract hierarchy. Formatting/Indentation/IndentChar are stored for API-name
     * compatibility but not currently consulted (documented, not silent).
     */
    class XmlTextWriter {
        std::unique_ptr<XmlWriter> inner_;

    public:
        /** @brief Creates a writer that serializes to the file at @p filename. @p encoding is accepted but not currently consulted (tinyxml2 always writes UTF-8). */
        XmlTextWriter(const std::string& filename, const std::string& encoding)
            : inner_(XmlWriter::Create(filename)) { (void)encoding; }

        /** @brief Creates a writer that serializes to an in-memory string; use ToString() to retrieve it. */
        XmlTextWriter() : inner_(XmlWriter::CreateToString()) {}

        [[nodiscard]] System::Xml::Formatting getFormattingProperty() const { return formatting_; }
        void setFormattingProperty(System::Xml::Formatting value) { formatting_ = value; }

        [[nodiscard]] SharpRuntime::intcs getIndentationProperty() const { return indentation_; }
        void setIndentationProperty(SharpRuntime::intcs value) { indentation_ = value; }

        void WriteStartDocument() { inner_->WriteStartDocument(); }
        void WriteEndDocument() { inner_->WriteEndDocument(); }
        void WriteStartElement(const std::string& localName) { inner_->WriteStartElement(localName); }
        void WriteEndElement() { inner_->WriteEndElement(); }
        void WriteAttributeString(const std::string& name, const std::string& value) { inner_->WriteAttributeString(name, value); }
        void WriteString(const std::string& text) { inner_->WriteString(text); }
        void WriteElementString(const std::string& name, const std::string& value) { inner_->WriteElementString(name, value); }
        void WriteComment(const std::string& text) { inner_->WriteComment(text); }
        void WriteCData(const std::string& text) { inner_->WriteCData(text); }
        [[nodiscard]] std::string ToString() const { return inner_->ToString(); }
        void Flush() { inner_->Flush(); }
        void Close() { inner_->Close(); }

    private:
        System::Xml::Formatting formatting_ = System::Xml::Formatting::None;
        SharpRuntime::intcs indentation_ = 2;
    };

} // namespace System::Xml
