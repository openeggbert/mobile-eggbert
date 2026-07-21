// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlCharacterData.hpp"

namespace System::Xml {

    /** @brief Represents whitespace between markup. C++ counterpart of .NET System.Xml.XmlWhitespace. */
    class XmlWhitespace : public XmlCharacterData {
    public:
        XmlWhitespace(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlCharacterData(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::Whitespace; }
        [[nodiscard]] std::string getNameProperty() const override { return "#whitespace"; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return "#whitespace"; }
    };

} // namespace System::Xml
