// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Version.hpp"

namespace System {

    /**
     * @brief Provides a unique identity for a manifest-based application.
     *
     * C++ counterpart of .NET System.ApplicationId.
     * Stores the name, version, processor architecture, culture, and public key
     * token that together uniquely identify a ClickOnce application.
     */
    class ApplicationId {
        std::string name_;
        Version     version_;
        std::string processorArchitecture_;
        std::string culture_;
        std::string publicKeyToken_;

    public:
        /**
         * @brief Initializes a new instance with the specified identity components.
         *
         * C++ counterpart of .NET ApplicationId(byte[], string, Version, string, string).
         * @param publicKeyToken The application's public key token (as a string).
         * @param name           The application name.
         * @param version        The application version.
         * @param processorArchitecture The processor architecture ("x86", "amd64", etc.).
         * @param culture        The culture string ("neutral", "en-US", etc.).
         */
        ApplicationId(const std::string& publicKeyToken,
                      const std::string& name,
                      const Version& version,
                      const std::string& processorArchitecture,
                      const std::string& culture)
            : name_(name), version_(version),
              processorArchitecture_(processorArchitecture),
              culture_(culture), publicKeyToken_(publicKeyToken) {}

        /** @brief Gets the application name. C++ counterpart of .NET ApplicationId.Name. */
        [[nodiscard]] const std::string& getNameProperty() const { return name_; }

        /** @brief Gets the application version. C++ counterpart of .NET ApplicationId.Version. */
        [[nodiscard]] const Version& getVersionProperty() const { return version_; }

        /** @brief Gets the processor architecture. C++ counterpart of .NET ApplicationId.ProcessorArchitecture. */
        [[nodiscard]] const std::string& getProcessorArchitectureProperty() const {
            return processorArchitecture_;
        }

        /** @brief Gets the application culture. C++ counterpart of .NET ApplicationId.Culture. */
        [[nodiscard]] const std::string& getCultureProperty() const { return culture_; }

        /** @brief Gets the public key token. C++ counterpart of .NET ApplicationId.PublicKeyToken. */
        [[nodiscard]] const std::string& getPublicKeyTokenProperty() const { return publicKeyToken_; }

        /**
         * @brief Creates a copy of this ApplicationId.
         *
         * C++ counterpart of .NET ApplicationId.Copy().
         */
        [[nodiscard]] ApplicationId Copy() const { return *this; }

        /**
         * @brief Determines whether this instance and the specified object have the same value.
         *
         * C++ counterpart of .NET ApplicationId.Equals(object).
         * Two ApplicationId instances are equal when all five fields match.
         */
        [[nodiscard]] bool Equals(const ApplicationId& other) const {
            return name_                == other.name_
                && version_             == other.version_
                && processorArchitecture_ == other.processorArchitecture_
                && culture_             == other.culture_
                && publicKeyToken_      == other.publicKeyToken_;
        }

        bool operator==(const ApplicationId& o) const { return Equals(o); }
        bool operator!=(const ApplicationId& o) const { return !Equals(o); }

        /**
         * @brief Returns a hash code for this ApplicationId based on the name and version.
         *
         * C++ counterpart of .NET ApplicationId.GetHashCode().
         */
        [[nodiscard]] int GetHashCode() const noexcept {
            std::size_t h = std::hash<std::string>{}(name_);
            h ^= std::hash<std::string>{}(version_.ToString()) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return static_cast<int>(h);
        }

        /**
         * @brief Returns a string representation of the application identity.
         *
         * C++ counterpart of .NET ApplicationId.ToString().
         */
        [[nodiscard]] std::string ToString() const {
            return name_ + ", Version=" + version_.ToString()
                + ", Culture=" + culture_
                + ", ProcessorArchitecture=" + processorArchitecture_;
        }
    };

} // namespace System
