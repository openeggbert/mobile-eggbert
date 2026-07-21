// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the type of node change reported by XmlDocument's NodeChanged/NodeChanging family.
     *
     * C++ counterpart of .NET System.Xml.XmlNodeChangedAction.
     */
    enum class XmlNodeChangedAction {
        /** A node is being/was inserted into the tree. */
        Insert = 0,
        /** A node is being/was removed from the tree. */
        Remove = 1,
        /** A node's value is being/was changed. */
        Change = 2,
    };

} // namespace System::Xml
