// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <unordered_set>

#include "System/Xml/XmlNameTable.hpp"

namespace System::Xml {

    /**
     * @brief Concrete XmlNameTable backed by a hash set of atomized strings.
     *
     * C++ counterpart of .NET System.Xml.NameTable.
     */
    class NameTable : public XmlNameTable {
        std::unordered_set<std::string> entries_;

    public:
        NameTable() = default;

        [[nodiscard]] std::optional<std::string> Get(const std::string& array) const override;
        [[nodiscard]] std::optional<std::string> Get(const char* array, SharpRuntime::intcs offset,
                                                      SharpRuntime::intcs length) const override;
        std::string Add(const std::string& array) override;
        std::string Add(const char* array, SharpRuntime::intcs offset, SharpRuntime::intcs length) override;
    };

} // namespace System::Xml
