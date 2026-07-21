// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/PlatformNotSupportedException.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Represents a cryptographic object identifier (OID).
     *
     * C++ counterpart of .NET System.Security.Cryptography.Oid.
     *
     * @note Reduced scope: .NET's single-string constructor, `FromFriendlyName()`, and the
     * auto-resolving `FriendlyName`/`Value` getters all go through `Internal.Cryptography.
     * OidLookup` — a large platform OID name/value database that is out of scope in this
     * codebase (`plan.sqlite3`: `OidLookup`/`OidGroup`/`Oids` all marked `ignore`). This port
     * keeps the "initialize once" `Value`/`FriendlyName` set semantics (still useful for
     * round-tripping an OID a caller already knows both parts of) but does not look up one part
     * from the other.
     */
    class Oid {
        std::optional<std::string> value_;
        std::optional<std::string> friendlyName_;

    public:
        Oid() = default;

        /** @brief Constructs with both the dotted-decimal value and friendly name already known. */
        Oid(std::optional<std::string> value, std::optional<std::string> friendlyName)
            : value_(std::move(value)), friendlyName_(std::move(friendlyName)) {}

        /** @return The dotted-decimal OID value (e.g. "1.2.840.113549.1.1.1"), if set. */
        [[nodiscard]] const std::optional<std::string>& getValueProperty() const { return value_; }

        /**
         * @brief Sets the OID value. Once set, may only be set again to the same value.
         * @throws System::PlatformNotSupportedException if already set to a different value.
         */
        void setValueProperty(std::optional<std::string> value) {
            if (value_.has_value() && value_ != value) {
                throw System::PlatformNotSupportedException("The OID value can only be set once.");
            }
            value_ = std::move(value);
        }

        /** @return The friendly (display) name of the OID, if set. */
        [[nodiscard]] const std::optional<std::string>& getFriendlyNameProperty() const { return friendlyName_; }

        /**
         * @brief Sets the friendly name. Once set, may only be set again to the same value.
         * @throws System::PlatformNotSupportedException if already set to a different value.
         */
        void setFriendlyNameProperty(std::optional<std::string> friendlyName) {
            if (friendlyName_.has_value() && friendlyName_ != friendlyName) {
                throw System::PlatformNotSupportedException("The OID friendly name can only be set once.");
            }
            friendlyName_ = std::move(friendlyName);
        }
    };

} // namespace System::Security::Cryptography
