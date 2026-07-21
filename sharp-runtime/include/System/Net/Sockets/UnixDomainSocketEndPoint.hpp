// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>
#include "System/Net/EndPoint.hpp"
#include "System/Net/Sockets/AddressFamily.hpp"
#include "System/String.hpp"

namespace System::Net::Sockets {

    /**
     * @brief Represents a Unix domain socket endpoint as a path.
     *
     * C++ counterpart of .NET System.Net.Sockets.UnixDomainSocketEndPoint.
     *
     * @note Adds one member beyond .NET's public surface: getPathProperty(), returning the raw
     * constructor-supplied path (no abstract-socket '@' translation). .NET keeps this internal
     * since its Socket implementation has direct field access; this runtime's Socket needs a
     * public way to read the path back out to build a native `sockaddr_un`.
     */
    class UnixDomainSocketEndPoint : public System::Net::EndPoint {
        std::string path_;

        static bool isAbstract(const std::string& path) { return !path.empty() && path[0] == '\0'; }

        // Private tag type: only code inside this class can name it, so the constructor overload
        // below — though public, to remain callable by std::make_shared from Create() — is
        // unreachable from outside the class. Used to round-trip a possibly-empty path recovered
        // from a native socket address (e.g. an unbound peer), which the validating constructor
        // would otherwise reject.
        struct UnvalidatedTag {};

    public:
        /**
         * @brief Constructs from a filesystem path (or an abstract-namespace name starting with
         * a NUL byte on Linux).
         * @throws System::ArgumentOutOfRangeException if @p path is empty or too long for a
         * native `sockaddr_un` (108 bytes on Linux, including the NUL terminator).
         * @throws System::PlatformNotSupportedException if this platform has no AF_UNIX support.
         */
        explicit UnixDomainSocketEndPoint(const std::string& path);

        /** @private Unvalidated constructor for internal round-tripping via Create(); not part of .NET's public surface. */
        UnixDomainSocketEndPoint(std::string path, UnvalidatedTag) : path_(std::move(path)) {}

        /** @return The path represented by this endpoint (with a leading "@" for abstract-namespace names). */
        [[nodiscard]] std::string ToString() const;

        /** @return The raw path as supplied to the constructor (no abstract-socket translation). */
        [[nodiscard]] const std::string& getPathProperty() const { return path_; }

        [[nodiscard]] System::Net::Sockets::AddressFamily getAddressFamilyProperty() const override {
            return System::Net::Sockets::AddressFamily::Unix;
        }

        [[nodiscard]] SocketAddress Serialize() const override;

        [[nodiscard]] std::shared_ptr<System::Net::EndPoint> Create(const SocketAddress& socketAddress) const override;

        [[nodiscard]] bool Equals(const UnixDomainSocketEndPoint& other) const { return path_ == other.path_; }
        bool operator==(const UnixDomainSocketEndPoint& o) const { return Equals(o); }
        bool operator!=(const UnixDomainSocketEndPoint& o) const { return !Equals(o); }

        /** @brief C++ counterpart of .NET UnixDomainSocketEndPoint.GetHashCode() -- _path.GetHashCode(). */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const { return System::String::GetHashCode(path_); }
    };

} // namespace System::Net::Sockets
