// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/Xml/ConformanceLevel.hpp"
#include "System/Xml/NewLineHandling.hpp"

namespace System::Xml {

    /**
     * @brief Specifies a set of features to support on an XmlWriter created by XmlWriter::Create.
     *
     * C++ counterpart of .NET System.Xml.XmlWriterSettings. Held as a plain property bag; this
     * runtime's XmlWriter is a single concrete tinyxml2-backed implementation, so most properties
     * here are storage for API-name compatibility rather than switches the writer itself
     * currently consults (each such property documents this in its own comment, not silently).
     */
    class XmlWriterSettings {
    public:
        /** @return A new XmlWriterSettings with the same property values as this one. */
        [[nodiscard]] XmlWriterSettings Clone() const { return *this; }

        /** @brief Resets every property to its default value. */
        void Reset() { *this = XmlWriterSettings(); }

        /** @brief Whether asynchronous methods may be used. Not currently consulted (no async I/O path). */
        bool Async = false;

        /** @brief Whether to omit the XML declaration. Not currently consulted; use WriteStartDocument() explicitly to emit one. */
        bool OmitXmlDeclaration = false;
        /** @brief How line breaks are normalized. Not currently consulted (tinyxml2 controls output line breaks). */
        System::Xml::NewLineHandling NewLineHandling = System::Xml::NewLineHandling::Replace;
        /** @brief The character string to use for line breaks. Not currently consulted. */
        std::string NewLineChars = "\r\n";
        /** @brief Whether to indent elements. Consulted by @c XmlWriter::ToString()/Flush(): @c false
         *  (the default) emits compact output with no inserted whitespace; @c true pretty-prints
         *  via tinyxml2's own indentation (which does not consult @c IndentChars). */
        bool Indent = false;
        /** @brief The character string to use when indenting. Not currently consulted (tinyxml2's
         *  own fixed-width indentation is used regardless of @c Indent). */
        std::string IndentChars = "  ";
        /** @brief Whether to write attributes on a new line. Not currently consulted. */
        bool NewLineOnAttributes = false;
        /** @brief Whether the underlying stream/file should be closed when the writer is closed. */
        bool CloseOutput = false;
        /** @brief The conformance level the writer is expected to enforce. Not currently enforced. */
        System::Xml::ConformanceLevel ConformanceLevel = System::Xml::ConformanceLevel::Document;
        /** @brief Whether to check that XML content is legal per the XML specification. Not currently enforced. */
        bool CheckCharacters = true;
        /** @brief Whether Close() implicitly calls WriteEndDocument(). */
        bool WriteEndDocumentOnClose = true;
    };

} // namespace System::Xml
