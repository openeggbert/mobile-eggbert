// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include <gtest/gtest.h>
#include "System/TimeZone.hpp"
#include "System/DateTime.hpp"
#include "System/TimeSpan.hpp"

using System::TimeZone;
using System::DateTime;
using System::TimeSpan;

namespace {

class ConcreteTimeZone final : public TimeZone {
    std::string std_ = "TestStandard";
    std::string dst_ = "TestDaylight";
public:
    const std::string& getStandardNameProperty()  const override { return std_; }
    const std::string& getDaylightNameProperty()  const override { return dst_; }
    TimeSpan GetUtcOffset(const DateTime& /*time*/) const override {
        return TimeSpan::FromHours(2.0);
    }
    bool IsDaylightSavingTime(const DateTime& /*time*/) const override { return false; }
};

} // namespace

TEST(TimeZoneTest, StandardName) {
    ConcreteTimeZone tz;
    EXPECT_EQ(tz.getStandardNameProperty(), "TestStandard");
}

TEST(TimeZoneTest, DaylightName) {
    ConcreteTimeZone tz;
    EXPECT_EQ(tz.getDaylightNameProperty(), "TestDaylight");
}

TEST(TimeZoneTest, GetUtcOffset) {
    ConcreteTimeZone tz;
    DateTime now;
    EXPECT_EQ(tz.GetUtcOffset(now).getTotalHoursProperty(), 2.0);
}

TEST(TimeZoneTest, IsDaylightSavingTime) {
    ConcreteTimeZone tz;
    DateTime now;
    EXPECT_FALSE(tz.IsDaylightSavingTime(now));
}

TEST(TimeZoneTest, CurrentTimeZoneNotNull) {
    const TimeZone& tz = TimeZone::CurrentTimeZone();
    (void)tz;
}

TEST(TimeZoneTest, CurrentTimeZoneHasStandardName) {
    const TimeZone& tz = TimeZone::CurrentTimeZone();
    EXPECT_FALSE(tz.getStandardNameProperty().empty());
}

TEST(TimeZoneTest, CurrentTimeZoneSameReference) {
    const TimeZone& a = TimeZone::CurrentTimeZone();
    const TimeZone& b = TimeZone::CurrentTimeZone();
    EXPECT_EQ(&a, &b);
}

TEST(TimeZoneTest, PolymorphicRef) {
    ConcreteTimeZone tz;
    TimeZone& ref = tz;
    EXPECT_EQ(ref.getStandardNameProperty(), "TestStandard");
}
