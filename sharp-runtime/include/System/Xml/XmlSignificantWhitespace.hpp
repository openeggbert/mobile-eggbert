// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlCharacterData.hpp"

namespace System::Xml {

    /**
     * @brief Represents whitespace in element content that is significant (xml:space="preserve" scope).
     * C++ counterpart of .NET System.Xml.XmlSignificantWhitespace.
     */
    class XmlSignificantWhitespace : public XmlCharacterData {
    public:
        XmlSignificantWhitespace(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlCharacterData(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::SignificantWhitespace; }
        [[nodiscard]] std::string getNameProperty() const override { return "#significant-whitespace"; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return "#significant-whitespace"; }
    };

} // namespace System::Xml
