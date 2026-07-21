// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlCharacterData.hpp"

namespace System::Xml {

    /** @brief Represents the text content of an element or attribute. C++ counterpart of .NET System.Xml.XmlText. */
    class XmlText : public XmlCharacterData {
    public:
        XmlText(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlCharacterData(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::Text; }
        [[nodiscard]] std::string getNameProperty() const override { return "#text"; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return "#text"; }

        /**
         * @brief Splits this text node into two nodes at @p offset, keeping [0, offset) in this
         * node and inserting a new sibling text node holding [offset, end).
         * @return The newly created sibling node containing the remainder.
         * @throws System::ArgumentOutOfRangeException if @p offset is greater than the node's length.
         * @throws System::InvalidOperationException if this node has no parent (is not in a live DOM tree).
         */
        virtual XmlText* SplitText(SharpRuntime::intcs offset);
    };

} // namespace System::Xml
