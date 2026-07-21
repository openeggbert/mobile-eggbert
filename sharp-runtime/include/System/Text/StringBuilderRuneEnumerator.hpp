// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/Text/Rune.hpp"
#include "System/Text/StringBuilder.hpp"

namespace System::Text {

    /**
     * @brief An enumerator for retrieving System::Text::Rune instances from a StringBuilder.
     *
     * C++ counterpart of .NET System.Text.StringBuilderRuneEnumerator. Takes a snapshot of the
     * builder's contents at construction time (via ToString()) — matching .NET's actual behavior
     * of enumerating over the builder's chunk list as it stood when the enumerator was created,
     * not observing later mutations.
     */
    class StringBuilderRuneEnumerator {
        struct Sentinel {};

        std::string snapshot_;
        Rune current_;
        size_t nextIndex_ = 0;
        bool hasCurrent_ = false;

    public:
        /** @brief Initializes an enumerator over a snapshot of @p value's current contents. */
        explicit StringBuilderRuneEnumerator(const StringBuilder& value) : snapshot_(value.ToString()) {}

        /** @return The current Rune. */
        [[nodiscard]] Rune getCurrentProperty() const { return current_; }

        /** @brief Advances to the next Rune. @return false once past the end of the builder's contents. */
        bool MoveNext() {
            if (nextIndex_ >= snapshot_.size()) {
                current_ = Rune();
                return false;
            }
            size_t consumed = 0;
            if (!Rune::TryGetRuneAt(snapshot_, nextIndex_, current_, consumed)) {
                current_ = Rune::ReplacementChar;
                consumed = (consumed == 0) ? 1 : consumed;
            }
            nextIndex_ += consumed;
            return true;
        }

        /** @brief Resets the enumerator to its initial position, before the first Rune. */
        void Reset() {
            current_ = Rune();
            nextIndex_ = 0;
            hasCurrent_ = false;
        }

        /** @brief Range-based-for support: advances to the first Rune and returns *this. */
        [[nodiscard]] StringBuilderRuneEnumerator& begin() {
            hasCurrent_ = MoveNext();
            return *this;
        }
        /** @brief Range-based-for support: returns the end sentinel. */
        [[nodiscard]] Sentinel end() const { return Sentinel{}; }
        bool operator!=(Sentinel) const { return hasCurrent_; }
        [[nodiscard]] Rune operator*() const { return current_; }
        StringBuilderRuneEnumerator& operator++() {
            hasCurrent_ = MoveNext();
            return *this;
        }
    };

    inline StringBuilderRuneEnumerator StringBuilder::EnumerateRunes() const { return StringBuilderRuneEnumerator(*this); }

} // namespace System::Text
