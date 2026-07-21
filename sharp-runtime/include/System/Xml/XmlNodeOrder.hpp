// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Describes the relative document-order relationship between two nodes.
     *
     * C++ counterpart of .NET System.Xml.XmlNodeOrder.
     */
    enum class XmlNodeOrder {
        /** The reference node comes before the comparand in document order. */
        Before = 0,
        /** The reference node comes after the comparand in document order. */
        After = 1,
        /** The two nodes are the same node. */
        Same = 2,
        /** The relative order cannot be determined (e.g. the nodes are in different trees). */
        Unknown = 3,
    };

} // namespace System::Xml
