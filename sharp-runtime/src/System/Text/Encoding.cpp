// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Text/Encoding.hpp"
#include "System/Text/UTF8Encoding.hpp"
#include "System/Text/ASCIIEncoding.hpp"
#include "System/Text/UnicodeEncoding.hpp"
#include "System/Text/UTF32Encoding.hpp"
#include "System/Text/UTF7Encoding.hpp"
#include "System/Text/Latin1Encoding.hpp"

namespace System::Text
{
    std::shared_ptr<Encoding> Encoding::UTF8()
    {
        static std::shared_ptr<Encoding> instance = std::make_shared<UTF8Encoding>();
        return instance;
    }

    std::shared_ptr<Encoding> Encoding::ASCII()
    {
        static std::shared_ptr<Encoding> instance = std::make_shared<ASCIIEncoding>();
        return instance;
    }

    std::shared_ptr<Encoding> Encoding::Unicode()
    {
        static std::shared_ptr<Encoding> instance = std::make_shared<UnicodeEncoding>();
        return instance;
    }

    std::shared_ptr<Encoding> Encoding::BigEndianUnicode()
    {
        static std::shared_ptr<Encoding> instance = std::make_shared<UnicodeEncoding>(true, true);
        return instance;
    }

    std::shared_ptr<Encoding> Encoding::UTF32()
    {
        static std::shared_ptr<Encoding> instance = std::make_shared<UTF32Encoding>();
        return instance;
    }

    std::shared_ptr<Encoding> Encoding::UTF7()
    {
        static std::shared_ptr<Encoding> instance = std::make_shared<UTF7Encoding>();
        return instance;
    }

    std::shared_ptr<Encoding> Encoding::Latin1()
    {
        static std::shared_ptr<Encoding> instance = std::make_shared<Latin1Encoding>();
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