// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <string>

#include "System/IO/Stream.hpp"
#include "System/IO/TextWriter.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::IO {

    /**
     * @brief Implements a TextWriter for writing characters to a stream in a
     * particular encoding (default: UTF-8).
     *
     * Partial C++ counterpart of .NET System.IO.StreamWriter.
     *
     * @note Status: Implemented
     */
    class StreamWriter : public TextWriter {
    private:
        Stream* stream_;
        bool    leaveOpen_;
        bool    ownsStream_ = false;

        void WriteRaw(const char* data, size_t len);

    public:
        /** Constructs a StreamWriter wrapping the given stream. */
        explicit StreamWriter(Stream* stream, bool leaveOpen = false);
        /** Constructs a StreamWriter that writes to a new or truncated file at path. */
        explicit StreamWriter(const std::string& path);
        /** Destroys the StreamWriter and closes the underlying stream if not leaveOpen. */
        ~StreamWriter() override;

        /** Returns the underlying stream. */
        [[nodiscard]] Stream* getBaseStreamProperty() const { return stream_; }

        using TextWriter::Write;
        using TextWriter::WriteLine;

        /** Writes a string to the stream. */
        void Write(const std::string& value) override;
        /** Writes a null-terminated character array to the stream. */
        void Write(const char* value) override;

        /** Flushes any buffered data to the underlying stream. */
        void Flush() override;
        /** Closes the writer and the underlying stream. */
        void Close() override;
    };

} // namespace System::IO
