// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <regex>
#include <string>
#include <utility>
#include <vector>
#include "System/Text/RegularExpressions/Match.hpp"
#include "System/Text/RegularExpressions/MatchCollection.hpp"
#include "System/Text/RegularExpressions/MatchEvaluator.hpp"
#include "System/Text/RegularExpressions/RegexOptions.hpp"
#include "System/Text/RegularExpressions/RegexParseException.hpp"

namespace System::Text::RegularExpressions {

    /**
     * @brief Represents an immutable regular expression.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.Regex, backed by `std::regex`
     * (ECMAScript grammar).
     *
     * @note Status: partial, with named-capture-group support (`(?<name>...)`/`(?'name'...)`
     * syntax is parsed and stripped to a plain `std::regex`-compatible capturing group, with the
     * name-to-index mapping preserved for `Match::Groups()`'s string indexer) — the one gap
     * NEXT.md previously called out ("no named groups") is now fixed. `RegexOptions` beyond
     * `IgnoreCase`/`Multiline` have no effect (see that enum's doc comment); no match timeout
     * support (`std::regex_search` cannot be interrupted mid-match).
     */
    class Regex {
        std::regex re_;
        std::string pattern_;
        std::vector<std::pair<std::string, intcs>> groupNames_;

        static std::regex::flag_type toStdFlags(RegexOptions options) {
            auto flags = std::regex::ECMAScript;
            if ((options & RegexOptions::IgnoreCase) != RegexOptions::None) flags |= std::regex::icase;
            if ((options & RegexOptions::Multiline) != RegexOptions::None) flags |= std::regex::multiline;
            return flags;
        }

        // Strips .NET-style named-group syntax ((?<name>...) / (?'name'...)) down to a plain
        // capturing group `std::regex` understands, recording each name's 1-based group index.
        // Correctly skips escaped characters, character classes, and non-capturing/lookaround
        // constructs ((?:...), (?=...), (?!...), (?<=...), (?<!...)) when counting group indices.
        static std::pair<std::string, std::vector<std::pair<std::string, intcs>>>
        stripNamedGroups(const std::string& pattern) {
            std::string out;
            std::vector<std::pair<std::string, intcs>> names;
            intcs groupIndex = 0;
            bool inClass = false;

            for (size_t i = 0; i < pattern.size(); ++i) {
                char c = pattern[i];
                if (c == '\\' && i + 1 < pattern.size()) {
                    out += c;
                    out += pattern[++i];
                    continue;
                }
                if (c == '[' && !inClass) {
                    inClass = true;
                    out += c;
                    continue;
                }
                if (c == ']' && inClass) {
                    inClass = false;
                    out += c;
                    continue;
                }
                if (c == '(' && !inClass) {
                    if (i + 1 < pattern.size() && pattern[i + 1] == '?') {
                        bool isAngle = i + 2 < pattern.size() && pattern[i + 2] == '<';
                        bool isQuote = i + 2 < pattern.size() && pattern[i + 2] == '\'';
                        bool isLookbehind = isAngle && i + 3 < pattern.size() && (pattern[i + 3] == '=' || pattern[i + 3] == '!');
                        if ((isAngle || isQuote) && !isLookbehind) {
                            char closeChar = isAngle ? '>' : '\'';
                            size_t nameStart = i + 3;
                            size_t nameEnd = pattern.find(closeChar, nameStart);
                            if (nameEnd != std::string::npos) {
                                std::string name = pattern.substr(nameStart, nameEnd - nameStart);
                                names.emplace_back(name, ++groupIndex);
                                out += '(';
                                i = nameEnd;
                                continue;
                            }
                        }
                        out += c; // (?:...), (?=...), (?!...), (?<=...), (?<!...): not capturing
                        continue;
                    }
                    ++groupIndex;
                }
                out += c;
            }
            return {out, names};
        }

        // Returns type is qualified as RegularExpressions::Match (not bare Match) throughout this
        // method — once the public Match(const std::string&) member below is declared, unqualified
        // "Match" inside any inline member function body (a complete-class context, resolved only
        // after the whole class is seen) resolves to that member function, not the sibling Match
        // class, which would break constructor-style calls like "Match(m, ...)".
        //
        // Searches input[offset..] using an iterator range into the ORIGINAL string, not a
        // re-materialized substring (input.substr(offset)) -- two bugs that fix, both
        // confirmed with a compiled reproduction before this change:
        //  1. m.position(i) is relative to whatever sequence was searched. Searching a fresh
        //     substr() made every reported index relative to that substring's start, not the
        //     true start of input; Match's positionOffset parameter corrects this back to an
        //     absolute index (previously uncorrected, corrupting Replace(string,
        //     MatchEvaluator)'s output and every NextMatch() chain's Index after the first).
        //  2. '^' (and any beginning-of-sequence assertion) was evaluated against the fresh
        //     substring's start, so it incorrectly matched at every resumption offset instead
        //     of only the true start of input. match_prev_avail tells the engine the position
        //     right before `first` is a real, readable character (since first genuinely points
        //     into input, not a separate string), so '^' correctly stops matching once
        //     offset > 0.
        RegularExpressions::Match matchFrom(const std::string& input, size_t offset) const {
            if (offset > input.size()) return RegularExpressions::Match();
            std::smatch m;
            auto flags = offset > 0 ? std::regex_constants::match_prev_avail
                                    : std::regex_constants::match_default;
            if (!std::regex_search(input.cbegin() + static_cast<std::ptrdiff_t>(offset), input.cend(), m, re_, flags))
                return RegularExpressions::Match();

            size_t matchStart = offset + static_cast<size_t>(m.position(0));
            size_t nextOffset = matchStart + std::max<size_t>(m[0].length(), 1);
            return RegularExpressions::Match(m, groupNames_, [this, input, nextOffset]() { return matchFrom(input, nextOffset); },
                                             static_cast<intcs>(offset));
        }

