// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <functional>
#include <string>

#include "System/Xml/XmlNodeChangedAction.hpp"

namespace System::Xml {

    class XmlNode;

    /**
     * @brief Event data for XmlDocument's NodeChanged/NodeChanging/NodeInserted/NodeInserting/
     * NodeRemoved/NodeRemoving events.
     *
     * C++ counterpart of .NET System.Xml.XmlNodeChangedEventArgs.
     */
    class XmlNodeChangedEventArgs {
        XmlNodeChangedAction action_;
        XmlNode* node_;
        XmlNode* oldParent_;
        XmlNode* newParent_;
        std::string oldValue_;
        std::string newValue_;

    public:
        XmlNodeChangedEventArgs(XmlNode* node, XmlNode* oldParent, XmlNode* newParent,
                                 const std::string& oldValue, const std::string& newValue,
                                 XmlNodeChangedAction action)
            : action_(action), node_(node), oldParent_(oldParent), newParent_(newParent),
              oldValue_(oldValue), newValue_(newValue) {}

        /** @return The kind of change being reported. */
        [[nodiscard]] XmlNodeChangedAction getActionProperty() const { return action_; }
        /** @return The node that changed. */
        [[nodiscard]] XmlNode* getNodeProperty() const { return node_; }
        /** @return The node's parent before the change, or nullptr. */
        [[nodiscard]] XmlNode* getOldParentProperty() const { return oldParent_; }
        /** @return The node's parent after the change, or nullptr. */
        [[nodiscard]] XmlNode* getNewParentProperty() const { return newParent_; }
        /** @return The node's value before the change. */
        [[nodiscard]] const std::string& getOldValueProperty() const { return oldValue_; }
        /** @return The node's value after the change. */
        [[nodiscard]] const std::string& getNewValueProperty() const { return newValue_; }
    };

    /** @brief Delegate invoked for XmlDocument node-change notifications. */
    using XmlNodeChangedEventHandler = std::function<void(void* sender, XmlNodeChangedEventArgs& e)>;

} // namespace System::Xml
