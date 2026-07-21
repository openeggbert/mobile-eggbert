// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlUrlResolver.hpp"

#include <fstream>
#include <sstream>

#include "System/Xml/XmlException.hpp"

namespace System::Xml {

    std::any XmlUrlResolver::GetEntity(const System::Uri& absoluteUri, const std::string&,
                                        const std::optional<std::string>&) {
        const std::string& scheme = absoluteUri.getSchemeProperty();
        if (!scheme.empty() && scheme != "file")
            throw XmlException("XmlUrlResolver::GetEntity: unsupported URI scheme '" + scheme +
                                "' (only local file access is supported).");

        const std::string& path = absoluteUri.getIsAbsoluteUriProperty()
            ? absoluteUri.getAbsolutePathProperty()
            : absoluteUri.getAbsoluteUriProperty();
        std::ifstream file(path, std::ios::binary);
        if (!file)
            throw XmlException("XmlUrlResolver::GetEntity: could not open '" + path + "'.");
        std::ostringstream ss;
        ss << file.rdbuf();
        return std::any(ss.str());
    }

} // namespace System::Xml
