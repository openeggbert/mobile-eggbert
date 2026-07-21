// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <memory>
#include <string>

#include "System/Xml/NameTable.hpp"

namespace System::Xml {

    class XmlDocument;

    /**
     * @brief Provides methods for performing operations independent of any particular document
     * instance (DOM Level 1 IDOMImplementation).
     *
     * C++ counterpart of .NET System.Xml.XmlImplementation.
     */
    class XmlImplementation {
        std::shared_ptr<XmlNameTable> nameTable_;

    public:
        XmlImplementation();
        explicit XmlImplementation(std::shared_ptr<XmlNameTable> nt);
        // CreateDocument() is virtual, so deleting a derived instance through a
        // std::unique_ptr<XmlImplementation> (as XmlDocument does) requires a virtual
        // destructor to avoid undefined behavior.
        virtual ~XmlImplementation() = default;

        /**
         * @brief Tests whether the DOM implementation supports a specific feature.
         *
         * C++ counterpart of .NET XmlImplementation.HasFeature(string, string).
         * @param strFeature The feature name to test (case-insensitive; only "XML" is recognized).
         * @param strVersion The feature version to test ("1.0", "2.0", or empty for "any version").
         * @return true if @p strFeature is "XML" and @p strVersion is empty, "1.0", or "2.0".
         */
        [[nodiscard]] bool HasFeature(const std::string& strFeature, const std::string& strVersion) const;

        /** @return A new, empty XmlDocument sharing this implementation's NameTable. */
        [[nodiscard]] virtual std::unique_ptr<XmlDocument> CreateDocument() const;
    };

} // namespace System::Xml
