// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <any>
#include <optional>
#include <string>

#include "System/Uri.hpp"

namespace System::Xml {

    /**
     * @brief Resolves external XML resources named by a URI, such as external DTD subsets or
     * XML includes.
     *
     * C++ counterpart of .NET System.Xml.XmlResolver (abstract base). @c ofObjectToReturn (a
     * .NET `Type?` requesting a specific CLR return type) has no meaningful analogue without
     * runtime reflection, so it is accepted but ignored; implementations always return the
     * resolved content as a `std::string` wrapped in `std::any`.
     */
    class XmlResolver {
    public:
        virtual ~XmlResolver() = default;

        /**
         * @brief Maps a URI to an object containing the resolved resource.
         * @param absoluteUri  The URI to resolve.
         * @param role  Current implementations ignore this; reserved for future use, per .NET's own contract.
         * @param ofObjectToReturn  Ignored (see class doc-comment); this runtime always returns a std::string.
         * @return The resolved content, or an empty std::any if the resource could not be resolved.
         */
        [[nodiscard]] virtual std::any GetEntity(const System::Uri& absoluteUri, const std::string& role,
                                                  const std::optional<std::string>& ofObjectToReturn) = 0;

        /**
         * @brief Resolves the absolute URI from a base and relative URI.
         * @param baseUri  The base URI, or nullopt if none.
         * @param relativeUri  The URI to resolve, which may be relative or absolute.
         */
        [[nodiscard]] virtual System::Uri ResolveUri(const std::optional<System::Uri>& baseUri,
                                                       const std::string& relativeUri) const;
    };

} // namespace System::Xml
