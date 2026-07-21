// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Attribute.hpp"

namespace System::Runtime::Versioning {

    /** Identifies the version of the .NET framework targeted by the assembly. */
    class TargetFrameworkAttribute : public System::Attribute {
        std::string frameworkName_;
        std::string frameworkDisplayName_;
    public:
        /** @param frameworkName The framework moniker (e.g. ".NETCoreApp,Version=v8.0"). */
        explicit TargetFrameworkAttribute(const std::string& frameworkName)
            : frameworkName_(frameworkName) {}

        /** @return The framework moniker string. */
        [[nodiscard]] const std::string& getFrameworkNameProperty()        const { return frameworkName_; }

        /** @return The human-readable framework display name, or empty if unset. */
        [[nodiscard]] const std::string& getFrameworkDisplayNameProperty() const { return frameworkDisplayName_; }

        /** Sets the human-readable framework display name. */
        void setFrameworkDisplayNameProperty(const std::string& v) { frameworkDisplayName_ = v; }
    };

    /** Indicates that an API is supported on the specified OS platform. */
    class SupportedOSPlatformAttribute : public System::Attribute {
        std::string platformName_;
    public:
        /** @param platformName Platform identifier (e.g. "windows", "linux10.0"). */
        explicit SupportedOSPlatformAttribute(const std::string& platformName)
            : platformName_(platformName) {}

        /** @return The platform identifier. */
        [[nodiscard]] const std::string& getPlatformNameProperty() const { return platformName_; }
    };

    /** Indicates that an API is not supported on the specified OS platform. */
    class UnsupportedOSPlatformAttribute : public System::Attribute {
        std::string platformName_;
        std::string message_;
    public:
        /** @param platformName Platform identifier (e.g. "windows"). */
        explicit UnsupportedOSPlatformAttribute(const std::string& platformName)
            : platformName_(platformName) {}

        /**
         * @param platformName Platform identifier (e.g. "windows").
         * @param message      Optional message explaining the lack of support.
         */
        UnsupportedOSPlatformAttribute(const std::string& platformName, const std::string& message)
            : platformName_(platformName), message_(message) {}

        /** @return The platform identifier. */
        [[nodiscard]] const std::string& getPlatformNameProperty() const { return platformName_; }

        /** @return The explanatory message, or empty if not provided. */
        [[nodiscard]] const std::string& getMessageProperty() const { return message_; }
    };

    /**
     * Annotates a custom guard field, property, or method with a supported platform name, for use
     * in conditionals/asserts that guard calls to platform-specific APIs.
     */
    class SupportedOSPlatformGuardAttribute : public System::Attribute {
        std::string platformName_;
    public:
        /** @param platformName Platform identifier the guard indicates support for. */
        explicit SupportedOSPlatformGuardAttribute(const std::string& platformName)
            : platformName_(platformName) {}

        /** @return The platform identifier. */
        [[nodiscard]] const std::string& getPlatformNameProperty() const { return platformName_; }
    };

    /**
     * Annotates a custom guard field, property, or method with an unsupported platform name, for
     * use in conditionals/asserts that guard against calling unsupported platform-specific APIs.
     */
    class UnsupportedOSPlatformGuardAttribute : public System::Attribute {
        std::string platformName_;
    public:
        /** @param platformName Platform identifier the guard indicates lack of support for. */
        explicit UnsupportedOSPlatformGuardAttribute(const std::string& platformName)
            : platformName_(platformName) {}

        /** @return The platform identifier. */
        [[nodiscard]] const std::string& getPlatformNameProperty() const { return platformName_; }
    };

    /** Indicates that an API has been obsoleted on the specified OS platform. */
    class ObsoletedOSPlatformAttribute : public System::Attribute {
        std::string platformName_;
        std::string message_;
        std::string url_;
    public:
        /**
         * @param platformName Platform on which the API is obsolete.
         * @param message      Optional deprecation message.
         * @param url          Optional URL with migration guidance.
         */
        explicit ObsoletedOSPlatformAttribute(const std::string& platformName,
                                               const std::string& message = {},
                                               const std::string& url = {})
            : platformName_(platformName), message_(message), url_(url) {}

        /** @return The platform identifier. */
        [[nodiscard]] const std::string& getPlatformNameProperty() const { return platformName_; }

        /** @return The deprecation message, or empty if not provided. */
        [[nodiscard]] const std::string& getMessageProperty()      const { return message_; }

        /** @return The migration URL, or empty if not provided. */
        [[nodiscard]] const std::string& getUrlProperty()          const { return url_; }
    };

    /** Marks an API as requiring preview features that may change in future releases. */
    class RequiresPreviewFeaturesAttribute : public System::Attribute {
        std::string message_;
        std::string url_;
    public:
        /** Default constructor — no message or URL. */
        RequiresPreviewFeaturesAttribute() = default;

        /**
         * @param message Explanation of why the API is preview.
         * @param url     Optional URL with more information.
         */
        explicit RequiresPreviewFeaturesAttribute(const std::string& message, const std::string& url = {})
            : message_(message), url_(url) {}

        /** @return The informational message, or empty if not provided. */
        [[nodiscard]] const std::string& getMessageProperty() const { return message_; }

        /** @return The informational URL, or empty if not provided. */
        [[nodiscard]] const std::string& getUrlProperty()     const { return url_; }
    };

} // namespace System::Runtime::Versioning
