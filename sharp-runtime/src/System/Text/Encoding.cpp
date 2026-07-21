// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/Encoding.hpp"
#include "System/Text/UTF8Encoding.hpp"

namespace System::Text
{
    std::shared_ptr<Encoding> Encoding::UTF8()
    {
        static std::shared_ptr<Encoding> instance = std::make_shared<UTF8Encoding>();
        return instance;
    }

    std::vector<SharpRuntime::bytecs> Encoding::GetBytes(const std::string& str) const
    {
        return std::vector<SharpRuntime::bytecs>(str.begin(), str.end());
    }

    std::string Encoding::GetString(
        const SharpRuntime::bytecs* data,
        SharpRuntime::intcs index,
        SharpRuntime::intcs count) const
    {
        if (data == nullptr || count <= 0)
        {
            return {};
        }

        return std::string(
            reinterpret_cast<const char*>(data + index),
            static_cast<size_t>(count));
    }
}
