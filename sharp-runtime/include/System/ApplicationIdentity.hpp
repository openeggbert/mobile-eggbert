// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>

namespace System {

/**
 * @brief Provides the ability to uniquely identify a manifest-activated application.
 *
 * C++ counterpart of .NET System.ApplicationIdentity (sealed).
 * ISerializable is not implemented (sharp-runtime has no reflection/serialization).
 */
class ApplicationIdentity final {
    std::string fullName_;
    std::string codeBase_;

public:
    /**
     * @brief Initializes a new instance with the specified application identity full name.
     *
     * C++ counterpart of .NET ApplicationIdentity(string).
     * If the full name contains a '#' separator, the part after '#' is treated as CodeBase.
     * @param applicationIdentityFullName The full name of the application identity.
     */
    explicit ApplicationIdentity(const std::string& applicationIdentityFullName)
        : fullName_(applicationIdentityFullName)
    {
        auto pos = applicationIdentityFullName.find('#');
        if (pos != std::string::npos)
            codeBase_ = applicationIdentityFullName.substr(pos + 1);
    }

    /**
     * @brief Gets the URL location of the application.
     *
     * C++ counterpart of .NET ApplicationIdentity.CodeBase.
     */
    [[nodiscard]] const std::string& getCodeBaseProperty() const { return codeBase_; }

    /**
     * @brief Gets the full name of the application identity.
     *
     * C++ counterpart of .NET ApplicationIdentity.FullName.
     */
    [[nodiscard]] const std::string& getFullNameProperty() const { return fullName_; }

    /**
     * @brief Returns a string representation of the application identity.
     *
     * C++ counterpart of .NET ApplicationIdentity.ToString().
     */
    [[nodiscard]] std::string ToString() const { return fullName_; }
};

} // namespace System
