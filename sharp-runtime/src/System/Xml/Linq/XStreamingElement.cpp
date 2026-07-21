// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/Linq/XStreamingElement.hpp"
#include <memory>
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml::Linq {

    void XStreamingElement::Add(const std::string& text) { content_.emplace_back(text); }
    void XStreamingElement::Add(std::shared_ptr<XAttribute> attr) { content_.emplace_back(std::move(attr)); }
    void XStreamingElement::Add(std::shared_ptr<XNode> node) { content_.emplace_back(std::move(node)); }
    void XStreamingElement::Add(std::shared_ptr<XStreamingElement> nested) { content_.emplace_back(std::move(nested)); }

    void XStreamingElement::WriteContent(System::Xml::XmlWriter& writer, const std::vector<std::any>& items) const {
        for (const auto& item : items) {
            if (const auto* s = std::any_cast<std::string>(&item)) {
                writer.WriteString(*s);
            } else if (const auto* a = std::any_cast<std::shared_ptr<XAttribute>>(&item)) {
                // See XElement::WriteTo's comment: the local name only, not XName::ToString()'s
                // Clark notation, which would otherwise produce malformed XML for a namespaced
                // attribute ('{'/'}' are not legal in an XML Name production).
                if (*a) writer.WriteAttributeString((*a)->getNameProperty().getLocalNameProperty(), (*a)->getValueProperty());
            } else if (const auto* n = std::any_cast<std::shared_ptr<XNode>>(&item)) {
                if (*n) (*n)->WriteTo(writer);
            } else if (const auto* se = std::any_cast<std::shared_ptr<XStreamingElement>>(&item)) {
                if (*se) (*se)->WriteTo(writer);
            }
            // Any other stored type is silently skipped — see class doc-comment on the scoped content model.
        }
    }

    void XStreamingElement::WriteTo(System::Xml::XmlWriter& writer) const {
        writer.WriteStartElement(name_.getLocalNameProperty());
        WriteContent(writer, content_);
        writer.WriteEndElement();
    }

    void XStreamingElement::Save(const std::string& fileName, SaveOptions /*options*/) const {
        std::unique_ptr<System::Xml::XmlWriter> w(System::Xml::XmlWriter::Create(fileName));
        if (!w) throw System::Xml::XmlException("XStreamingElement::Save: failed to open '" + fileName + "'.");
        WriteTo(*w);
        w->Close();
    }

    void XStreamingElement::Save(System::Xml::XmlWriter& writer) const {
        writer.WriteStartDocument();
        WriteTo(writer);
        writer.WriteEndDocument();
    }

    std::string XStreamingElement::ToString(SaveOptions /*options*/) const {
        std::unique_ptr<System::Xml::XmlWriter> w(System::Xml::XmlWriter::CreateToString());
        WriteTo(*w);
        return w->ToString();
    }

} // namespace System::Xml::Linq
