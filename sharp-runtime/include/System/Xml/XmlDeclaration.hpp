// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlLinkedNode.hpp"

namespace System::Xml {

    /**
     * @brief Represents the XML declaration node, e.g. `<?xml version='1.0'?>`.
     *
     * C++ counterpart of .NET System.Xml.XmlDeclaration. Backed by a tinyxml2 XMLDeclaration
     * whose Value() is "xml version=\"...\" encoding=\"...\" standalone=\"...\"";
     * Version/Encoding/Standalone are parsed out of and rebuilt into that text.
     */
    class XmlDeclaration : public XmlLinkedNode {
    public:
        XmlDeclaration(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlLinkedNode(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::XmlDeclaration; }
        [[nodiscard]] std::string getNameProperty() const override { return "xml"; }
        [[nodiscard]] std::string getLocalNameProperty() const override { return "xml"; }

        /** @return The XML version, always "1.0". */
        [[nodiscard]] std::string getVersionProperty() const;
        /** @return The encoding attribute value, or "" if absent. */
        [[nodiscard]] std::string getEncodingProperty() const;
        /** @brief Sets the encoding attribute. */
        void setEncodingProperty(const std::string& encoding);
        /** @return The standalone attribute value ("yes", "no", or "" if absent). */
        [[nodiscard]] std::string getStandaloneProperty() const;
        /**
         * @brief Sets the standalone attribute ("yes" or "no").
         * @throws System::ArgumentException if @p standalone is non-empty and not "yes" or "no".
         */
        void setStandaloneProperty(const std::string& standalone);

        [[nodiscard]] std::string getValueProperty() const override;
        void setValueProperty(const std::string& value) override;
        [[nodiscard]] std::string getInnerTextProperty() const override { return getValueProperty(); }
        void setInnerTextProperty(const std::string& text) override { setValueProperty(text); }

    private:
        /** @brief Rebuilds the native Value() text from version/encoding/standalone. */
        void Rebuild(const std::string& version, const std::string& encoding, const std::string& standalone);
    };

} // namespace System::Xml
