// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>

#include "System/Xml/XPath/IXPathNavigable.hpp"

namespace System::Xml {
    class XmlDocument;
} // namespace System::Xml

namespace System::Xml::XPath {

    /**
     * @brief A parsed XML document, exposed only for creating an XPathNavigator over it.
     *
     * C++ counterpart of .NET System.Xml.XPath.XPathDocument. Real .NET's XPathDocument is a
     * separate, specially-optimized read-only tree engine (distinct from XmlDocument's mutable
     * DOM) — but current .NET guidance treats it as largely legacy, superseded by
     * `XmlDocument.CreateNavigator()`. Per the user's 2026-07-07 scope decision, this
     * implementation matches that guidance directly: it is a thin wrapper around an internal
     * System::Xml::XmlDocument, not a second optimized tree implementation. Practically, prefer
     * constructing an System::Xml::XmlDocument and calling its CreateNavigator() directly; use
     * XPathDocument only when porting C# code that specifically constructs one.
     */
    class XPathDocument : public IXPathNavigable {
        std::unique_ptr<System::Xml::XmlDocument> doc_;

    public:
        /** @brief Creates an empty document. */
        XPathDocument();
        /** @brief Creates a document by parsing the XML file at @p uri. @throws XmlException on a parse error. */
        explicit XPathDocument(const std::string& uri);
        ~XPathDocument() override;

        XPathDocument(const XPathDocument&) = delete;
        XPathDocument& operator=(const XPathDocument&) = delete;

        /** @brief Parses @p xml as this document's content (this runtime's equivalent of constructing from an XmlReader/TextReader/Stream, all of which real .NET ultimately reads as XML text). @throws XmlException on a parse error. */
        void LoadXml(const std::string& xml);

        /** @return A new XPathNavigator positioned on the root of this document. Caller takes ownership. */
        [[nodiscard]] XPathNavigator* CreateNavigator() const override;
    };

} // namespace System::Xml::XPath
