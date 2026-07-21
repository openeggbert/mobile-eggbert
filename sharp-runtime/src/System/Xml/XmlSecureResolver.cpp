// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Xml/XmlSecureResolver.hpp"

#include "System/Xml/XmlException.hpp"

namespace System::Xml {

    std::any XmlSecureResolver::GetEntity(const System::Uri& absoluteUri, const std::string&,
                                           const std::optional<std::string>&) {
        throw XmlException("XmlSecureResolver::GetEntity: external entity resolution for '" +
                            absoluteUri.getAbsoluteUriProperty() + "' is forbidden.");
    }

} // namespace System::Xml
