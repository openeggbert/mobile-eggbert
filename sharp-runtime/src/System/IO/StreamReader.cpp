// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/StreamReader.hpp"
#include "System/IO/FileStream.hpp"
#include "System/IO/FileMode.hpp"
#include "System/IO/FileAccess.hpp"

namespace System::IO
{
    StreamReader::StreamReader(Stream* stream, bool leaveOpen)
        : stream_(stream), leaveOpen_(leaveOpen), ownsStream_(false)
    {
    }

    StreamReader::StreamReader(const std::string& path)
        : stream_(new FileStream(path, FileMode::Open, FileAccess::Read)),
          leaveOpen_(false), ownsStream_(true)
    {
    }

    StreamReader::~StreamReader()
    {
        if (!leaveOpen_ && stream_) stream_->Close();
        if (ownsStream_) delete stream_;
    }

    intcs StreamReader::Peek()
    {
        if (hasPeeked_) return static_cast<intcs>(peeked_);
        if (stream_ == nullptr) return -1;

        bytecs b;
        const intcs n = stream_->Read(&b, 0, 1);
        if (n == 0) return -1;

        peeked_ = b;
        hasPeeked_ = true;
        return static_cast<intcs>(b);
    }

    intcs StreamReader::Read()
    {
        if (hasPeeked_)
        {
            hasPeeked_ = false;
            return static_cast<intcs>(peeked_);
        }
        if (stream_ == nullptr) return -1;

        bytecs b;
        const intcs n = stream_->Read(&b, 0, 1);
        return n == 0 ? -1 : static_cast<intcs>(b);
    }

    // Verified against StreamReader.cs's ReadLine(): real .NET treats '\r' and '\n' as
    // interchangeable line terminators -- a lone '\r' (classic Mac line ending) ends the line
    // on its own, not just as part of "\r\n". If '\r' is immediately followed by '\n', that
    // '\n' is consumed as part of the same terminator (CRLF); a '\r' not followed by '\n'
    // still terminates the line by itself. This previously only stopped scanning at '\n', so a
    // lone '\r' was treated as ordinary line content -- silently merging what should be two
    // separate lines into one, with the '\r' left embedded in the middle of the result.
    std::string StreamReader::ReadLine()
    {
        intcs c = Read();
        if (c == -1) return "";

        std::string line;
        while (c != -1 && c != '\n' && c != '\r')
        {
            line.push_back(static_cast<char>(c));
            c = Read();
        }
        if (c == '\r' && Peek() == '\n') Read();
        return line;
    }

    std::string StreamReader::ReadToEnd()
    {
        std::string result;
        if (hasPeeked_)
        {
            result.push_back(static_cast<char>(peeked_));
            hasPeeked_ = false;
        }

        intcs c;
        while ((c = Read()) != -1) result.push_back(static_cast<char>(c));
        return result;
    }

    void StreamReader::Close()
    {
        // Verified against StreamReader.cs: Close() delegates to Dispose(true), which checks
        // _closable (i.e. !leaveOpen) before closing the underlying stream -- matching this
        // type's own destructor (above), which already got this right. This method previously
        // closed the stream unconditionally, defeating leaveOpen's entire purpose for any
        // caller that called Close() explicitly instead of only relying on the destructor.
        if (!leaveOpen_ && stream_) stream_->Close();
    }
}
