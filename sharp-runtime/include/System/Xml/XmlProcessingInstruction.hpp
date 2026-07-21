// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlLinkedNode.hpp"

namespace System::Xml {

    /**
     * @brief Represents a processing instruction, e.g. `<?target data?>`.
     *
     * C++ counterpart of .NET System.Xml.XmlProcessingInstruction. Backed by a tinyxml2
     * XMLDeclaration node (tinyxml2 parses every `<?...?>` as XMLDeclaration regardless of
     * target, unlike .NET which only special-cases the literal "xml" target); Target/Data are
     * derived by splitting the native Value() on the first whitespace run.
     */
    class XmlProcessingInstruction : public XmlLinkedNode {
    public:
        XmlProcessingInstruction(tinyxml2::XMLNode* native, XmlDocument* ownerDocument)
            : XmlLinkedNode(native, ownerDocument) {}

        [[nodiscard]] XmlNodeType getNodeTypeProperty() const override { return XmlNodeType::ProcessingInstruction; }
        [[nodiscard]] std::string getNameProperty() const override { return getTargetProperty(); }
        [[nodiscard]] std::string getLocalNameProperty() const override { return getTargetProperty(); }

        /** @return The target of the processing instruction (the token immediately after `<?`). */
        [[nodiscard]] std::string getTargetProperty() const;
        /** @return The content of the processing instruction, following the target. */
        [[nodiscard]] std::string getDataProperty() const;
        /** @brief Sets the content of the processing instruction. */
        void setDataProperty(const std::string& data);

        [[nodiscard]] std::string getValueProperty() const override { return getDataProperty(); }
        void setValueProperty(const std::string& value) override { setDataProperty(value); }
        [[nodiscard]] std::string getInnerTextProperty() const override { return getDataProperty(); }
        void setInnerTextProperty(const std::string& text) override { setDataProperty(text); }
    };

} // namespace System::Xml
