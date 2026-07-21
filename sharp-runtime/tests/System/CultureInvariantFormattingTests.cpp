// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Regression coverage for a systemic bug: std::ostringstream/std::ostream formatting
// consults the *global* C++ locale unless explicitly imbued with std::locale::classic().
// If anything in the embedding process (CNA/mobile-eggbert) ever calls
// std::locale::global() with a non-"C" locale, every unguarded stream-based ToString()
// in this codebase would silently start inserting thousands separators (and, for some
// locales, substituting the decimal point) — breaking .NET's invariant-culture-style
// output and any code that round-trips these strings. Verified empirically before fixing:
// under "en_US.utf8", `std::ostringstream{} << 1234567` produces "1,234,567", and even
// `std::hex` output gets corrupted the same way ("12D687" -> "12D,687").
//
// These tests actually flip the global locale (not just construct a std::locale value)
// so a regression here reproduces the real bug, not just a hypothetical one.
#include <gtest/gtest.h>
#include <locale>
#include "System/BitConverter.hpp"
#include "System/Convert.hpp"
#include "System/DateOnly.hpp"
#include "System/Int32.hpp"
#include "System/TimeSpan.hpp"
#include "System/Version.hpp"

namespace {

    // RAII guard restoring the previous global locale even if the test body throws,
    // so a failure here can't leak a non-"C" global locale into later tests.
    class ScopedGlobalLocale {
        std::locale prev_;
    public:
        explicit ScopedGlobalLocale(const std::locale& loc) : prev_(std::locale::global(loc)) {}
        ~ScopedGlobalLocale() { std::locale::global(prev_); }
    };

    // Returns an installed non-"C"/"POSIX" locale name usable as a stress case, or
    // empty if none of the common candidates are installed on this machine.
    std::string findNonInvariantLocaleName() {
        for (const char* name : {"en_US.utf8", "en_US.UTF-8", "de_DE.utf8", "de_DE.UTF-8"}) {
            try {
                std::locale probe(name);
                return name;
            } catch (const std::runtime_error&) {
                // not installed on this system; try the next candidate
            }
        }
        return "";
    }

} // namespace

TEST(CultureInvariantFormattingTests, NumericAndDateFormatting_UnaffectedByNonInvariantGlobalLocale) {
    std::string localeName = findNonInvariantLocaleName();
    if (localeName.empty()) {
        GTEST_SKIP() << "No non-invariant locale (en_US.utf8/de_DE.utf8) installed on this system";
    }
    ScopedGlobalLocale guard{std::locale(localeName)};

    // Plain std::to_string()-based paths were never at risk (the standard guarantees
    // std::to_string is locale-independent), but included here as a baseline.
    EXPECT_EQ(System::Int32::ToString(1234567), "1234567");

    // Regression: Int32::ToString(value, "X") built its result via std::ostringstream
    // with no imbue() call, so under en_US/de_DE this used to produce "12D,687".
    EXPECT_EQ(System::Int32::ToString(1234567, "X"), "12D687");

    // Regression: TimeSpan::ToString() formats `days` via an unguarded ostringstream;
    // a multi-year TimeSpan easily exceeds 999 days.
    System::TimeSpan ts(1234, 5, 6, 7);
    EXPECT_EQ(ts.ToString().substr(0, 4), "1234");

    // Regression: DateOnly::ToString() formats `year_` via an unguarded ostringstream.
    System::DateOnly d(2026, 7, 9);
    EXPECT_EQ(d.ToString(), "2026-07-09");

    // Regression: Version::ToString() formats Build via an unguarded ostringstream.
    System::Version v(1, 2, 3456, 7890);
    EXPECT_EQ(v.ToString(), "1.2.3456.7890");

    // Regression: Convert::ToString(intcs, base) built hex/octal output via an
    // unguarded ostringstream.
    EXPECT_EQ(System::Convert::ToString(static_cast<System::intcs>(1234567), 16), "12d687");

    // Regression: BitConverter::ToString() builds hex-byte-pair output via an unguarded
    // ostringstream (each individual insertion is only 2 hex digits so grouping could
    // never actually trigger here, but the fix was applied for consistency — this locks
    // that in).
    std::vector<System::bytecs> bytes = {0x12, 0xD6, 0x87};
    EXPECT_EQ(System::BitConverter::ToString(bytes), "12-D6-87");
}
