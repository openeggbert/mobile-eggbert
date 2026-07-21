// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/FormatException.hpp"
#include "System/OverflowException.hpp"

namespace System {

    using SharpRuntime::intcs;
    using SharpRuntime::shortcs;

    /**
     * @brief Represents a version number with Major, Minor, Build, and Revision components.
     *
     * C++ counterpart of .NET System.Version.
     */
    class Version {
    public:
        intcs Major    = 0;  ///< Major version component.
        intcs Minor    = 0;  ///< Minor version component.
        intcs Build    = -1; ///< Build number; -1 means not specified.
        intcs Revision = -1; ///< Revision number; -1 means not specified.

        /** @brief Constructs a Version with all components set to their defaults (0.0). */
        Version() = default;

        /**
         * @brief Constructs a Version with the given major and minor components.
         * @throws System::ArgumentOutOfRangeException if @p major or @p minor is negative.
         */
        Version(intcs major, intcs minor) : Major(major), Minor(minor) {
            requireNonNegative(major, "major");
            requireNonNegative(minor, "minor");
        }

        /**
         * @brief Constructs a Version with major, minor, and build components.
         * @throws System::ArgumentOutOfRangeException if @p major, @p minor, or @p build is negative.
         */
        Version(intcs major, intcs minor, intcs build) : Major(major), Minor(minor), Build(build) {
            requireNonNegative(major, "major");
            requireNonNegative(minor, "minor");
            requireNonNegative(build, "build");
        }

        /**
         * @brief Constructs a Version with all four components.
         * @throws System::ArgumentOutOfRangeException if any component is negative (unlike the
         *         2-/3-arg overloads, this one validates @p revision too, since it
         *         is explicitly user-supplied here rather than defaulted to -1).
         */
        Version(intcs major, intcs minor, intcs build, intcs revision)
            : Major(major), Minor(minor), Build(build), Revision(revision) {
            requireNonNegative(major, "major");
            requireNonNegative(minor, "minor");
            requireNonNegative(build, "build");
            requireNonNegative(revision, "revision");
        }

        /** @brief Parses a version from a dot-separated string such as "1.2.3.4". */
        explicit Version(const std::string& versionString) { parse(versionString); }

        /**
         * @brief Parses a version from a dot-separated string.
         *
         * Real .NET's Version(string)/Version.Parse(string) share the exact same
         * validation logic and throw the exact same exception types on failure (see
         * the private parse() helper below) — matched here by simply delegating to
         * the constructor with no separate exception-translation layer.
         * @throws System::ArgumentException if the string has fewer than two or more
         *         than four dot-separated components.
         * @throws System::ArgumentOutOfRangeException if a component is negative.
         * @throws System::FormatException if a component is not an integer.
         * @throws System::OverflowException if a component exceeds Int32 range.
         */
        static Version Parse(const std::string& s) { return Version(s); }

        /** @brief Tries to parse a version string without throwing. Returns true on success. */
        static bool TryParse(const std::string& s, Version& result) {
            try { result = Version(s); return true; } catch (...) { return false; }
        }

        /**
         * @brief Gets the high 16 bits of the Revision component.
         * Matches .NET Version.MajorRevision.
         */
        [[nodiscard]] shortcs getMajorRevisionProperty() const {
            return static_cast<shortcs>(Revision >> 16);
        }

        /**
         * @brief Gets the low 16 bits of the Revision component.
         * Matches .NET Version.MinorRevision.
         */
        [[nodiscard]] shortcs getMinorRevisionProperty() const {
            return static_cast<shortcs>(Revision & 0xFFFF);
        }

        /** @brief Compares this version to another. Returns negative, zero, or positive. */
        [[nodiscard]] intcs CompareTo(const Version& other) const { return cmp(other); }

        /** @brief Returns true if this version has equal components to other. */
        [[nodiscard]] bool Equals(const Version& other) const { return cmp(other) == 0; }

        /** @brief Returns a hash code for this version. */
        [[nodiscard]] intcs GetHashCode() const {
            // Mirror .NET: accumulate 4 components with bit shifts
            intcs hash = 0;
            hash |= (Major & 0x0000000F) << 28;
            hash |= (Minor & 0x000000FF) << 20;
            hash |= (Build & 0x000000FF) << 12;
            hash |= (Revision & 0x00000FFF);
            return hash;
        }

        /**
         * @brief Returns the version as a dot-separated string,
         * omitting unspecified (negative) components.
         */
        [[nodiscard]] std::string ToString() const {
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << Major << '.' << Minor;
            if (Build    >= 0) oss << '.' << Build;
            if (Revision >= 0) oss << '.' << Revision;
            return oss.str();
        }

