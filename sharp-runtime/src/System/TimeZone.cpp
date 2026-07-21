// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/TimeZone.hpp"
#include "System/TimeZoneInfo.hpp"

namespace System {

namespace {

/**
 * Concrete adapter that wraps the system TimeZoneInfo as a legacy TimeZone.
 * Used only by CurrentTimeZone().
 */
class SystemTimeZoneAdapter final : public TimeZone {
    std::string standardName_;
    std::string daylightName_;
    TimeSpan    offset_;
public:
    SystemTimeZoneAdapter()
        : standardName_(TimeZoneInfo::Local().getStandardNameProperty()),
          daylightName_(TimeZoneInfo::Local().getDaylightNameProperty()),
          offset_(TimeZoneInfo::Local().getBaseUtcOffsetProperty()) {}

    const std::string& getStandardNameProperty()  const override { return standardName_; }
    const std::string& getDaylightNameProperty()  const override { return daylightName_; }
    TimeSpan GetUtcOffset(const DateTime& /*time*/) const override { return offset_; }
    bool IsDaylightSavingTime(const DateTime& /*time*/) const override { return false; }
};

} // anonymous namespace

const TimeZone& TimeZone::CurrentTimeZone() {
    static SystemTimeZoneAdapter instance;
    return instance;
}

} // namespace System
