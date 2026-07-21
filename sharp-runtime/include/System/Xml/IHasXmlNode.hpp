// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    class XmlNode;

    /**
     * @brief Bridges an XPathNavigator-like cursor back to the underlying XmlNode it wraps.
     *
     * C++ counterpart of .NET System.Xml.IHasXmlNode. In real .NET this interface is implemented
     * by the DOM's internal XPathNavigator, letting XPath query code retrieve the concrete DOM
     * node behind a navigator. System.Xml.XPath (XPathNavigator/XPathDocument) is a separate,
     * not-yet-ported namespace in this runtime, so no concrete type implements this interface
     * yet — it is provided for API name compatibility with ported code that references it.
     */
    class IHasXmlNode {
    public:
        virtual ~IHasXmlNode() = default;

        /** @return The underlying XmlNode. */
        [[nodiscard]] virtual XmlNode* GetNode() const = 0;
    };

} // namespace System::Xml