        /**
         * @brief Returns the version string with exactly fieldCount components.
         * @param fieldCount Number of components to include (1–4).
         * @throws System::ArgumentException if fieldCount is out of range.
         */
        [[nodiscard]] std::string ToString(intcs fieldCount) const {
            if (fieldCount < 0 || fieldCount > 4)
                throw System::ArgumentException("fieldCount must be 0-4", "fieldCount");
            if (fieldCount == 0) return "";
            std::ostringstream oss;
            oss.imbue(std::locale::classic());
            oss << Major;
            if (fieldCount >= 2) oss << '.' << Minor;
            if (fieldCount >= 3) oss << '.' << Build;
            if (fieldCount >= 4) oss << '.' << Revision;
            return oss.str();
        }

        /** @brief Returns true if this version is equal to o. */
        bool operator==(const Version& o) const { return cmp(o) == 0; }
        /** @brief Returns true if this version is not equal to o. */
        bool operator!=(const Version& o) const { return cmp(o) != 0; }
        /** @brief Returns true if this version is less than o. */
        bool operator< (const Version& o) const { return cmp(o) <  0; }
        /** @brief Returns true if this version is less than or equal to o. */
        bool operator<=(const Version& o) const { return cmp(o) <= 0; }
        /** @brief Returns true if this version is greater than o. */
        bool operator> (const Version& o) const { return cmp(o) >  0; }
        /** @brief Returns true if this version is greater than or equal to o. */
        bool operator>=(const Version& o) const { return cmp(o) >= 0; }

    private:
        static void requireNonNegative(intcs value, const char* name) {
            if (value < 0)
                throw System::ArgumentOutOfRangeException(name, std::string(name) + " must be greater than or equal to zero.");
        }

        intcs cmp(const Version& o) const {
            // Uses direct comparison rather than subtraction, matching .NET's actual
            // CompareTo: a subtraction-based comparison would silently overflow
            // (undefined behavior) if components could span the full int32 range.
            if (Major    != o.Major)    return Major    > o.Major    ? 1 : -1;
            if (Minor    != o.Minor)    return Minor    > o.Minor    ? 1 : -1;
            if (Build    != o.Build)    return Build    > o.Build    ? 1 : -1;
            if (Revision != o.Revision) return Revision > o.Revision ? 1 : -1;
            return 0;
        }
        void parse(const std::string& s) {
            static constexpr const char* kComponentNames[4] = {"major", "minor", "build", "revision"};

            // A trailing '.' (e.g. "1.2.") must produce an empty final component and fail --
            // real .NET's ParseVersion slices up to the *last* separator it finds and rejects an
            // empty trailing component with FormatException. std::getline(ss, tok, '.') instead
            // silently drops that phantom trailing empty token (extraction hits EOF with nothing
            // left to read, so the loop condition just becomes false one iteration early) --
            // confirmed via a standalone repro that "1.2." and "1.2.3." both parsed successfully
            // here even though real .NET throws for both. Reject it explicitly before splitting.
            if (!s.empty() && s.back() == '.')
                throw System::FormatException("Input string was not in a correct format.");

            std::vector<std::string> toks;
            std::istringstream ss(s);
            std::string tok;
            while (std::getline(ss, tok, '.')) toks.push_back(tok);
            // .NET requires at least "major.minor" and at most 4 dot-separated components.
            if (toks.size() < 2 || toks.size() > 4)
                throw System::ArgumentException("Version string portion was too short or too long.", "version");

            intcs parts[4] = {0, 0, -1, -1};
            for (std::size_t i = 0; i < toks.size(); ++i) {
                std::size_t consumed = 0;
                int v;
                try {
                    v = std::stoi(toks[i], &consumed);
                } catch (const std::out_of_range&) {
                    throw System::OverflowException("Value was either too large or too small for an Int32.");
                } catch (const std::invalid_argument&) {
                    throw System::FormatException("Input string was not in a correct format.");
                }
                if (consumed != toks[i].size())
                    throw System::FormatException("Input string was not in a correct format.");
                if (v < 0)
                    throw System::ArgumentOutOfRangeException(kComponentNames[i],
                        std::string(kComponentNames[i]) + " must be greater than or equal to zero.");
                parts[i] = static_cast<intcs>(v);
            }
            Major = parts[0]; Minor = parts[1]; Build = parts[2]; Revision = parts[3];
        }
    };

} // namespace System
