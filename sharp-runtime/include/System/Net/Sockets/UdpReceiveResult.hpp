// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string_view>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Net/IPEndPoint.hpp"

namespace System::Net::Sockets {

    /**
     * @brief Presents UDP receive result information from UdpClient::ReceiveAsync().
     *
     * C++ counterpart of .NET System.Net.Sockets.UdpReceiveResult.
     */
    class UdpReceiveResult {
        std::vector<SharpRuntime::bytecs> buffer_;
        System::Net::IPEndPoint remoteEndPoint_;

    public:
        UdpReceiveResult() = default;

        UdpReceiveResult(std::vector<SharpRuntime::bytecs> buffer, System::Net::IPEndPoint remoteEndPoint)
            : buffer_(std::move(buffer)), remoteEndPoint_(std::move(remoteEndPoint)) {}

        /** @return The buffer with the data received in the UDP packet. */
        [[nodiscard]] const std::vector<SharpRuntime::bytecs>& getBufferProperty() const { return buffer_; }

        /** @return The remote endpoint from which the UDP packet was received. */
        [[nodiscard]] System::Net::IPEndPoint getRemoteEndPointProperty() const { return remoteEndPoint_; }

        [[nodiscard]] bool Equals(const UdpReceiveResult& other) const {
            return buffer_ == other.buffer_ && remoteEndPoint_ == other.remoteEndPoint_;
        }
        bool operator==(const UdpReceiveResult& o) const { return Equals(o); }
        bool operator!=(const UdpReceiveResult& o) const { return !Equals(o); }

        /**
         * @brief C++ counterpart of .NET UdpReceiveResult.GetHashCode().
         * @note Real .NET combines the buffer's *reference-identity* hash (byte[] doesn't
         * override GetHashCode, so it's Object.GetHashCode()) with the endpoint's hash via XOR,
         * and returns 0 when the buffer reference is null (the default-constructed struct's
         * state). A std::vector value type has no equivalent reference-identity or null concept
         * -- an empty vector is still a valid, hashable value -- so this combines a content-based
         * hash of the buffer with the endpoint's hash instead, unconditionally. This is
         * semantically closer to Equals() (which is also content-based here, unlike .NET's
         * reference-based array equality) than to real .NET's exact bit-for-bit hash value.
         */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
            std::size_t bufferHash = std::hash<std::string_view>{}(
                std::string_view(reinterpret_cast<const char*>(buffer_.data()), buffer_.size()));
            return static_cast<SharpRuntime::intcs>(bufferHash) ^ remoteEndPoint_.GetHashCode();
        }
    };

} // namespace System::Net::Sockets
