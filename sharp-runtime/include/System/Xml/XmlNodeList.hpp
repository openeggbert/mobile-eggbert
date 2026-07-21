// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <vector>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Xml {

    class XmlNode;

    /**
     * @brief Represents an ordered collection of nodes, e.g. XmlNode::ChildNodes.
     *
     * C++ counterpart of .NET System.Xml.XmlNodeList. Holds a materialized snapshot (taken at
     * construction) of non-owning node pointers, not a live view — matching this runtime's
     * broader "ChildNodes is a snapshot" simplification (see XmlNode's doc-comment).
     */
    class XmlNodeList {
        std::vector<XmlNode*> items_;

    public:
        explicit XmlNodeList(std::vector<XmlNode*> items) : items_(std::move(items)) {}

        /** @return The node at @p index, or nullptr if out of range. */
        [[nodiscard]] XmlNode* Item(SharpRuntime::intcs index) const {
            if (index < 0 || static_cast<size_t>(index) >= items_.size()) return nullptr;
            return items_[static_cast<size_t>(index)];
        }
        /** @return The node at @p index, or nullptr if out of range. */
        [[nodiscard]] XmlNode* operator[](SharpRuntime::intcs index) const { return Item(index); }
        /** @return The number of nodes in the list. */
        [[nodiscard]] SharpRuntime::intcs getCountProperty() const { return static_cast<SharpRuntime::intcs>(items_.size()); }

        [[nodiscard]] auto begin() const { return items_.begin(); }
        [[nodiscard]] auto end() const { return items_.end(); }
    };

} // namespace System::Xml
