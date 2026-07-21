// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlProcessingInstruction.hpp"

#include <tinyxml2/tinyxml2.h>

namespace System::Xml {

    namespace {
        std::string RawValue(tinyxml2::XMLNode* n) {
            auto* d = n ? n->ToDeclaration() : nullptr;
            return d && d->Value() ? d->Value() : "";
        }
    }

    std::string XmlProcessingInstruction::getTargetProperty() const {
        std::string raw = RawValue(native_);
        size_t sp = raw.find_first_of(" \t\r\n");
        return sp == std::string::npos ? raw : raw.substr(0, sp);
    }

    std::string XmlProcessingInstruction::getDataProperty() const {
        std::string raw = RawValue(native_);
        size_t sp = raw.find_first_of(" \t\r\n");
        if (sp == std::string::npos) return {};
        size_t dataStart = raw.find_first_not_of(" \t\r\n", sp);
        return dataStart == std::string::npos ? "" : raw.substr(dataStart);
    }

    void XmlProcessingInstruction::setDataProperty(const std::string& data) {
        auto* d = native_ ? native_->ToDeclaration() : nullptr;
        if (!d) return;
        std::string newValue = getTargetProperty() + " " + data;
        d->SetValue(newValue.c_str());
    }

} // namespace System::Xml
