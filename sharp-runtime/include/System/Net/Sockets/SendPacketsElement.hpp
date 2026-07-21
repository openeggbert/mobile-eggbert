// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System::Net::Sockets {

    /**
     * @brief Wraps a file path or in-memory buffer to be sent as part of a scatter/gather send.
     *
     * C++ counterpart of .NET System.Net.Sockets.SendPacketsElement.
     *
     * @note Reduced scope: only the file-path and byte-buffer constructors are reproduced. The
     * `FileStream`-based constructors are not, since the consuming API
     * (`Socket::SendPacketsAsync`, a Windows `TRANSMIT_PACKETS`-based scatter/gather send) is
     * itself out of scope in this runtime's `Socket` port — implementing this data holder more
     * fully than its only consumer would be speculative.
     */
    class SendPacketsElement {
        std::optional<std::string> filePath_;
        std::vector<SharpRuntime::bytecs> buffer_;
        SharpRuntime::longcs offset_ = 0;
        SharpRuntime::intcs count_ = 0;
        bool endOfPacket_ = false;

    public:
        /** @brief Constructs a file element sending the entire file at @p filePath. */
        explicit SendPacketsElement(const std::string& filePath) : SendPacketsElement(filePath, 0, 0, false) {}

        /** @brief Constructs a file element sending @p count bytes starting at @p offset. */
        SendPacketsElement(const std::string& filePath, SharpRuntime::longcs offset, SharpRuntime::intcs count,
                            bool endOfPacket = false)
            : filePath_(filePath), offset_(offset), count_(count), endOfPacket_(endOfPacket) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(offset, "offset");
            System::ArgumentOutOfRangeException::ThrowIfNegative(count, "count");
        }

        /** @brief Constructs a buffer element sending the entire buffer. */
        explicit SendPacketsElement(std::vector<SharpRuntime::bytecs> buffer)
            : SendPacketsElement(std::move(buffer), 0, -1, false) {}

        /** @brief Constructs a buffer element sending @p count bytes starting at @p offset. */
        SendPacketsElement(std::vector<SharpRuntime::bytecs> buffer, SharpRuntime::intcs offset,
                            SharpRuntime::intcs count, bool endOfPacket = false)
            : buffer_(std::move(buffer)), offset_(offset), endOfPacket_(endOfPacket) {
            SharpRuntime::intcs actualCount = count >= 0 ? count : static_cast<SharpRuntime::intcs>(buffer_.size());
            System::ArgumentOutOfRangeException::ThrowIfGreaterThan(static_cast<SharpRuntime::uintcs>(offset),
                                                                     static_cast<SharpRuntime::uintcs>(buffer_.size()),
                                                                     "offset");
            System::ArgumentOutOfRangeException::ThrowIfGreaterThan(
                static_cast<SharpRuntime::uintcs>(actualCount),
                static_cast<SharpRuntime::uintcs>(buffer_.size() - static_cast<size_t>(offset)), "count");
            count_ = actualCount;
        }

        /** @return The file path, if this is a file element. */
        [[nodiscard]] const std::optional<std::string>& getFilePathProperty() const { return filePath_; }

        /** @return The buffer, if this is a buffer element. */
        [[nodiscard]] const std::vector<SharpRuntime::bytecs>& getBufferProperty() const { return buffer_; }

        /** @return The number of bytes to send. */
        [[nodiscard]] SharpRuntime::intcs getCountProperty() const { return count_; }

        /** @return The 64-bit offset into the file/buffer at which to start sending. */
        [[nodiscard]] SharpRuntime::longcs getOffsetLongProperty() const { return offset_; }

        /** @return The offset into the file/buffer at which to start sending, truncated to 32 bits. */
        [[nodiscard]] SharpRuntime::intcs getOffsetProperty() const { return static_cast<SharpRuntime::intcs>(offset_); }

        /** @return true if a packet boundary should be forced after this element. */
        [[nodiscard]] bool getEndOfPacketProperty() const { return endOfPacket_; }
    };

} // namespace System::Net::Sockets
