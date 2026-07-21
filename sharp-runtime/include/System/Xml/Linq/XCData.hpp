// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Xml/Linq/XText.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Represents a text node that contains CDATA.
     *
     * C++ counterpart of .NET System.Xml.Linq.XCData. Matches .NET's hierarchy: XCData derives
     * from XText (not directly from XNode) — it is exactly like a text node except for how it
     * serializes and its NodeType.
     *
     * @note Content containing the literal sequence "]]>" is written verbatim (not split across
     * multiple CDATA sections the way real .NET's XmlWriter does) — a narrow, documented gap for
     * an XML edge case unlikely to matter for game data.
     */
    class XCData : public XText {
    public:
        /** @brief Initializes a new CDATA text node with the given value. */
        explicit XCData(const std::string& value) : XText(value) {}

        [[nodiscard]] System::Xml::XmlNodeType getNodeTypeProperty() const override { return System::Xml::XmlNodeType::CDATA; }

        void WriteTo(System::Xml::XmlWriter& writer) const override;

    protected:
        void SerializeTo(std::ostream& os, int depth, bool indent) const override;
    };

} // namespace System::Xml::Linq
