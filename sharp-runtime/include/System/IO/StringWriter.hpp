// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <sstream>
#include <string>
#include "System/IO/TextWriter.hpp"

namespace System::IO {

    /**
     * @brief Implements a TextWriter that writes to a string (via std::ostringstream).
     *
     * Partial C++ counterpart of .NET System.IO.StringWriter.
     *
     * @note Status: Implemented
     */
    class StringWriter : public TextWriter {
        std::ostringstream buf_;
    public:
        /** Constructs a StringWriter with an empty buffer. */
        StringWriter() = default;

        using TextWriter::Write;
        using TextWriter::WriteLine;

        /** Writes a string to the internal buffer. */
        void Write(const std::string& value) override { buf_ << value; }

        /** @brief Returns the string written so far. */
        [[nodiscard]] std::string ToString() const { return buf_.str(); }

        /** @brief Returns the underlying string builder contents (alias for ToString). */
        [[nodiscard]] std::string GetStringBuilder() const { return buf_.str(); }
    };

} // namespace System::IO
