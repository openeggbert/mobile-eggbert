// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/Text/Rune.hpp"

namespace System::Text {

    /**
     * @brief An enumerator for retrieving System::Text::Rune instances from a std::string.
     *
     * C++ counterpart of .NET System.Text.StringRuneEnumerator. Iterates UTF-8 byte sequences
     * (this runtime's string representation) instead of .NET's UTF-16 code units; invalid byte
     * sequences are replaced with Rune::ReplacementChar, matching .NET's behavior for invalid
     * surrogate sequences.
     *
     * Primary API matches .NET (getCurrentProperty()/MoveNext()/Reset()); a range-based-for
     * adaptor (begin()/end(), via an end sentinel) is provided as a C++ convenience.
     */
    class StringRuneEnumerator {
        struct Sentinel {};

        // Owns a copy of the source string rather than holding a pointer/reference to it.
        // .NET's StringRuneEnumerator can safely hold a `string` field because C# strings
        // are immutable and garbage-collected -- the string outlives the enumerator by
        // construction. C++ has neither guarantee: a `const std::string&` constructor
        // parameter routinely binds to a temporary (e.g. the extremely common
        // `StringRuneEnumerator e("abc");` -- a string literal implicitly converts to a
        // temporary std::string), and that temporary is destroyed at the end of the full
        // expression, before the enumerator is ever used. Storing just a pointer to it was a
        // dangling-pointer bug on the single most natural way to construct this type.
        std::string string_;
        Rune current_;
        size_t nextIndex_ = 0;
        bool hasCurrent_ = false;

    public:
        /** @brief Initializes an enumerator over a copy of @p value. */
        explicit StringRuneEnumerator(const std::string& value) : string_(value) {}

        /** @return The current Rune. */
        [[nodiscard]] Rune getCurrentProperty() const { return current_; }

        /** @brief Advances to the next Rune. @return false once past the end of the string. */
        bool MoveNext() {
            if (nextIndex_ >= string_.size()) {
                current_ = Rune();
                return false;
            }
            size_t consumed = 0;
            if (!Rune::TryGetRuneAt(string_, nextIndex_, current_, consumed)) {
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

        // --- C++ range-based-for adaptor ---------------------------------------------------

        /** @brief Range-based-for support: advances to the first Rune and returns *this. */
        [[nodiscard]] StringRuneEnumerator& begin() {
            hasCurrent_ = MoveNext();
            return *this;
        }
        /** @brief Range-based-for support: returns the end sentinel. */
        [[nodiscard]] Sentinel end() const { return Sentinel{}; }
        bool operator!=(Sentinel) const { return hasCurrent_; }
        [[nodiscard]] Rune operator*() const { return current_; }
        StringRuneEnumerator& operator++() {
            hasCurrent_ = MoveNext();
            return *this;
        }
    };

} // namespace System::Text
