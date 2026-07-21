// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Text/RegularExpressions/Match.hpp"

namespace System::Text::RegularExpressions {

    /**
     * @brief Represents the set of successful matches found by iteratively applying a regular expression.
     *
     * Partial C++ counterpart of .NET System.Text.RegularExpressions.MatchCollection.
     *
     * @note Status: Implemented
     */
    class MatchCollection {
        std::vector<Match> matches_;
    public:
        /** Constructs an empty MatchCollection. */
        MatchCollection() = default;
        /** Constructs a MatchCollection from a vector of Match objects. */
        explicit MatchCollection(std::vector<Match> m) : matches_(std::move(m)) {}

        /** Gets the number of matches in the collection. */
        [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(matches_.size()); }
        /**
         * @brief Returns the match at the given index.
         * @throws System::ArgumentOutOfRangeException if @p i is negative or >= Count.
         */
        [[nodiscard]] const Match& operator[](intcs i) const {
            if (i < 0 || i >= static_cast<intcs>(matches_.size())) {
                throw System::ArgumentOutOfRangeException("i");
            }
            return matches_[static_cast<size_t>(i)];
        }

        /** Returns an iterator to the first match. */
        auto begin() const { return matches_.begin(); }
        /** Returns an iterator past the last match. */
        auto end()   const { return matches_.end(); }
    };

} // namespace System::Text::RegularExpressions
