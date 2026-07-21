// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml::Linq {

    /**
     * @brief Describes the type of change to an XML tree that produced an XObjectChangeEventArgs.
     *
     * C++ counterpart of .NET System.Xml.Linq.XObjectChange.
     *
     * @note This runtime does not raise these events (see XObject's doc comment) — reproduced
     * for API completeness/data-holding use.
     */
    enum class XObjectChange {
        Add,
        Remove,
        Name,
        Value,
    };

} // namespace System::Xml::Linq
