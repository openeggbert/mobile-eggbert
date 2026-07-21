// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>
#include <string>
#include "System/InvalidOperationException.hpp"

namespace System::Threading::Channels {

    /**
     * @brief Thrown when a channel is used after it has been closed.
     *
     * C++ counterpart of .NET System.Threading.Channels.ChannelClosedException.
     */
    class ChannelClosedException : public System::InvalidOperationException {
    public:
        ChannelClosedException() : InvalidOperationException("The channel has been closed.") {}
        explicit ChannelClosedException(const std::string& message) : InvalidOperationException(message) {}
        explicit ChannelClosedException(std::exception_ptr innerException)
            : InvalidOperationException("The channel has been closed.", innerException) {}
        ChannelClosedException(const std::string& message, std::exception_ptr innerException)
            : InvalidOperationException(message, innerException) {}
    };

} // namespace System::Threading::Channels