    public:
        /** @brief Constructs a Regex with the given pattern and options (ECMAScript syntax). */
        explicit Regex(const std::string& pattern, RegexOptions options = RegexOptions::None) : pattern_(pattern) {
            try {
                auto [stripped, names] = stripNamedGroups(pattern);
                groupNames_ = std::move(names);
                re_ = std::regex(stripped, toStdFlags(options));
            } catch (const std::regex_error&) {
                throw RegexParseException(RegexParseError::Unknown, "Invalid regular expression pattern: " + pattern);
            }
        }

        /** @brief Constructs a Regex with raw std::regex syntax flags (bypasses RegexOptions mapping). */
        Regex(const std::string& pattern, std::regex::flag_type flags) : re_(pattern, flags), pattern_(pattern) {}

        /** @return The pattern this Regex was constructed with. */
        [[nodiscard]] const std::string& getPatternProperty() const { return pattern_; }

        /** @return true if the pattern matches anywhere in the input string. */
        [[nodiscard]] bool IsMatch(const std::string& input) const { return std::regex_search(input, re_); }

        /** @return The first Match in the input, or an unsuccessful Match if none found. */
        [[nodiscard]] class Match Match(const std::string& input) const { return matchFrom(input, 0); }

        /** @brief Deprecated alias for Match() — kept for source compatibility with earlier ported code. */
        [[nodiscard]] RegularExpressions::Match Match_(const std::string& input) const { return matchFrom(input, 0); }

        /** @return All non-overlapping matches in the input. */
        [[nodiscard]] MatchCollection Matches(const std::string& input) const {
            std::vector<RegularExpressions::Match> results;
            auto begin = std::sregex_iterator(input.begin(), input.end(), re_);
            auto end = std::sregex_iterator();
            for (auto it = begin; it != end; ++it) {
                results.emplace_back(*it, groupNames_);
            }
            return MatchCollection(std::move(results));
        }

        /** @brief Replaces all matches in the input with the replacement string ($1-style backreferences supported). */
        [[nodiscard]] std::string Replace(const std::string& input, const std::string& replacement) const {
            return std::regex_replace(input, re_, replacement);
        }

        /** @brief Replaces all matches in the input using @p evaluator to compute each replacement. */
        [[nodiscard]] std::string Replace(const std::string& input, const MatchEvaluator& evaluator) const {
            std::string result;
            size_t lastEnd = 0;
            RegularExpressions::Match m = matchFrom(input, 0);
            while (m.getSuccessProperty()) {
                result += input.substr(lastEnd, static_cast<size_t>(m.getIndexProperty()) - lastEnd);
                result += evaluator(m);
                lastEnd = static_cast<size_t>(m.getIndexProperty()) + static_cast<size_t>(m.getLengthProperty());
                m = m.NextMatch();
            }
            result += input.substr(lastEnd);
            return result;
        }

        /** @brief Splits the input string on occurrences of the pattern. */
        [[nodiscard]] std::vector<std::string> Split(const std::string& input) const {
            std::vector<std::string> result;
            std::sregex_token_iterator it(input.begin(), input.end(), re_, -1), end;
            for (; it != end; ++it) result.push_back(it->str());
            return result;
        }

        /** @return true if the pattern matches anywhere in the input (static overload). */
        [[nodiscard]] static bool IsMatch(const std::string& input, const std::string& pattern) {
            return Regex(pattern).IsMatch(input);
        }

        /** @brief Replaces all pattern matches in the input with the replacement (static overload). */
        [[nodiscard]] static std::string Replace(const std::string& input, const std::string& pattern,
                                                  const std::string& replacement) {
            return Regex(pattern).Replace(input, replacement);
        }

        /** @brief Splits the input string on occurrences of the pattern (static overload). */
        [[nodiscard]] static std::vector<std::string> Split(const std::string& input, const std::string& pattern) {
            return Regex(pattern).Split(input);
        }

        /** @brief Escapes regex metacharacters in @p str so it can be used as a literal within a pattern. */
        [[nodiscard]] static std::string Escape(const std::string& str) {
            static const std::string metaChars = "\\^$.|?*+()[]{}";
            std::string result;
            result.reserve(str.size());
            for (char c : str) {
                if (metaChars.find(c) != std::string::npos) result += '\\';
                result += c;
            }
            return result;
        }
    };

} // namespace System::Text::RegularExpressions
