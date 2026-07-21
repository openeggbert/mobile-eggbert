// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlDeclaration.hpp"

#include <tinyxml2/tinyxml2.h>

#include "System/ArgumentException.hpp"

namespace System::Xml {

    namespace {
        std::string RawValue(tinyxml2::XMLNode* n) {
            auto* d = n ? n->ToDeclaration() : nullptr;
            return d && d->Value() ? d->Value() : "";
        }

        std::string ExtractAttr(const std::string& raw, const std::string& attrName) {
            std::string needle = attrName + "=\"";
            size_t pos = raw.find(needle);
            if (pos == std::string::npos) {
                needle = attrName + "='";
                pos = raw.find(needle);
                if (pos == std::string::npos) return {};
            }
            size_t start = pos + needle.size();
            char quote = raw[pos + needle.size() - 1];
            size_t end = raw.find(quote, start);
            if (end == std::string::npos) return {};
            return raw.substr(start, end - start);
        }
    }

    std::string XmlDeclaration::getVersionProperty() const {
        std::string v = ExtractAttr(RawValue(native_), "version");
        return v.empty() ? "1.0" : v;
    }

    std::string XmlDeclaration::getEncodingProperty() const {
        return ExtractAttr(RawValue(native_), "encoding");
    }

    std::string XmlDeclaration::getStandaloneProperty() const {
        return ExtractAttr(RawValue(native_), "standalone");
    }

    void XmlDeclaration::Rebuild(const std::string& version, const std::string& encoding, const std::string& standalone) {
        auto* d = native_ ? native_->ToDeclaration() : nullptr;
        if (!d) return;
        std::string text = "xml version=\"" + version + "\"";
        if (!encoding.empty()) text += " encoding=\"" + encoding + "\"";
        if (!standalone.empty()) text += " standalone=\"" + standalone + "\"";
        d->SetValue(text.c_str());
    }

    void XmlDeclaration::setEncodingProperty(const std::string& encoding) {
        Rebuild(getVersionProperty(), encoding, getStandaloneProperty());
    }

    void XmlDeclaration::setStandaloneProperty(const std::string& standalone) {
        if (!standalone.empty() && standalone != "yes" && standalone != "no")
            throw System::ArgumentException("Wrong value for the XML declaration standalone attribute of '" + standalone + "'.");
        Rebuild(getVersionProperty(), getEncodingProperty(), standalone);
    }

    std::string XmlDeclaration::getValueProperty() const {
        return RawValue(native_).substr(3); // skip "xml" prefix, matching .NET's Value (everything after "xml ")
    }

    void XmlDeclaration::setValueProperty(const std::string& value) {
        auto* d = native_ ? native_->ToDeclaration() : nullptr;
        if (!d) return;
        d->SetValue(("xml" + value).c_str());
    }

} // namespace System::Xml
