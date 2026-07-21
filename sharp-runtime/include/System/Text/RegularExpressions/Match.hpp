// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <cctype>
#include <functional>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/RegularExpressions/GroupCollection.hpp"

namespace System::Text::RegularExpressions {

    using SharpRuntime::intcs;

    /**
     * @brief Represents the results from a single regular expression match.
     *
     * C++ counterpart of .NET System.Text.RegularExpressions.Match.
     */
    class Match {
        // Per-submatch data extracted eagerly at construction time (value/index/length/matched),
        // rather than keeping the std::smatch itself: a std::smatch's sub_matches hold iterators
        // into whatever sequence was searched, and callers (e.g. Regex::matchFrom) frequently
        // search a temporary/local string that no longer exists once the Match is returned to the
        // caller — retaining the smatch would leave those iterators dangling.
        struct SubMatchData {
            std::string value;
            intcs index = 0;
            intcs length = 0;
            bool matched = false;
        };

        bool success_ = false;
        std::vector<SubMatchData> subMatches_;
        std::vector<std::pair<std::string, intcs>> groupNames_;
        std::function<Match()> nextMatchFunc_;

    public:
        /** @brief Constructs an empty (unsuccessful) Match. */
        Match() = default;

        /**
         * @brief Constructs a Match from a std::smatch result, optionally with named-group
         * info and a NextMatch() continuation.
         *
         * @param m            The std::smatch produced by regex_search.
         * @param groupNames   Name-to-index mapping for named capture groups.
         * @param nextMatchFunc Continuation invoked by NextMatch().
         * @param positionOffset Added to every submatch's reported index. `m.position(i)` is
         *        relative to the sequence that was actually searched -- when that sequence is
         *        an iterator range starting partway into the real input (as
         *        Regex::matchFrom does, to search "the rest of the string" for
         *        Match()/NextMatch() chains and Replace(string, MatchEvaluator)), `m.position(i)`
         *        is relative to that range's start, not the true start of the original input.
         *        Without this correction, every match after the first reported the wrong
         *        Index, corrupting Replace(string, MatchEvaluator)'s output. Callers that
         *        search the whole original string directly (e.g. Regex::Matches() via
         *        sregex_iterator) already get absolute positions and pass 0 (the default).
         */
        explicit Match(const std::smatch& m, std::vector<std::pair<std::string, intcs>> groupNames = {},
                        std::function<Match()> nextMatchFunc = nullptr, intcs positionOffset = 0)
            : success_(m.ready() && !m.empty()), groupNames_(std::move(groupNames)),
              nextMatchFunc_(std::move(nextMatchFunc)) {
            if (success_) {
                subMatches_.reserve(m.size());
                for (size_t i = 0; i < m.size(); ++i) {
                    bool matched = m[i].matched;
                    subMatches_.push_back(SubMatchData{
                        matched ? m[i].str() : std::string(),
                        matched ? static_cast<intcs>(m.position(i)) + positionOffset : 0,
                        matched ? static_cast<intcs>(m[i].length()) : 0, matched});
                }
            }
        }

        /** @return true if the match was successful. */
        [[nodiscard]] bool getSuccessProperty() const { return success_; }
        /** @return The matched substring, or empty string if unsuccessful. */
        [[nodiscard]] std::string getValueProperty() const { return success_ ? subMatches_[0].value : ""; }
        /** @return The zero-based index of the match in the input string, or -1 if unsuccessful. */
        [[nodiscard]] intcs getIndexProperty() const { return success_ ? subMatches_[0].index : -1; }
        /** @return The length of the matched substring, or 0 if unsuccessful. */
        [[nodiscard]] intcs getLengthProperty() const { return success_ ? subMatches_[0].length : 0; }

        /** @return The value of the capture group at the given index, or empty string if not matched/out of range. */
        [[nodiscard]] std::string Group(intcs index) const {
            if (!success_ || index < 0 || index >= static_cast<intcs>(subMatches_.size())) return "";
            return subMatches_[static_cast<size_t>(index)].value;
        }

        /**
         * @brief All capture groups from this match, including group 0 (the whole match) and
         * any named groups.
         *
         * Each Group's Name is the parsed `(?<name>...)`/`(?'name'...)` name when the group
         * was named, or the numeric index as a string otherwise -- matching real .NET, where
         * e.g. `Regex.Match("a1", "(\d)").Groups[1].Name` is "1". A prior version of this
         * method always used the numeric index, ignoring groupNames_ entirely, so a named
         * group's own Group.Name never reflected the name that was parsed (even though the
         * *value* was already reachable correctly via GroupCollection's string indexer, which
         * consults groupNames_ directly).
         */
        [[nodiscard]] GroupCollection Groups() const {
            std::vector<System::Text::RegularExpressions::Group> groups;
            std::unordered_map<std::string, intcs> nameToIndex;
            for (const auto& [name, idx] : groupNames_) {
                nameToIndex[name] = idx;
            }
            std::unordered_map<intcs, std::string> indexToName;
            for (const auto& [name, idx] : nameToIndex) {
                indexToName[idx] = name;
            }
            for (size_t i = 0; i < subMatches_.size(); ++i) {
                const auto& sm = subMatches_[i];
                auto it = indexToName.find(static_cast<intcs>(i));
                std::string name = (it != indexToName.end()) ? it->second : std::to_string(i);
                groups.emplace_back(sm.matched ? sm.value : std::string(), sm.matched ? sm.index : 0,
                                     sm.matched ? sm.length : 0, sm.matched, std::move(name));
            }
            return GroupCollection(std::move(groups), std::move(nameToIndex));
        }

        /** @return A collection containing just this match's overall capture (group 0). */
        [[nodiscard]] CaptureCollection Captures() const {
            if (!success_) return CaptureCollection();
            return CaptureCollection({System::Text::RegularExpressions::Capture(
                subMatches_[0].value, subMatches_[0].index, subMatches_[0].length)});
        }

        /** @brief Searches for the next match, starting immediately after this one. Returns an unsuccessful Match if there is no next match or this wasn't constructed with search context. */
        [[nodiscard]] Match NextMatch() const { return nextMatchFunc_ ? nextMatchFunc_() : Match(); }

        /**
         * @brief Expands a replacement pattern (e.g. "$1-$2", "${name}") using this match's groups.
         * @note Supports `$0`-`$9` (single digit) and `${name}` only — .NET's full `$$`/`` $` ``/`$'`/
         * `$_`/`$+` substitution tokens and multi-digit `$12` group references are not reproduced.
         */
        [[nodiscard]] std::string Result(const std::string& replacementPattern) const {
            std::string result;
            GroupCollection groups = Groups();
            for (size_t i = 0; i < replacementPattern.size(); ++i) {
                char c = replacementPattern[i];
                if (c == '$' && i + 1 < replacementPattern.size()) {
                    char next = replacementPattern[i + 1];
                    if (std::isdigit(static_cast<unsigned char>(next))) {
                        intcs groupIndex = next - '0';
                        if (groupIndex < groups.getCountProperty()) {
                            result += groups[groupIndex].getValueProperty();
                        }
                        ++i;
                        continue;
                    }
                    if (next == '{') {
                        size_t close = replacementPattern.find('}', i + 2);
                        if (close != std::string::npos) {
                            std::string name = replacementPattern.substr(i + 2, close - i - 2);
                            result += groups[name].getValueProperty();
                            i = close;
                            continue;
                        }
                    }
                }
                result += c;
            }
            return result;
        }

        /** @return A singleton empty (unsuccessful) Match. */
        static const Match& Empty() {
            static Match m;
            return m;
        }
    };

} // namespace System::Text::RegularExpressions
