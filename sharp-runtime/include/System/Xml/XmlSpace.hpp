// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the xml:space scope in effect for the current node.
     *
     * C++ counterpart of .NET System.Xml.XmlSpace.
     */
    enum class XmlSpace {
        /** No xml:space scope has been specified. */
        None = 0,
        /** xml:space="default". */
        Default = 1,
        /** xml:space="preserve". */
        Preserve = 2,
    };

} // namespace System::Xml
