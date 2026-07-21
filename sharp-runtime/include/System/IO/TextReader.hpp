// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::IO {

    using SharpRuntime::intcs;

    /**
     * @brief Represents a reader that can read a sequential series of characters.
     *
     * Partial C++ counterpart of .NET System.IO.TextReader.
     *
     * @note Status: Partial
     */
    class TextReader {
    public:
        /** Destroys the TextReader. */
        virtual ~TextReader() = default;

        /** @brief Reads the next character without changing the state of the reader. Returns -1 at end. */
        [[nodiscard]] virtual intcs Peek() { return -1; }

        /** @brief Reads the next character. Returns -1 at end. */
        virtual intcs Read() { return -1; }

        /** @brief Reads a line of characters. Returns empty string at end-of-stream. */
        [[nodiscard]] virtual std::string ReadLine() { return ""; }

        /** @brief Reads all characters from the current position to the end. */
        [[nodiscard]] virtual std::string ReadToEnd() { return ""; }

        /** Closes the reader and releases any resources. */
        virtual void Close() {}
    };

} // namespace System::IO
