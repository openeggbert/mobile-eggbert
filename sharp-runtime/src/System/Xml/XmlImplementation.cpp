// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlImplementation.hpp"

#include <cctype>
#include <cstring>

#include "System/Xml/XmlDocument.hpp"

namespace System::Xml {

    XmlImplementation::XmlImplementation() : nameTable_(std::make_shared<NameTable>()) {}

    XmlImplementation::XmlImplementation(std::shared_ptr<XmlNameTable> nt) : nameTable_(std::move(nt)) {}

    namespace {
        bool equalsOrdinalIgnoreCase(const std::string& a, const char* b) {
            if (a.size() != std::strlen(b)) return false;
            for (std::size_t i = 0; i < a.size(); ++i)
                if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
                    return false;
            return true;
        }
    }

    bool XmlImplementation::HasFeature(const std::string& strFeature, const std::string& strVersion) const {
        if (equalsOrdinalIgnoreCase(strFeature, "XML")) {
            if (strVersion.empty() || strVersion == "1.0" || strVersion == "2.0")
                return true;
        }
        return false;
    }

    std::unique_ptr<XmlDocument> XmlImplementation::CreateDocument() const {
        return std::make_unique<XmlDocument>(nameTable_);
    }

} // namespace System::Xml
