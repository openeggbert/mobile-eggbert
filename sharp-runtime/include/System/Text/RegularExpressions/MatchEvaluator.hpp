// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <string>

namespace System::Text::RegularExpressions {

    class Match;

    /**
     * @brief Represents a method called each time a regular expression match is found during a
     * Replace() operation, returning the replacement text for that match.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.MatchEvaluator.
     */
    using MatchEvaluator = std::function<std::string(const Match&)>;

} // namespace System::Text::RegularExpressions
