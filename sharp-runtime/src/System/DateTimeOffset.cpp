// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Created by robertvokac on 6/7/25.
//

#include "System/DateTimeOffset.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace {

    using namespace System;

    // C++ counterpart of .NET DateTimeOffset's private ValidateOffset/ValidateDate helpers.
    void validateOffsetAndRange(const DateTime& clockDateTime, const TimeSpan& offset) {
        constexpr longcs maxOffsetMinutes = 14 * 60;

        if (offset.getTicksProperty() % TimeSpan::TicksPerMinute != 0) {
            throw ArgumentException("Offset must be specified in whole minutes.", "offset");
        }

        const longcs offsetMinutes = offset.getTicksProperty() / TimeSpan::TicksPerMinute;
        if (offsetMinutes < -maxOffsetMinutes || offsetMinutes > maxOffsetMinutes) {
            throw ArgumentOutOfRangeException("offset", "Offset must be within plus or minus 14 hours.");
        }

        const longcs utcTicks = clockDateTime.getTicksProperty() - offset.getTicksProperty();
        if (utcTicks < 0 || utcTicks > DateTime::MaxTicks) {
            throw ArgumentOutOfRangeException("offset",
                "The UTC time represented when the offset is applied must be between year 0 and 10,000.");
        }
    }

} // namespace

namespace System {

    DateTimeOffset::DateTimeOffset()
        : dateTime_(DateTime()), offset_(TimeSpan::Zero) {}

    DateTimeOffset::DateTimeOffset(const DateTime& dateTime, const TimeSpan& offset)
        : dateTime_(dateTime), offset_(offset) {
        validateOffsetAndRange(dateTime_, offset_);
    }

    DateTimeOffset DateTimeOffset::getUtcNowProperty() {
        return DateTimeOffset(DateTime::getNowProperty(), TimeSpan::Zero);
    }

    longcs DateTimeOffset::getUtcTicksProperty() const {
        return dateTime_.getTicksProperty() - offset_.getTicksProperty();
    }

    bool DateTimeOffset::Equals(const DateTimeOffset& other) const {
        return getUtcTicksProperty() == other.getUtcTicksProperty();
    }

    bool DateTimeOffset::operator==(const DateTimeOffset& other) const { return Equals(other); }

    GetTypeNameCPP(DateTimeOffset, "System.DateTimeOffset")
}
