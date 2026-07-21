// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <string>

namespace System {

    class IFormatProvider;

    /**
     * @brief Defines a mechanism for parsing a string to a value of the implementing type.
     *
     * C++ counterpart of .NET System.IParsable<TSelf>.
     * Implement Parse() and TryParse() to provide string-parsing capability.
     *
     * .NET declares Parse/TryParse as `static abstract` interface members (C# 11), callable
     * without an instance (`TSelf.Parse(s, provider)`). C++ has no static-virtual dispatch
     * mechanism, so both are mapped here to ordinary instance virtual methods instead; an
     * implementing type still normally also exposes real `static` Parse/TryParse methods
     * (as System::Guid does) for the ergonomic call site — the virtual methods here exist so
     * IParsable<TSelf> can be used polymorphically through a base-class pointer/reference.
     *
     * @tparam TSelf The implementing type (CRTP pattern).
     */
    template<typename TSelf>
    class IParsable {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IParsable() = default;
        /**
         * @brief Parses a string into a value of type @p TSelf.
         * @param s      The string to parse.
         * @param provider Optional format provider (may be nullptr).
         * @return Parsed value.
         * @throws System::FormatException if the string cannot be parsed.
         */
        virtual TSelf Parse(const std::string& s, const IFormatProvider* provider) = 0;
        /**
         * @brief Tries to parse a string into a value of type @p TSelf.
         * @param s       The string to parse.
         * @param provider Optional format provider (may be nullptr).
         * @param result  Receives the parsed value on success.
         * @return true if parsing succeeded; false otherwise.
         */
        virtual bool TryParse(const std::string& s, const IFormatProvider* provider, TSelf& result) = 0;
    };

} // namespace System
