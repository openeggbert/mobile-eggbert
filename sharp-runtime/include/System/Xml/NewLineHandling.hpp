// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Xml {

    /**
     * @brief Specifies how an XmlWriter handles line breaks.
     *
     * C++ counterpart of .NET System.Xml.NewLineHandling.
     */
    enum class NewLineHandling {
        /** Normalizes all line breaks to XmlWriterSettings::NewLineChars. */
        Replace = 0,
        /** Replaces line breaks that would be normalized away by a normalizing reader with character entities. */
        Entitize = 1,
        /** Line breaks are written unchanged. */
        None = 2,
    };

} // namespace System::Xml
