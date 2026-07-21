// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/HashCode.hpp"
#include "System/Text/Rune.hpp"

namespace System::Text {

    /**
     * @brief Represents a position in Unicode data, allowing for deeper data inspection.
     *
     * C++ counterpart of .NET System.Text.RunePosition. Invalid Unicode symbols are represented
     * by Rune::ReplacementChar (WasReplaced true).
     *
     * @note .NET provides separate `EnumerateUtf16`/`EnumerateUtf8` factories over `char`/`byte`
     * spans. Since this runtime represents strings as UTF-8 `std::string` throughout (no native
     * UTF-16 buffer type), only one enumeration entry point is provided — `Enumerate()` — over a
     * UTF-8 `std::string`, with `StartIndex`/`Length` in UTF-8 byte units.
     */
    class RunePosition {
        Rune rune_;
        SharpRuntime::intcs startIndex_ = 0;
        SharpRuntime::intcs length_ = 0;
        bool wasReplaced_ = false;

    public:
        RunePosition() = default;

        RunePosition(Rune rune, SharpRuntime::intcs startIndex, SharpRuntime::intcs length, bool wasReplaced)
            : rune_(rune), startIndex_(startIndex), length_(length), wasReplaced_(wasReplaced) {
            System::ArgumentOutOfRangeException::ThrowIfNegative(startIndex, "startIndex");
            System::ArgumentOutOfRangeException::ThrowIfNegative(length, "length");
        }

        /** @return The Unicode scalar value of the current symbol (Rune::ReplacementChar if invalid). */
        [[nodiscard]] Rune getRuneProperty() const { return rune_; }
        /** @return The byte offset of the current symbol within the source UTF-8 data. */
        [[nodiscard]] SharpRuntime::intcs getStartIndexProperty() const { return startIndex_; }
        /** @return The byte length of the current symbol within the source UTF-8 data. */
        [[nodiscard]] SharpRuntime::intcs getLengthProperty() const { return length_; }
        /** @return true if the current symbol was invalid and replaced by Rune::ReplacementChar. */
        [[nodiscard]] bool getWasReplacedProperty() const { return wasReplaced_; }

        [[nodiscard]] bool Equals(const RunePosition& other) const {
            return rune_ == other.rune_ && startIndex_ == other.startIndex_ && length_ == other.length_ &&
                   wasReplaced_ == other.wasReplaced_;
        }
        bool operator==(const RunePosition& o) const { return Equals(o); }
        bool operator!=(const RunePosition& o) const { return !Equals(o); }

        /** @brief Returns a hash code combining the rune value, start index, length, and replaced flag. */
        [[nodiscard]] SharpRuntime::intcs GetHashCode() const {
            return System::HashCode::Combine(rune_.getValueProperty(), startIndex_, length_, wasReplaced_);
        }

        class Enumerator;

        /** @brief Returns an enumerator over RunePosition values from the given UTF-8 data. */
        [[nodiscard]] static Enumerator Enumerate(const std::string& data);
    };

    /**
     * @brief Enumerator over RunePosition values from UTF-8-encoded Unicode data.
     *
     * C++ counterpart of .NET RunePosition.Utf8Enumerator (this runtime has no separate
     * Utf16Enumerator — see RunePosition's class doc comment).
     */
    class RunePosition::Enumerator {
        // Owns a copy of the source data rather than holding a pointer/reference to it -- see
        // StringRuneEnumerator.hpp's class doc-comment for why: a `const std::string&`
        // constructor parameter routinely binds to a temporary (e.g.
        // `RunePosition::Enumerate("abc")`), and a bare pointer to that parameter dangles the
        // instant the temporary is destroyed at the end of the full expression.
        std::string data_;
        size_t nextIndex_ = 0;
        Rune currentRune_;
        SharpRuntime::intcs currentStart_ = 0;
        SharpRuntime::intcs currentLength_ = 0;
        bool currentWasReplaced_ = false;
        bool hasCurrent_ = false;
        struct Sentinel {};

    public:
        /** @brief Initializes an enumerator over a copy of @p data. */
        explicit Enumerator(const std::string& data) : data_(data) {}

        /** @return The current RunePosition. */
        [[nodiscard]] RunePosition getCurrentProperty() const {
            return RunePosition(currentRune_, currentStart_, currentLength_, currentWasReplaced_);
        }

        bool MoveNext() {
            if (nextIndex_ >= data_.size()) {
                currentRune_ = Rune();
                currentStart_ = currentLength_ = 0;
                currentWasReplaced_ = false;
                return false;
            }
            SharpRuntime::intcs start = static_cast<SharpRuntime::intcs>(nextIndex_);
            Rune rune;
            size_t consumed = 0;
            bool ok = Rune::TryGetRuneAt(data_, nextIndex_, rune, consumed);
            if (!ok) {
                rune = Rune::ReplacementChar;
                consumed = (consumed == 0) ? 1 : consumed;
            }
            currentRune_ = rune;
            currentStart_ = start;
            currentLength_ = static_cast<SharpRuntime::intcs>(consumed);
            currentWasReplaced_ = !ok;
            nextIndex_ += consumed;
            return true;
        }

        /** @brief Resets the enumerator to its initial position, before the first RunePosition. */
        void Reset() {
            nextIndex_ = 0;
            currentRune_ = Rune();
            currentStart_ = currentLength_ = 0;
            currentWasReplaced_ = false;
            hasCurrent_ = false;
        }

        /** @brief Range-based-for support: advances to the first RunePosition and returns *this. */
        [[nodiscard]] Enumerator& begin() {
            hasCurrent_ = MoveNext();
            return *this;
        }
        /** @brief Range-based-for support: returns the end sentinel. */
        [[nodiscard]] Sentinel end() const { return Sentinel{}; }
        bool operator!=(Sentinel) const { return hasCurrent_; }
        [[nodiscard]] RunePosition operator*() const { return getCurrentProperty(); }
        Enumerator& operator++() {
            hasCurrent_ = MoveNext();
            return *this;
        }
    };

    inline RunePosition::Enumerator RunePosition::Enumerate(const std::string& data) { return Enumerator(data); }

} // namespace System::Text
