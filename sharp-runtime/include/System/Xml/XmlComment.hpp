// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlCharacterData.hpp"
#include "System/Xml/XmlWriter.hpp"

namespace System::Xml {

    /** @brief Represents an XML comment (`<!-- text -->`). C++ counterpart of .NET System.Xml.XmlComment. */
    class XmlComment : public XmlCharacterData {
    public:
        XmlComment(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlCharacterData(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::Comment; }
        [[nodiscard]] std::string getNameProperty() const override { return "#comment"; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return "#comment"; }

        /** @brief Writes this comment as @c <!-- data --> rather than XmlCharacterData's default plain-text run. */
        void WriteTo(XmlWriter& writer) const override { writer.WriteComment(getDataProperty()); }
    };

} // namespace System::Xml
