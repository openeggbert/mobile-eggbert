// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies how whitespace is handled by the legacy XmlTextReader.
     *
     * C++ counterpart of .NET System.Xml.WhitespaceHandling.
     */
    enum class WhitespaceHandling {
        /** Return all Whitespace and SignificantWhitespace nodes (the default). */
        All = 0,
        /** Return only SignificantWhitespace nodes (in scope of xml:space="preserve"). */
        Significant = 1,
        /** Suppress all Whitespace and SignificantWhitespace nodes. */
        None = 2,
    };

} // namespace System::Xml
