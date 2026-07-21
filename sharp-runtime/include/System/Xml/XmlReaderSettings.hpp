// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Xml/ConformanceLevel.hpp"
#include "System/Xml/DtdProcessing.hpp"

namespace System::Xml {

    /**
     * @brief Specifies a set of features to support on an XmlReader created by XmlReader::Create.
     *
     * C++ counterpart of .NET System.Xml.XmlReaderSettings. Held as a plain property bag; this
     * runtime's XmlReader is a single concrete tinyxml2-backed implementation (not an extensible
     * abstract hierarchy driven by settings), so most properties here are read/write storage for
     * API-name compatibility rather than switches the reader itself currently consults. Not a
     * silent gap: each such property says so in its own doc-comment.
     */
    class XmlReaderSettings {
    public:
        /** @return A new XmlReaderSettings with the same property values as this one. */
        [[nodiscard]] XmlReaderSettings Clone() const { return *this; }

        /** @brief Resets every property to its default value. */
        void Reset() { *this = XmlReaderSettings(); }

        /** @brief Whether asynchronous methods may be used. Not currently consulted by this runtime's reader (no async I/O path). */
        bool Async = false;

        /** @brief Offset added to line numbers reported by IXmlLineInfo. Not currently consulted. */
        SharpRuntime::intcs LineNumberOffset = 0;
        /** @brief Offset added to line positions reported by IXmlLineInfo. Not currently consulted. */
        SharpRuntime::intcs LinePositionOffset = 0;

        /** @brief The conformance level the reader is expected to enforce. Not currently enforced. */
        System::Xml::ConformanceLevel ConformanceLevel = System::Xml::ConformanceLevel::Document;
        /** @brief Whether to check that XML content is legal per the XML specification. Not currently enforced. */
        bool CheckCharacters = true;

        /** @brief Whether to skip insignificant whitespace. */
        bool IgnoreWhitespace = false;
        /** @brief Whether to skip processing instructions. */
        bool IgnoreProcessingInstructions = false;
        /** @brief Whether to skip comments. */
        bool IgnoreComments = false;

        /** @brief Whether encountering a DTD is treated as an error. */
        bool ProhibitDtd = false;
        /** @brief Whether/how DTDs are processed. This runtime does not perform DTD-driven validation or entity expansion regardless of this setting. */
        System::Xml::DtdProcessing DtdProcessing = System::Xml::DtdProcessing::Prohibit;

        /** @brief Whether the underlying stream/file should be closed when the reader is closed. */
        bool CloseInput = false;
    };

} // namespace System::Xml
