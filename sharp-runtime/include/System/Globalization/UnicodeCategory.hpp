// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Globalization {

/**
 * @brief Defines the Unicode category of a character (the "General_Category" Unicode property).
 *
 * C++ counterpart of .NET System.Globalization.UnicodeCategory.
 */
enum class UnicodeCategory {
    UppercaseLetter         = 0,  ///< Uppercase letter (Lu).
    LowercaseLetter         = 1,  ///< Lowercase letter (Ll).
    TitlecaseLetter         = 2,  ///< Titlecase letter (Lt).
    ModifierLetter          = 3,  ///< Modifier letter (Lm).
    OtherLetter             = 4,  ///< Letter not falling into another Letter category (Lo).
    NonSpacingMark          = 5,  ///< Non-spacing combining mark (Mn).
    SpacingCombiningMark    = 6,  ///< Spacing combining mark (Mc).
    EnclosingMark           = 7,  ///< Enclosing combining mark (Me).
    DecimalDigitNumber      = 8,  ///< Decimal digit (Nd).
    LetterNumber            = 9,  ///< Letter-like numeric character (Nl).
    OtherNumber             = 10, ///< Number not falling into another Number category (No).
    SpaceSeparator          = 11, ///< Space character (Zs).
    LineSeparator           = 12, ///< Line separator character (Zl).
    ParagraphSeparator      = 13, ///< Paragraph separator character (Zp).
    Control                 = 14, ///< Control code point (Cc).
    Format                  = 15, ///< Format character (Cf).
    Surrogate               = 16, ///< Surrogate code point (Cs).
    PrivateUse              = 17, ///< Private-use character (Co).
    ConnectorPunctuation    = 18, ///< Connector punctuation (Pc).
    DashPunctuation         = 19, ///< Dash or hyphen punctuation (Pd).
    OpenPunctuation         = 20, ///< Opening punctuation such as an opening parenthesis (Ps).
    ClosePunctuation        = 21, ///< Closing punctuation such as a closing parenthesis (Pe).
    InitialQuotePunctuation = 22, ///< Opening/initial quotation mark punctuation (Pi).
    FinalQuotePunctuation   = 23, ///< Closing/final quotation mark punctuation (Pf).
    OtherPunctuation        = 24, ///< Punctuation not falling into another Punctuation category (Po).
    MathSymbol              = 25, ///< Mathematical symbol (Sm).
    CurrencySymbol          = 26, ///< Currency symbol (Sc).
    ModifierSymbol          = 27, ///< Modifier symbol (Sk).
    OtherSymbol             = 28, ///< Symbol not falling into another Symbol category (So).
    OtherNotAssigned        = 29, ///< A code point to which no Unicode category has been assigned (Cn).
};

} // namespace System::Globalization
