// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlResolver.hpp"
#include "System/NotSupportedException.hpp"

namespace System::Xml {

    System::Uri XmlResolver::ResolveUri(const std::optional<System::Uri>& baseUri,
                                         const std::string& relativeUri) const {
        if (!baseUri.has_value())
            return System::Uri(relativeUri, UriKind::RelativeOrAbsolute);
        if (relativeUri.empty())
            return *baseUri;
        if (!baseUri->getIsAbsoluteUriProperty())
            throw System::NotSupportedException("Relative URIs are not supported.");
        return System::Uri(*baseUri, relativeUri);
    }

} // namespace System::Xml
