// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies formatting options for the legacy XmlTextWriter.
     *
     * C++ counterpart of .NET System.Xml.Formatting.
     */
    enum class Formatting {
        /** No special formatting is applied (the default). */
        None = 0,
        /** Child elements are indented using IndentChar/Indentation settings. */
        Indented = 1,
    };

} // namespace System::Xml
