// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::XPath {

    class XPathNavigator;

    /**
     * @brief Provides an object for navigating XML data using an XPath data model.
     *
     * C++ counterpart of .NET System.Xml.XPath.IXPathNavigable. Implemented by
     * System::Xml::XmlNode (whole DOM tree, positioned per-node) and by XPathDocument.
     */
    class IXPathNavigable {
    public:
        virtual ~IXPathNavigable() = default;

        /** @return A new XPathNavigator positioned on this object. Caller takes ownership. */
        [[nodiscard]] virtual XPathNavigator* CreateNavigator() const = 0;
    };

} // namespace System::Xml::XPath
