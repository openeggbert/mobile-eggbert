// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Xml/Linq/XNode.hpp"

namespace System::Xml::Linq {

    /**
     * @brief Represents an XML processing instruction.
     *
     * C++ counterpart of .NET System.Xml.Linq.XProcessingInstruction.
     *
     * @note .NET validates Target as a well-formed NCName (and rejects the reserved "xml"
     * target, case-insensitively) via XmlConvert.VerifyNCName. This port does not perform that
     * validation — out of scope, consistent with this runtime's general lack of strict XML name
     * validation elsewhere (e.g. XName).
     */
    class XProcessingInstruction : public XNode {
        std::string target_;
        std::string data_;

    public:
        /** @brief Initializes a new processing instruction with the given target and data. */
        XProcessingInstruction(const std::string& target, const std::string& data)
            : target_(target), data_(data) {}

        [[nodiscard]] System::Xml::XmlNodeType getNodeTypeProperty() const override { return System::Xml::XmlNodeType::ProcessingInstruction; }

        /** @return The target application for this processing instruction. */
        [[nodiscard]] const std::string& getTargetProperty() const { return target_; }
        /** @brief Sets the target application. */
        void setTargetProperty(const std::string& target) { target_ = target; }

        /** @return The content of this processing instruction. */
        [[nodiscard]] const std::string& getDataProperty() const { return data_; }
        /** @brief Sets the content. */
        void setDataProperty(const std::string& data) { data_ = data; }

        void WriteTo(System::Xml::XmlWriter& writer) const override;
        [[nodiscard]] SharpRuntime::intcs GetDeepHashCode() const override {
            return static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(target_)) ^
                   static_cast<SharpRuntime::intcs>(std::hash<std::string>{}(data_));
        }

    protected:
        void SerializeTo(std::ostream& os, int depth, bool indent) const override;
        [[nodiscard]] bool DeepEqualsCore(const XNode& other) const override {
            const auto& o = static_cast<const XProcessingInstruction&>(other);
            return target_ == o.target_ && data_ == o.data_;
        }
    };

} // namespace System::Xml::Linq
