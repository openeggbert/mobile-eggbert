// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Text::RegularExpressions {

    /**
     * @brief Specifies the detailed underlying reason a RegexParseException is thrown.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.RegexParseError.
     *
     * @note Since Regex is backed by `std::regex` here, not .NET's own hand-written parser, this
     * runtime's Regex constructor cannot distinguish between most of these reasons — it always
     * uses `Unknown` when wrapping a `std::regex_error` as a RegexParseException. The full value
     * set is still reproduced for API/data compatibility with ported code that pattern-matches
     * on specific `RegexParseError` values.
     */
    enum class RegexParseError {
        Unknown,
        AlternationHasTooManyConditions,
        AlternationHasMalformedCondition,
        InvalidUnicodePropertyEscape,
        MalformedUnicodePropertyEscape,
        UnrecognizedEscape,
        UnrecognizedControlCharacter,
        MissingControlCharacter,
        InsufficientOrInvalidHexDigits,
        QuantifierOrCaptureGroupOutOfRange,
        UndefinedNamedReference,
        UndefinedNumberedReference,
        MalformedNamedReference,
        UnescapedEndingBackslash,
        UnterminatedComment,
        InvalidGroupingConstruct,
        AlternationHasNamedCapture,
        AlternationHasComment,
        AlternationHasMalformedReference,
        AlternationHasUndefinedReference,
        CaptureGroupNameInvalid,
        CaptureGroupOfZero,
        UnterminatedBracket,
        ExclusionGroupNotLast,
        ReversedCharacterRange,
        ShorthandClassInCharacterRange,
        InsufficientClosingParentheses,
        ReversedQuantifierRange,
        NestedQuantifiersNotParenthesized,
        QuantifierAfterNothing,
        InsufficientOpeningParentheses,
        UnrecognizedUnicodeProperty,
    };

} // namespace System::Text::RegularExpressions
