// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include "System/Security/Cryptography/CryptographicException.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief Specifies the name of a cryptographic hash algorithm.
     *
     * C++ counterpart of .NET System.Security.Cryptography.HashAlgorithmName (a readonly struct
     * open-ended over any string, with the discoverable feel of an enum for common values).
     */
    class HashAlgorithmName {
        std::optional<std::string> name_;

    public:
        HashAlgorithmName() = default;

        /** @brief Constructs a HashAlgorithmName from a custom name. */
        explicit HashAlgorithmName(std::optional<std::string> name) : name_(std::move(name)) {}

        /** @return The underlying algorithm name, or empty if none is applicable. */
        [[nodiscard]] const std::optional<std::string>& getNameProperty() const { return name_; }

        [[nodiscard]] std::string ToString() const { return name_.value_or(""); }

        [[nodiscard]] bool Equals(const HashAlgorithmName& other) const { return name_ == other.name_; }
        bool operator==(const HashAlgorithmName& o) const { return Equals(o); }
        bool operator!=(const HashAlgorithmName& o) const { return !Equals(o); }

        static const HashAlgorithmName MD5;
        static const HashAlgorithmName SHA1;
        static const HashAlgorithmName SHA256;
        static const HashAlgorithmName SHA384;
        static const HashAlgorithmName SHA512;
        static const HashAlgorithmName SHA3_256;
        static const HashAlgorithmName SHA3_384;
        static const HashAlgorithmName SHA3_512;

        /** @brief Attempts to convert @p oidValue to a well-known HashAlgorithmName. */
        [[nodiscard]] static bool TryFromOid(const std::string& oidValue, HashAlgorithmName& value) {
            if (oidValue == "1.2.840.113549.2.5") { value = MD5; return true; }
            if (oidValue == "1.3.14.3.2.26") { value = SHA1; return true; }
            if (oidValue == "2.16.840.1.101.3.4.2.1") { value = SHA256; return true; }
            if (oidValue == "2.16.840.1.101.3.4.2.2") { value = SHA384; return true; }
            if (oidValue == "2.16.840.1.101.3.4.2.3") { value = SHA512; return true; }
            if (oidValue == "2.16.840.1.101.3.4.2.8") { value = SHA3_256; return true; }
            if (oidValue == "2.16.840.1.101.3.4.2.9") { value = SHA3_384; return true; }
            if (oidValue == "2.16.840.1.101.3.4.2.10") { value = SHA3_512; return true; }
            value = HashAlgorithmName();
            return false;
        }

        /**
         * @brief Converts @p oidValue to a well-known HashAlgorithmName.
         * @throws CryptographicException if @p oidValue does not represent a known hash algorithm.
         */
        [[nodiscard]] static HashAlgorithmName FromOid(const std::string& oidValue) {
            HashAlgorithmName value;
            if (TryFromOid(oidValue, value)) {
                return value;
            }
            throw CryptographicException("The specified OID (" + oidValue + ") does not represent a known hash algorithm.");
        }
    };

    inline const HashAlgorithmName HashAlgorithmName::MD5{std::string("MD5")};
    inline const HashAlgorithmName HashAlgorithmName::SHA1{std::string("SHA1")};
    inline const HashAlgorithmName HashAlgorithmName::SHA256{std::string("SHA256")};
    inline const HashAlgorithmName HashAlgorithmName::SHA384{std::string("SHA384")};
    inline const HashAlgorithmName HashAlgorithmName::SHA512{std::string("SHA512")};
    inline const HashAlgorithmName HashAlgorithmName::SHA3_256{std::string("SHA3-256")};
    inline const HashAlgorithmName HashAlgorithmName::SHA3_384{std::string("SHA3-384")};
    inline const HashAlgorithmName HashAlgorithmName::SHA3_512{std::string("SHA3-512")};

} // namespace System::Security::Cryptography
