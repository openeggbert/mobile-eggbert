// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include "System/StringComparison.hpp"
#include "System/ArgumentException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

    /**
     * @brief Represents a string comparison operation that uses specific case and culture-based rules.
     *
     * C++ counterpart of .NET System.StringComparer.
     * Abstract base for all string comparers. Use the static factory properties
     * (Ordinal, OrdinalIgnoreCase, InvariantCulture, etc.) to obtain instances,
     * or use FromComparison to convert a StringComparison value.
     */
    class StringComparer {
    public:
        virtual ~StringComparer() = default;

        /**
         * @brief Compares two strings and returns a signed integer indicating their relative order.
         *
         * C++ counterpart of .NET StringComparer.Compare(string, string).
         * @param x The first string.
         * @param y The second string.
         * @return Negative if x &lt; y, zero if equal, positive if x &gt; y.
         */
        [[nodiscard]] virtual SharpRuntime::intcs Compare(const std::string& x, const std::string& y) const = 0;

        /**
         * @brief Determines whether two strings are equal under this comparer's rules.
         *
         * C++ counterpart of .NET StringComparer.Equals(string, string).
         * @param x The first string.
         * @param y The second string.
         * @return true if the strings are considered equal; otherwise false.
         */
        [[nodiscard]] virtual bool Equals(const std::string& x, const std::string& y) const = 0;

        /**
         * @brief Returns a hash code for the specified string, consistent with this comparer's equality.
         *
         * C++ counterpart of .NET StringComparer.GetHashCode(string).
         * @param s The string to hash.
         * @return A hash code for @p s.
         */
        [[nodiscard]] virtual SharpRuntime::intcs GetHashCode(const std::string& s) const = 0;

        /**
         * @brief Gets a StringComparer that performs case-sensitive ordinal (binary) string comparisons.
         *
         * C++ counterpart of .NET StringComparer.Ordinal.
         */
        static std::shared_ptr<StringComparer> Ordinal();

        /**
         * @brief Gets a StringComparer that performs case-insensitive ordinal string comparisons.
         *
         * C++ counterpart of .NET StringComparer.OrdinalIgnoreCase.
         */
        static std::shared_ptr<StringComparer> OrdinalIgnoreCase();

        /**
         * @brief Gets a StringComparer that uses invariant-culture rules (ordinal in this port).
         *
         * C++ counterpart of .NET StringComparer.InvariantCulture.
         */
        static std::shared_ptr<StringComparer> InvariantCulture();

        /**
         * @brief Gets a StringComparer that uses case-insensitive invariant-culture rules.
         *
         * C++ counterpart of .NET StringComparer.InvariantCultureIgnoreCase.
         */
        static std::shared_ptr<StringComparer> InvariantCultureIgnoreCase();

        /**
         * @brief Gets a StringComparer for the current culture (falls back to ordinal in this port).
         *
         * C++ counterpart of .NET StringComparer.CurrentCulture.
         */
        static std::shared_ptr<StringComparer> CurrentCulture();

        /**
         * @brief Gets a case-insensitive StringComparer for the current culture.
         *
         * C++ counterpart of .NET StringComparer.CurrentCultureIgnoreCase.
         */
        static std::shared_ptr<StringComparer> CurrentCultureIgnoreCase();

        /**
         * @brief Returns a StringComparer that corresponds to the specified StringComparison value.
         *
         * C++ counterpart of .NET StringComparer.FromComparison(StringComparison).
         * @param comparisonType One of the StringComparison enumeration values.
         * @return The StringComparer instance for @p comparisonType.
         * @throws System::ArgumentException if @p comparisonType is not a valid value.
         */
        static std::shared_ptr<StringComparer> FromComparison(StringComparison comparisonType);
    };

    /**
     * @brief A StringComparer that performs ordinal (byte-by-byte) string comparisons.
     *
     * C++ counterpart of .NET System.OrdinalComparer.
     * When ignoreCase is false, comparisons are case-sensitive (binary).
     * When ignoreCase is true, comparisons fold case using the C locale rules (ASCII).
     */
    class OrdinalComparer : public StringComparer {
        bool ignoreCase_;

        static std::string toLower(std::string s) {
            std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            return s;
        }

    public:
        /**
         * @brief Constructs an OrdinalComparer.
         *
         * C++ counterpart of .NET OrdinalComparer(bool ignoreCase).
         * @param ignoreCase true for case-insensitive ordinal comparison; false for case-sensitive.
         */
        explicit OrdinalComparer(bool ignoreCase = false) : ignoreCase_(ignoreCase) {}

        /**
         * @brief Gets whether this comparer ignores case during comparisons.
         *
         * C++ counterpart of inspecting the OrdinalComparer's internal ignoreCase field.
         */
        [[nodiscard]] bool getIgnoreCaseProperty() const noexcept { return ignoreCase_; }

        /**
         * @brief Compares two strings using ordinal rules.
         *
         * C++ counterpart of .NET OrdinalComparer.Compare(string, string).
         */
        [[nodiscard]] SharpRuntime::intcs Compare(const std::string& x, const std::string& y) const override {
            if (ignoreCase_) {
                auto lx = toLower(x), ly = toLower(y);
                return lx < ly ? -1 : (lx > ly ? 1 : 0);
            }
            return x < y ? -1 : (x > y ? 1 : 0);
        }

        /**
         * @brief Determines whether two strings are equal using ordinal rules.
         *
         * C++ counterpart of .NET OrdinalComparer.Equals(string, string).
         */
        [[nodiscard]] bool Equals(const std::string& x, const std::string& y) const override {
            return ignoreCase_ ? toLower(x) == toLower(y) : x == y;
        }

        /**
         * @brief Returns a hash code consistent with this comparer's ordinal equality.
         *
         * C++ counterpart of .NET OrdinalComparer.GetHashCode(string).
         */
        [[nodiscard]] SharpRuntime::intcs GetHashCode(const std::string& s) const override {
            // Widen to a fixed 64-bit type before shifting by 32 -- std::size_t is only 32
            // bits on some targets (e.g. Emscripten's wasm32), where "h >> 32" would shift by
            // the full width of the type (UB). Matches the identical fix already established
            // in String::GetHashCode (src/System/String.cpp).
            const auto h = static_cast<std::uint64_t>(std::hash<std::string>{}(ignoreCase_ ? toLower(s) : s));
            return static_cast<SharpRuntime::intcs>(h ^ (h >> 32));
        }
    };

    /**
     * @brief A StringComparer that uses culture-aware comparison rules.
     *
     * C++ counterpart of .NET System.CultureAwareComparer.
     * In this port, full ICU/CultureInfo support is not available; the comparer
     * falls back to ordinal comparison while honouring the ignoreCase flag.
     * Used by StringComparer.InvariantCulture, InvariantCultureIgnoreCase,
     * CurrentCulture, and CurrentCultureIgnoreCase.
     */
    class CultureAwareComparer : public StringComparer {
        bool ignoreCase_;

        static std::string toLower(std::string s) {
            std::transform(s.begin(), s.end(), s.begin(),
                [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            return s;
        }

    public:
        /**
         * @brief Constructs a CultureAwareComparer.
         *
         * C++ counterpart of .NET CultureAwareComparer(CultureInfo, CompareOptions).
         * In this port the culture parameter is omitted; only the ignoreCase flag is used.
         * @param ignoreCase true for case-insensitive comparison; false for case-sensitive.
         */
        explicit CultureAwareComparer(bool ignoreCase = false) : ignoreCase_(ignoreCase) {}

        /**
         * @brief Gets whether this comparer ignores case during comparisons.
         */
        [[nodiscard]] bool getIgnoreCaseProperty() const noexcept { return ignoreCase_; }

        /**
         * @brief Compares two strings using (emulated) culture-aware rules.
         *
         * C++ counterpart of .NET CultureAwareComparer.Compare(string, string).
         */
        [[nodiscard]] SharpRuntime::intcs Compare(const std::string& x, const std::string& y) const override {
            if (ignoreCase_) {
                auto lx = toLower(x), ly = toLower(y);
                return lx < ly ? -1 : (lx > ly ? 1 : 0);
            }
            return x < y ? -1 : (x > y ? 1 : 0);
        }

        /**
         * @brief Determines whether two strings are equal using (emulated) culture-aware rules.
         *
         * C++ counterpart of .NET CultureAwareComparer.Equals(string, string).
         */
        [[nodiscard]] bool Equals(const std::string& x, const std::string& y) const override {
            return ignoreCase_ ? toLower(x) == toLower(y) : x == y;
        }

        /**
         * @brief Returns a hash code consistent with this comparer's equality.
         *
         * C++ counterpart of .NET CultureAwareComparer.GetHashCode(string).
         */
        [[nodiscard]] SharpRuntime::intcs GetHashCode(const std::string& s) const override {
            // See OrdinalComparer::GetHashCode's comment: widen before shifting by 32 to
            // avoid UB on targets where std::size_t is 32 bits (e.g. Emscripten's wasm32).
            const auto h = static_cast<std::uint64_t>(std::hash<std::string>{}(ignoreCase_ ? toLower(s) : s));
            return static_cast<SharpRuntime::intcs>(h ^ (h >> 32));
        }
    };

    // -----------------------------------------------------------------------
    // StringComparer factory implementations
    // -----------------------------------------------------------------------

    inline std::shared_ptr<StringComparer> StringComparer::Ordinal() {
        static auto inst = std::make_shared<OrdinalComparer>(false);
        return inst;
    }
    inline std::shared_ptr<StringComparer> StringComparer::OrdinalIgnoreCase() {
        static auto inst = std::make_shared<OrdinalComparer>(true);
        return inst;
    }
    inline std::shared_ptr<StringComparer> StringComparer::InvariantCulture() {
        static auto inst = std::make_shared<CultureAwareComparer>(false);
        return inst;
    }
    inline std::shared_ptr<StringComparer> StringComparer::InvariantCultureIgnoreCase() {
        static auto inst = std::make_shared<CultureAwareComparer>(true);
        return inst;
    }
    inline std::shared_ptr<StringComparer> StringComparer::CurrentCulture() {
        return std::make_shared<CultureAwareComparer>(false);
    }
    inline std::shared_ptr<StringComparer> StringComparer::CurrentCultureIgnoreCase() {
        return std::make_shared<CultureAwareComparer>(true);
    }
    inline std::shared_ptr<StringComparer> StringComparer::FromComparison(StringComparison comparisonType) {
        switch (comparisonType) {
            case StringComparison::CurrentCulture:             return CurrentCulture();
            case StringComparison::CurrentCultureIgnoreCase:   return CurrentCultureIgnoreCase();
            case StringComparison::InvariantCulture:           return InvariantCulture();
            case StringComparison::InvariantCultureIgnoreCase: return InvariantCultureIgnoreCase();
            case StringComparison::Ordinal:                    return Ordinal();
            case StringComparison::OrdinalIgnoreCase:          return OrdinalIgnoreCase();
            default:
                throw ArgumentException("Not a valid StringComparison value.");
        }
    }

} // namespace System
