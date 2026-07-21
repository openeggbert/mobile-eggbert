// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/IO/StreamReader.hpp"

namespace System::IO
{
    StreamReader::StreamReader(Stream* stream, bool leaveOpen)
        : stream_(stream), leaveOpen_(leaveOpen), ownsStream_(false)
    {
    }

    StreamReader::~StreamReader()
    {
        if (!leaveOpen_ && stream_) stream_->Close();
        if (ownsStream_) delete stream_;
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
}
