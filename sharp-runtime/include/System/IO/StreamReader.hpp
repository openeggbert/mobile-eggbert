// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/IO/Stream.hpp"

namespace System::IO
{
    /**
     * @brief Reads characters from a Stream.
     *
     * This is a lightweight subset of the .NET StreamReader class: encoding
     * detection/BOM handling is not implemented, and bytes are read as raw
     * Latin-1/ASCII characters (no multi-byte decoding).
     *
     * @note Status: PARTIAL
     */
    class StreamReader
    {
    private:
        Stream* stream_;
        bool    leaveOpen_;
        bool    ownsStream_;
        bool    hasPeeked_ = false;
        bytecs  peeked_ = 0;

    public:
        /**
         * @brief Initializes a StreamReader for the specified stream.
         * @param stream Stream to read from.
         * @param leaveOpen If false (the default), the stream is closed when this StreamReader is destroyed.
         */
        explicit StreamReader(Stream* stream, bool leaveOpen = false);

        /** Destroys the StreamReader, closing the underlying stream unless leaveOpen was set. */
        ~StreamReader();

        /** Reads and returns the next character, or -1 at end. */
        intcs Read();

        /** Reads all remaining characters from the current position to the end. */
        [[nodiscard]] std::string ReadToEnd();
    };
}
