// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <optional>
#include <string>

#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Xml {

    /**
     * @brief Table of atomized string objects, used by XmlReader/XmlDocument to intern names.
     *
     * C++ counterpart of .NET System.Xml.XmlNameTable (abstract base). C++ has no reference-
     * identity string interning, so "atomization" here means "returns a canonical copy of an
     * equal string previously added" rather than "returns the same object reference."
     */
    class XmlNameTable {
    public:
        virtual ~XmlNameTable() = default;

        /** @return The atomized string equal to @p array, or std::nullopt if not present. */
        [[nodiscard]] virtual std::optional<std::string> Get(const std::string& array) const = 0;

        /** @return The atomized string equal to the substring [offset, offset+length) of @p array, or std::nullopt. */
        [[nodiscard]] virtual std::optional<std::string> Get(const char* array, SharpRuntime::intcs offset,
                                                              SharpRuntime::intcs length) const = 0;

        /** @return The atomized copy of @p array (adding it to the table if not already present). */
        virtual std::string Add(const std::string& array) = 0;

        /** @return The atomized copy of the substring [offset, offset+length) of @p array (added if not already present). */
        virtual std::string Add(const char* array, SharpRuntime::intcs offset, SharpRuntime::intcs length) = 0;
    };

} // namespace System::Xml
