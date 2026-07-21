// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/StringBuilder.hpp"

namespace System::Text
{
    StringBuilder::StringBuilder()
        : buffer()
    {
    }

    StringBuilder::StringBuilder(const std::string& value)
        : buffer(value)
    {
    }

    void StringBuilder::Clear()
    {
        buffer.clear();
    }

    StringBuilder& StringBuilder::Append(const std::string& value)
    {
        buffer += value;
        return *this;
    }

    StringBuilder& StringBuilder::Append(const char* value)
    {
        if (value != nullptr)
        {
            buffer += value;
        }
        return *this;
    }

    StringBuilder& StringBuilder::Append(intcs value)
    {
        buffer += std::to_string(value);
        return *this;
    }

    std::string StringBuilder::ToString() const
    {
        return buffer;
    }
}
