// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlResolver.hpp"

namespace System::Xml {

    /**
     * @brief Resolves external XML resources named by a URI, reading from the local filesystem.
     *
     * C++ counterpart of .NET System.Xml.XmlUrlResolver. .NET's version can also fetch http(s)
     * URIs via the network stack; this runtime supports only `file://` and plain local paths
     * (no network stack for XML entity resolution) — a documented practical-subset limitation,
     * not a silent gap. Credentials/Proxy/CachePolicy (network-transport concerns) are omitted.
     */
    class XmlUrlResolver : public XmlResolver {
    public:
        /**
         * @brief Reads the file named by @p absoluteUri and returns its contents as a std::string.
         * @throws System::Xml::XmlException if @p absoluteUri's scheme is not "file" or empty, or the file cannot be opened.
         */
        [[nodiscard]] std::any GetEntity(const System::Uri& absoluteUri, const std::string& role,
                                          const std::optional<std::string>& ofObjectToReturn) override;
    };

} // namespace System::Xml
