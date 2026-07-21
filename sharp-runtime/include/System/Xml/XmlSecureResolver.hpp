// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Xml/XmlResolver.hpp"

namespace System::Xml {

    /**
     * @brief An XmlResolver that forbids all external entity resolution.
     *
     * C++ counterpart of .NET System.Xml.XmlSecureResolver. Matches modern .NET's actual
     * (security-hardened) behavior: the constructor's `resolver`/`securityUrl` arguments are
     * accepted for API-name compatibility with old call sites but no longer change any
     * behavior — GetEntity always throws XmlException, exactly like the wrapped resolver
     * argument's own settings are ignored in real .NET since the XXE-hardening change made
     * XmlSecureResolver equivalent to XmlResolver::ThrowingResolver.
     */
    class XmlSecureResolver : public XmlResolver {
    public:
        XmlSecureResolver(XmlResolver& resolver, const std::string& securityUrl) {
            (void)resolver;
            (void)securityUrl;
        }

        /** @throws System::Xml::XmlException always. */
        [[nodiscard]] std::any GetEntity(const System::Uri& absoluteUri, const std::string& role,
                                          const std::optional<std::string>& ofObjectToReturn) override;
    };

} // namespace System::Xml
