// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Xml/Linq/XNode.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Represents a text node.
     *
     * C++ counterpart of .NET System.Xml.Linq.XText.
     *
     * @note Two related, deliberate scope reductions relative to real .NET: (1) real .NET's
     * XContainer.Add rejects a non-whitespace-only XText added directly under an XDocument with
     * ArgumentException (only prolog/epilog whitespace is legal there); this port performs no
     * such validation, since System::Xml::XmlWriter has no WriteWhitespace primitive to give the
     * distinction real behavioral meaning. (2) real .NET's XText::WriteTo calls
     * writer.WriteWhitespace(text) when the parent is an XDocument and writer.WriteString(text)
     * otherwise; this port always calls WriteString for the same reason (no WriteWhitespace
     * primitive exists on this port's XmlWriter). Adding a distinct WriteWhitespace primitive
     * with its own validation/formatting semantics is a larger XmlWriter change than a single
     * type audit should take on.
     */
    class XText : public XNode {
        std::string text_;

    public:
        /** @brief Initializes a new text node with the given value. */
        explicit XText(const std::string& value) : text_(value) {}

        [[nodiscard]] System::Xml::XmlNodeType getNodeTypeProperty() const override { return System::Xml::XmlNodeType::Text; }

        /** @return The text content of this node. */
        [[nodiscard]] virtual const std::string& getValueProperty() const { return text_; }
        /** @brief Sets the text content of this node. */
        virtual void setValueProperty(const std::string& value) { text_ = value; }

        void WriteTo(System::Xml::XmlWriter& writer) const override;
        [[nodiscard]] SharpRuntime::intcs GetDeepHashCode() const override {
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(text_));
        }

    protected:
        void SerializeTo(std::ostream& os, int depth, bool indent) const override;
        [[nodiscard]] bool DeepEqualsCore(const XNode& other) const override;
    };

} // namespace System::Xml::Linq
