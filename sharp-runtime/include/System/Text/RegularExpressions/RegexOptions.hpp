// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::RegularExpressions {

    /**
     * @brief Provides enumerated values to use to set regular expression options. [Flags]
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.RegexOptions.
     *
     * @note This runtime's Regex is backed by `std::regex` (ECMAScript grammar), not .NET's own
     * regex engine — only `IgnoreCase` and `Multiline` map to real `std::regex` flags. The rest
     * (`ExplicitCapture`, `Singleline`, `IgnorePatternWhitespace`, `RightToLeft`, `Compiled`,
     * `CultureInvariant`, `NonBacktracking`, `AnyNewLine`) are accepted (so ported code compiles)
     * but have no effect — `std::regex` has no dotall-equivalent flag to implement `Singleline`
     * without rewriting the pattern, and no engine hook for the others.
     */
    enum class RegexOptions {
        None = 0x0000,
        IgnoreCase = 0x0001,
        Multiline = 0x0002,
        ExplicitCapture = 0x0004,
        Compiled = 0x0008,
        Singleline = 0x0010,
        IgnorePatternWhitespace = 0x0020,
        RightToLeft = 0x0040,
        ECMAScript = 0x0100,
        CultureInvariant = 0x0200,
        NonBacktracking = 0x0400,
        AnyNewLine = 0x0800,
    };

    /** @brief Bitwise OR for the [Flags] RegexOptions enum. */
    constexpr RegexOptions operator|(RegexOptions a, RegexOptions b) {
        return static_cast<RegexOptions>(static_cast<int>(a) | static_cast<int>(b));
    }

    /** @brief Bitwise AND for the [Flags] RegexOptions enum. */
    constexpr RegexOptions operator&(RegexOptions a, RegexOptions b) {
        return static_cast<RegexOptions>(static_cast<int>(a) & static_cast<int>(b));
    }

} // namespace System::Text::RegularExpressions
