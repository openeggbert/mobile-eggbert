// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>

#include "System/Xml/XmlWriterSettings.hpp"

namespace System::Xml {

    struct XmlWriterState; ///< Opaque tinyxml2 state; defined in XmlWriter.cpp.

    /**
     * @brief Represents a writer that provides a fast, non-cached, forward-only way to generate XML.
     *
     * Builds a tinyxml2 DOM tree in memory.  Call @c ToString() to obtain the
     * serialized XML, or @c Close() to write it to the file supplied to @c Create().
     *
     * Partial C++ counterpart of .NET System.Xml.XmlWriter.
     *
     * @note Status: IMPLEMENTED — backed by vendored tinyxml2.
     */
    class XmlWriter {
        std::unique_ptr<XmlWriterState> state_;

        explicit XmlWriter(std::unique_ptr<XmlWriterState> s);

    public:
        ~XmlWriter();

        /**
         * @brief Writes the XML declaration (@c <?xml version="1.0"?>).
         *
         * Optional; may be called at most once, before any elements.
         */
        void WriteStartDocument();

        /** @brief Finalises the document (no-op for tinyxml2 DOM, flushes to file if path was set). */
        void WriteEndDocument();

        /**
         * @brief Opens a new element with the given local name.
         *
         * @param localName  Element tag name.
         */
        void WriteStartElement(const std::string& localName);

        /**
         * @brief Closes the most recently opened element.
         */
        void WriteEndElement();

        /**
         * @brief Writes an attribute on the current element.
         *
         * Must be called immediately after @c WriteStartElement, before any child
         * content or @c WriteEndElement.
         *
         * @param name   Attribute name.
         * @param value  Attribute value.
         */
        void WriteAttributeString(const std::string& name, const std::string& value);

        /**
         * @brief Writes a text node as child of the current element.
         *
         * @param text  Text content (will be XML-escaped).
         */
        void WriteString(const std::string& text);

        /**
         * @brief Writes a complete element: @c <name>value</name>.
         *
         * Equivalent to WriteStartElement + WriteString + WriteEndElement.
         *
         * @param name   Element tag name.
         * @param value  Text content.
         */
        void WriteElementString(const std::string& name, const std::string& value);

        /**
         * @brief Writes a comment node: @c <!-- text -->.
         *
         * If @p text contains @c "--" or ends with @c '-', a space is silently inserted to
         * keep the emitted markup well-formed, matching real .NET's
         * @c XmlEncodedRawTextWriter (which self-heals rather than throwing).
         *
         * @param text  Comment text.
         */
        void WriteComment(const std::string& text);

        /**
         * @brief Writes a CDATA section as child of the current element: @c <![CDATA[text]]>.
         *
         * If @p text contains an embedded @c "]]>", the section is silently split into
         * adjacent CDATA sections around it (matching real .NET) so the terminator never
         * appears mid-content; the original text round-trips unchanged on read-back.
         *
         * @param text  Content; not XML-escaped (that is the point of a CDATA section).
         */
        void WriteCData(const std::string& text);

        /**
         * @brief Writes a processing instruction: @c <?target data?>.
         *
         * If @p data contains @c "?>", a space is silently inserted to keep the emitted
         * markup well-formed, matching real .NET's @c XmlEncodedRawTextWriter.
         *
         * @param target  The PI target name.
         * @param data    The PI content (not XML-escaped, matching real XML PI syntax).
         */
        void WriteProcessingInstruction(const std::string& target, const std::string& data);

        /**
         * @brief Writes a document type declaration: @c <!DOCTYPE name PUBLIC "..." "..." [subset]>.
         *
         * @param name           The DOCTYPE root element name.
         * @param publicId       The public identifier, or "" to omit.
         * @param systemId       The system identifier, or "" to omit.
         * @param internalSubset The internal subset, or "" to omit.
         */
        void WriteDocType(const std::string& name, const std::string& publicId,
                           const std::string& systemId, const std::string& internalSubset);

        /**
         * @brief Returns the serialized XML as a string.
         *
         * Includes the XML declaration if @c WriteStartDocument() was called. Compact
         * (no inserted whitespace) unless the writer was created with
         * @c XmlWriterSettings::Indent set to @c true, matching real .NET's default.
         */
        [[nodiscard]] std::string ToString() const;

        /** @brief Flushes and, if a file path was provided to @c Create(), saves to that file. */
        void Flush();

        /** @brief Same as @c Flush(); releases resources. */
        void Close();

        /**
         * @brief Creates an XmlWriter that serializes to a file.
         *
         * Call @c Close() or @c Flush() to write the file.
         *
         * @param outputFileName  Destination file path.
         * @param settings        Formatting settings; only @c Indent is currently consulted
         *                        (matches real @c XmlWriterSettings' default of @c false —
         *                        compact, non-pretty-printed output).
         * @return Heap-allocated XmlWriter; caller owns the pointer.
         */
        static XmlWriter* Create(const std::string& outputFileName,
                                  const XmlWriterSettings& settings = XmlWriterSettings());

        /**
         * @brief Creates an XmlWriter that serializes to an in-memory string.
         *
         * Use @c ToString() to retrieve the result.
         *
         * @param settings  Formatting settings; only @c Indent is currently consulted
         *                  (matches real @c XmlWriterSettings' default of @c false —
         *                  compact, non-pretty-printed output).
         * @return Heap-allocated XmlWriter; caller owns the pointer.
         */
        static XmlWriter* CreateToString(const XmlWriterSettings& settings = XmlWriterSettings());
    };

} // namespace System::Xml
