// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Xml/Linq/XNode.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Represents an XML comment.
     *
     * C++ counterpart of .NET System.Xml.Linq.XComment.
     */
    class XComment : public XNode {
        std::string value_;

    public:
        /** @brief Initializes a new comment node with the given content. */
        explicit XComment(const std::string& value) : value_(value) {}

        [[nodiscard]] System::Xml::XmlNodeType getNodeTypeProperty() const override { return System::Xml::XmlNodeType::Comment; }

        /** @return The comment text. */
        [[nodiscard]] const std::string& getValueProperty() const { return value_; }
        /** @brief Sets the comment text. */
        void setValueProperty(const std::string& value) { value_ = value; }

        void WriteTo(System::Xml::XmlWriter& writer) const override;
        [[nodiscard]] SharpRuntime::intcs GetDeepHashCode() const override {
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(value_));
        }

    protected:
        void SerializeTo(std::ostream& os, int depth, bool indent) const override;
        [[nodiscard]] bool DeepEqualsCore(const XNode& other) const override {
            return value_ == static_cast<const XComment&>(other).value_;
        }
    };

} // namespace System::Xml::Linq
