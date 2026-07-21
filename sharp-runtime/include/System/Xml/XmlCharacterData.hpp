// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlLinkedNode.hpp"

namespace System::Xml {

    /**
     * @brief Base class for text-like nodes (XmlComment, XmlText, XmlCDataSection,
     * XmlWhitespace, XmlSignificantWhitespace) that hold a single string of character data.
     *
     * C++ counterpart of .NET System.Xml.XmlCharacterData.
     */
    class XmlCharacterData : public XmlLinkedNode {
    protected:
        XmlCharacterData(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlLinkedNode(native, ownerDocument) {}

    public:
        /** @return The character data. Same as Value/InnerText for this node type. */
        [[nodiscard]] std::string getValueProperty() const override { return getDataProperty(); }
        /** @brief Sets the character data. */
        void setValueProperty(const std::string& value) override { setDataProperty(value); }
        /** @return The character data. Same as Value for this node type. */
        [[nodiscard]] std::string getInnerTextProperty() const override { return getDataProperty(); }
        /** @brief Sets the character data. */
        void setInnerTextProperty(const std::string& text) override { setDataProperty(text); }

        /** @return The character data of this node. */
        [[nodiscard]] virtual std::string getDataProperty() const;
        /** @brief Sets the character data of this node. */
        virtual void setDataProperty(const std::string& data);
        /** @return The number of characters in Data. */
        [[nodiscard]] virtual SharpRuntime::intcs getLengthProperty() const;

        /** @return The substring of Data starting at @p offset with length @p count. */
        [[nodiscard]] virtual std::string Substring(SharpRuntime::intcs offset, SharpRuntime::intcs count) const;
        /** @brief Appends @p strData to Data. */
        virtual void AppendData(const std::string& strData);
        /** @brief Inserts @p strData into Data at @p offset. */
        virtual void InsertData(SharpRuntime::intcs offset, const std::string& strData);
        /** @brief Removes @p count characters from Data starting at @p offset. */
        virtual void DeleteData(SharpRuntime::intcs offset, SharpRuntime::intcs count);
        /** @brief Replaces @p count characters of Data starting at @p offset with @p strData. */
        virtual void ReplaceData(SharpRuntime::intcs offset, SharpRuntime::intcs count, const std::string& strData);

        /** @brief Writes this node's data as a plain (escaped) text run. XmlComment overrides this to write comment syntax instead. */
        void WriteTo(XmlWriter& writer) const override;
    };

} // namespace System::Xml
