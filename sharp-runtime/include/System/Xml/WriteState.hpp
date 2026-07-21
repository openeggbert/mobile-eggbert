// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies the state of an XmlWriter.
     *
     * C++ counterpart of .NET System.Xml.WriteState.
     */
    enum class WriteState {
        /** Nothing has been written yet. */
        Start = 0,
        /** Writing the prolog. */
        Prolog = 1,
        /** Writing an element start tag. */
        Element = 2,
        /** Writing an attribute value. */
        Attribute = 3,
        /** Writing element content. */
        Content = 4,
        /** Close() has been called. */
        Closed = 5,
        /** The writer is in an error state. */
        Error = 6,
    };

} // namespace System::Xml
