// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlText.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml {

    /** @brief Represents a CDATA section (`<![CDATA[...]]>`). C++ counterpart of .NET System.Xml.XmlCDataSection. */
    class XmlCDataSection : public XmlText {
    public:
        XmlCDataSection(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlText(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::CDATA; }
        [[nodiscard]] std::string getNameProperty() const override { return "#cdata-section"; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return "#cdata-section"; }

        /** @brief Writes this section as @c <![CDATA[data]]> rather than XmlCharacterData's default escaped plain-text run. */
        void WriteTo(XmlWriter& writer) const override { writer.WriteCData(getDataProperty()); }
    };

} // namespace System::Xml
