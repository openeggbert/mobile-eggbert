// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/DateTime.hpp"
#include "System/DateTimeOffset.hpp"
#include "System/DayOfWeek.hpp"
#include "System/TimeSpan.hpp"

namespace System {

    /**
     * @brief Represents a time zone — a named region with a UTC offset.
     *
     * C++ counterpart of .NET System.TimeZoneInfo.
     *
     * Implements the subset of the .NET API needed for game-engine porting.
     * @c Local() reads the real system timezone via POSIX @c localtime_r().
     * @c FindSystemTimeZoneById() resolves IANA names from @c /usr/share/zoneinfo/.
     *
     * **Limitations (documented, not bugs):**
     * - DST transitions are not modelled; IsDaylightSavingTime always returns false.
     * - GetAdjustmentRules() returns an empty array.
     * - Serialisation (ToSerializedString / FromSerializedString) is not implemented.
     * - POSIX-only: Local() and FindSystemTimeZoneById() use localtime_r and /usr/share/zoneinfo.
     */
    class TimeZoneInfo {
    public:
        // =====================================================================
        // Nested types
        // =====================================================================

        /**
         * @brief Represents the date and time when a time zone changes from standard
         * time to daylight saving time, or vice versa.
         *
         * C++ counterpart of .NET System.TimeZoneInfo.TransitionTime.
         * In this implementation all instances are stubs; DST transitions are not modelled.
         */
        struct TransitionTime {
            DateTime   timeOfDay_;
            intcs      month_          = 0;
            intcs      week_           = 0;
            intcs      day_            = 0;
            DayOfWeek  dayOfWeek_      = DayOfWeek::Sunday;
            bool       isFixedDateRule_ = false;

            /** @brief Gets the time of day at which the transition occurs. */
            [[nodiscard]] DateTime   getTimeOfDayProperty()     const { return timeOfDay_; }
            /** @brief Gets the month in which the transition occurs (1-12). */
            [[nodiscard]] intcs      getMonthProperty()         const { return month_; }
            /** @brief Gets the week of the month (1-5) in which the transition occurs. */
            [[nodiscard]] intcs      getWeekProperty()          const { return week_; }
            /** @brief Gets the day on which the transition occurs for a fixed-date rule. */
            [[nodiscard]] intcs      getDayProperty()           const { return day_; }
            /** @brief Gets the day of the week on which the transition occurs for a floating rule. */
            [[nodiscard]] DayOfWeek  getDayOfWeekProperty()     const { return dayOfWeek_; }
            /** @brief Gets a value indicating whether the transition is fixed-date or floating. */
            [[nodiscard]] bool       getIsFixedDateRuleProperty() const { return isFixedDateRule_; }

            /**
             * @brief Validates that @p timeOfDay represents only a time-of-day (no date
             * component beyond DateTime.MinValue's implicit date) at millisecond granularity.
             *
             * C++ counterpart of the timeOfDay portion of .NET
             * TimeZoneInfo.TransitionTime.ValidateTransitionTime -- real .NET also rejects a
             * timeOfDay.Kind other than Unspecified, which does not apply here since this
             * port's DateTime does not track DateTimeKind (see DateTime.hpp's documented
             * Kind limitation).
             * @throws ArgumentException if @p timeOfDay has a date component or sub-millisecond ticks.
             */
            static void validateTimeOfDay(const DateTime& timeOfDay) {
                longcs ticks = timeOfDay.getTicksProperty();
                if (ticks >= TimeSpan::TicksPerDay || ticks % TimeSpan::TicksPerMillisecond != 0)
                    throw System::ArgumentException(
                        "The supplied DateTime must have the Year, Month, and Day properties set to 1, "
                        "and the Millisecond, and Ticks properties set to 0.", "timeOfDay");
            }

            /**
             * @brief Creates a fixed-date transition rule.
             *
             * C++ counterpart of .NET TransitionTime.CreateFixedDateRule(DateTime, int, int).
             * @param timeOfDay Time of day when the transition occurs (must have no date component).
             * @param month     Month of the transition (1-12).
             * @param day       Day of the month (1-31).
             * @throws ArgumentOutOfRangeException if month or day is out of range.
             * @throws ArgumentException if timeOfDay has a date component or sub-millisecond ticks.
             */
            static TransitionTime CreateFixedDateRule(DateTime timeOfDay, intcs month, intcs day) {
                validateTimeOfDay(timeOfDay);
                if (month < 1 || month > 12)
                    throw ArgumentOutOfRangeException("month: Month must be between 1 and 12.");
                if (day < 1 || day > 31)
                    throw ArgumentOutOfRangeException("day: Day must be between 1 and 31.");
                TransitionTime t;
                t.timeOfDay_       = timeOfDay;
                t.month_           = month;
                t.day_             = day;
                t.week_            = 1;
                t.isFixedDateRule_ = true;
                return t;
            }

            /**
             * @brief Creates a floating-date transition rule.
             *
             * C++ counterpart of .NET TransitionTime.CreateFloatingDateRule(DateTime, int, int, DayOfWeek).
             * @param timeOfDay  Time of day when the transition occurs.
             * @param month      Month of the transition (1-12).
             * @param week       Week of the month (1-5).
             * @param dayOfWeek  Day of the week (Sunday=0 … Saturday=6).
             * @throws ArgumentOutOfRangeException if month, week, or dayOfWeek is out of range.
             * @throws ArgumentException if timeOfDay has a date component or sub-millisecond ticks.
             */
            static TransitionTime CreateFloatingDateRule(DateTime timeOfDay, intcs month,
                                                         intcs week, DayOfWeek dayOfWeek) {
                validateTimeOfDay(timeOfDay);
                if (month < 1 || month > 12)
                    throw ArgumentOutOfRangeException("month: Month must be between 1 and 12.");
                if (week < 1 || week > 5)
                    throw ArgumentOutOfRangeException("week: Week must be between 1 and 5.");
                if (static_cast<int>(dayOfWeek) < 0 || static_cast<int>(dayOfWeek) > 6)
                    throw ArgumentOutOfRangeException("dayOfWeek: DayOfWeek must be between 0 and 6.");
                TransitionTime t;
                t.timeOfDay_       = timeOfDay;
                t.month_           = month;
                t.week_            = week;
                t.day_             = 1;
                t.dayOfWeek_       = dayOfWeek;
                t.isFixedDateRule_ = false;
                return t;
            }

            /**
             * @brief Returns true if this TransitionTime is equal to @p other.
             *
             * C++ counterpart of .NET TransitionTime.Equals(TransitionTime).
             * For fixed-date rules only Day is compared; for floating rules Week and DayOfWeek
             * are compared instead, matching .NET's exact semantics.
             */
            [[nodiscard]] bool Equals(const TransitionTime& other) const {
                if (isFixedDateRule_ != other.isFixedDateRule_) return false;
                if (!(timeOfDay_ == other.timeOfDay_))          return false;
                if (month_ != other.month_)                     return false;
                return isFixedDateRule_
                    ? (day_ == other.day_)
                    : (week_ == other.week_ && dayOfWeek_ == other.dayOfWeek_);
            }

            /**
             * @brief Returns a hash code for this TransitionTime.
             *
             * C++ counterpart of .NET TransitionTime.GetHashCode().
             */
            [[nodiscard]] intcs GetHashCode() const noexcept {
                return month_ ^ (week_ << 8);
            }

            /**
             * @brief Returns true if both TransitionTime instances are equal.
             *
             * C++ counterpart of .NET TransitionTime operator==.
             */
            bool operator==(const TransitionTime& o) const { return Equals(o); }

            /**
             * @brief Returns true if both TransitionTime instances differ.
             *
             * C++ counterpart of .NET TransitionTime operator!=.
             */
            bool operator!=(const TransitionTime& o) const { return !Equals(o); }
        };

        /**
         * @brief Provides information about a time zone adjustment (DST rule).
         *
         * C++ counterpart of .NET System.TimeZoneInfo.AdjustmentRule.
         * In this implementation GetAdjustmentRules() always returns an empty vector;
         * these objects are exposed only for API completeness.
         */
        class AdjustmentRule {
            DateTime       dateStart_;
            DateTime       dateEnd_;
            TimeSpan       daylightDelta_;
            TransitionTime daylightTransitionStart_;
            TransitionTime daylightTransitionEnd_;
            TimeSpan       baseUtcOffsetDelta_;
            bool           noDaylightTransitions_ = false;

            AdjustmentRule() = default;
        public:
            /** @brief Gets the date when the adjustment rule begins. */
            [[nodiscard]] DateTime       getDateStartProperty()               const { return dateStart_; }
            /** @brief Gets the date when the adjustment rule ends. */
            [[nodiscard]] DateTime       getDateEndProperty()                 const { return dateEnd_; }
            /** @brief Gets the time difference between standard time and DST. */
            [[nodiscard]] TimeSpan       getDaylightDeltaProperty()           const { return daylightDelta_; }
            /** @brief Gets the start transition for DST. */
            [[nodiscard]] TransitionTime getDaylightTransitionStartProperty() const { return daylightTransitionStart_; }
            /** @brief Gets the end transition for DST. */
            [[nodiscard]] TransitionTime getDaylightTransitionEndProperty()   const { return daylightTransitionEnd_; }

            /**
             * @brief Gets the time difference with the base UTC offset for the time zone
             * during the adjustment-rule period.
             *
             * C++ counterpart of .NET AdjustmentRule.BaseUtcOffsetDelta.
             */
            [[nodiscard]] TimeSpan       getBaseUtcOffsetDeltaProperty()     const { return baseUtcOffsetDelta_; }

            /**
             * @brief Gets a value indicating that this rule fixes the time zone offset
             * from DateStart to DateEnd without any daylight transitions in between.
             *
             * C++ counterpart of .NET AdjustmentRule.NoDaylightTransitions (internal).
             */
            [[nodiscard]] bool           getNoDaylightTransitionsProperty()  const { return noDaylightTransitions_; }

            /**
             * @brief Gets a value indicating whether this rule involves a DST change.
             *
             * C++ counterpart of .NET AdjustmentRule.HasDaylightSaving (internal).
             * Returns true when DaylightDelta is non-zero or either transition time is
             * non-default.
             */
            [[nodiscard]] bool getHasDaylightSavingProperty() const {
                static const TransitionTime kDefault{};
                return daylightDelta_ != TimeSpan::Zero ||
                       !(daylightTransitionStart_ == kDefault) ||
                       !(daylightTransitionEnd_   == kDefault);
            }

            /**
             * @brief Returns a hash code for this AdjustmentRule.
             *
             * C++ counterpart of .NET AdjustmentRule.GetHashCode().
             */
            [[nodiscard]] intcs GetHashCode() const noexcept {
                auto ticks = dateStart_.getTicksProperty();
                return static_cast<intcs>(ticks ^ (ticks >> 32));
            }

            /**
             * @brief Returns true if this rule is equal to @p other.
             *
             * C++ counterpart of .NET AdjustmentRule.Equals(AdjustmentRule).
             * Two rules are equal when all fields (including BaseUtcOffsetDelta) match.
             */
            [[nodiscard]] bool Equals(const AdjustmentRule& other) const {
                return dateStart_            == other.dateStart_            &&
                       dateEnd_              == other.dateEnd_              &&
                       daylightDelta_        == other.daylightDelta_        &&
                       baseUtcOffsetDelta_   == other.baseUtcOffsetDelta_   &&
                       daylightTransitionStart_ == other.daylightTransitionStart_ &&
                       daylightTransitionEnd_   == other.daylightTransitionEnd_;
            }

            bool operator==(const AdjustmentRule& o) const { return Equals(o); }
            bool operator!=(const AdjustmentRule& o) const { return !Equals(o); }

            /**
             * @brief Creates an adjustment rule with zero BaseUtcOffsetDelta.
             *
             * C++ counterpart of .NET AdjustmentRule.CreateAdjustmentRule(DateTime, DateTime,
             * TimeSpan, TransitionTime, TransitionTime).
             */
            static std::shared_ptr<AdjustmentRule> CreateAdjustmentRule(
                DateTime dateStart, DateTime dateEnd, TimeSpan daylightDelta,
                TransitionTime daylightTransitionStart, TransitionTime daylightTransitionEnd)
            {
                auto r = std::shared_ptr<AdjustmentRule>(new AdjustmentRule());
                r->dateStart_               = dateStart;
                r->dateEnd_                 = dateEnd;
                r->daylightDelta_           = daylightDelta;
                r->daylightTransitionStart_ = daylightTransitionStart;
                r->daylightTransitionEnd_   = daylightTransitionEnd;
                r->baseUtcOffsetDelta_      = TimeSpan::Zero;
                r->noDaylightTransitions_   = false;
                return r;
            }

            /**
             * @brief Creates an adjustment rule with an explicit BaseUtcOffsetDelta.
             *
             * C++ counterpart of .NET AdjustmentRule.CreateAdjustmentRule(DateTime, DateTime,
             * TimeSpan, TransitionTime, TransitionTime, TimeSpan).
             * @param baseUtcOffsetDelta The delta from the zone's default UTC offset that
             *                           applies during this rule's period.
             */
            static std::shared_ptr<AdjustmentRule> CreateAdjustmentRule(
                DateTime dateStart, DateTime dateEnd, TimeSpan daylightDelta,
                TransitionTime daylightTransitionStart, TransitionTime daylightTransitionEnd,
                TimeSpan baseUtcOffsetDelta)
            {
                auto r = std::shared_ptr<AdjustmentRule>(new AdjustmentRule());
                r->dateStart_               = dateStart;
                r->dateEnd_                 = dateEnd;
                r->daylightDelta_           = daylightDelta;
                r->daylightTransitionStart_ = daylightTransitionStart;
                r->daylightTransitionEnd_   = daylightTransitionEnd;
                r->baseUtcOffsetDelta_      = baseUtcOffsetDelta;
                r->noDaylightTransitions_   = false;
                return r;
            }
        };

    private:
        std::string id_;
        std::string displayName_;
        std::string standardName_;
        std::string daylightName_;
        TimeSpan    baseUtcOffset_;
        bool        supportsDst_ = false;

        TimeZoneInfo(std::string id, TimeSpan baseUtcOffset,
                     std::string displayName, std::string standardName,
                     std::string daylightName, bool supportsDst)
            : id_(std::move(id)), displayName_(std::move(displayName)),
              standardName_(std::move(standardName)), daylightName_(std::move(daylightName)),
              baseUtcOffset_(baseUtcOffset), supportsDst_(supportsDst) {}

    public:
        // =====================================================================
        // Properties
        // =====================================================================

        /**
         * @brief Gets the IANA or well-known identifier of this time zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.Id.
         */
        [[nodiscard]] const std::string& getIdProperty()           const { return id_; }

        /**
         * @brief Gets a human-readable display name for this time zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.DisplayName.
         */
        [[nodiscard]] const std::string& getDisplayNameProperty()  const { return displayName_; }

        /**
         * @brief Gets the standard (non-DST) name.
         *
         * C++ counterpart of .NET TimeZoneInfo.StandardName.
         */
        [[nodiscard]] const std::string& getStandardNameProperty() const { return standardName_; }

        /**
         * @brief Gets the daylight-saving name.
         *
         * C++ counterpart of .NET TimeZoneInfo.DaylightName.
         * May equal the standard name when DST is not observed.
         */
        [[nodiscard]] const std::string& getDaylightNameProperty() const { return daylightName_; }

        /**
         * @brief Gets the fixed UTC offset for this zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.BaseUtcOffset.
         * DST transitions are not modelled; this is always the standard offset.
         */
        [[nodiscard]] TimeSpan getBaseUtcOffsetProperty()          const { return baseUtcOffset_; }

        /**
         * @brief Gets a value indicating whether this zone ever observes DST.
         *
         * C++ counterpart of .NET TimeZoneInfo.SupportsDaylightSavingTime.
         */
        [[nodiscard]] bool getSupportsDaylightSavingTimeProperty() const { return supportsDst_; }

        /**
         * @brief Gets a value indicating whether the time zone ID has the IANA format.
         *
         * C++ counterpart of .NET TimeZoneInfo.HasIanaId.
         * Returns true when the ID contains a '/' (e.g. "Europe/Prague").
         */
        [[nodiscard]] bool getHasIanaIdProperty() const {
            return id_.find('/') != std::string::npos;
        }

        // =====================================================================
        // Instance methods
        // =====================================================================

        /**
         * @brief Returns false — DST transitions are not modelled.
         *
         * C++ counterpart of .NET TimeZoneInfo.IsDaylightSavingTime(DateTime).
         */
        [[nodiscard]] bool IsDaylightSavingTime(const DateTime& /*dt*/) const { return false; }

        /**
         * @brief Returns false — DST transitions are not modelled.
         *
         * C++ counterpart of .NET TimeZoneInfo.IsDaylightSavingTime(DateTimeOffset).
         */
        [[nodiscard]] bool IsDaylightSavingTime(const DateTimeOffset& /*dt*/) const { return false; }

        /**
         * @brief Returns the fixed UTC offset for any DateTime.
         *
         * C++ counterpart of .NET TimeZoneInfo.GetUtcOffset(DateTime).
         */
        [[nodiscard]] TimeSpan GetUtcOffset(const DateTime& /*dt*/)       const { return baseUtcOffset_; }

        /**
         * @brief Returns the fixed UTC offset for any DateTimeOffset.
         *
         * C++ counterpart of .NET TimeZoneInfo.GetUtcOffset(DateTimeOffset).
         */
        [[nodiscard]] TimeSpan GetUtcOffset(const DateTimeOffset& /*dt*/) const { return baseUtcOffset_; }

        /**
         * @brief Returns false — ambiguous times are not modelled.
         *
         * C++ counterpart of .NET TimeZoneInfo.IsAmbiguousTime(DateTime).
         */
        [[nodiscard]] bool IsAmbiguousTime(const DateTime& /*dt*/)        const { return false; }

        /**
         * @brief Returns false — ambiguous times are not modelled.
         *
         * C++ counterpart of .NET TimeZoneInfo.IsAmbiguousTime(DateTimeOffset).
         */
        [[nodiscard]] bool IsAmbiguousTime(const DateTimeOffset& /*dt*/)  const { return false; }

        /**
         * @brief Returns false — invalid times are not modelled.
         *
         * C++ counterpart of .NET TimeZoneInfo.IsInvalidTime(DateTime).
         */
        [[nodiscard]] bool IsInvalidTime(const DateTime& /*dt*/)          const { return false; }

        /**
         * @brief Returns an empty array — DST transitions are not modelled.
         *
         * C++ counterpart of .NET TimeZoneInfo.GetAmbiguousTimeOffsets(DateTime).
         */
        [[nodiscard]] std::vector<TimeSpan> GetAmbiguousTimeOffsets(const DateTime& /*dt*/) const {
            return {};
        }

        /**
         * @brief Returns an empty array — DST transitions are not modelled.
         *
         * C++ counterpart of .NET TimeZoneInfo.GetAmbiguousTimeOffsets(DateTimeOffset).
         */
        [[nodiscard]] std::vector<TimeSpan> GetAmbiguousTimeOffsets(const DateTimeOffset& /*dt*/) const {
            return {};
        }

        /**
         * @brief Returns an empty array — DST adjustment rules are not modelled.
         *
         * C++ counterpart of .NET TimeZoneInfo.GetAdjustmentRules().
         */
        [[nodiscard]] std::vector<std::shared_ptr<AdjustmentRule>> GetAdjustmentRules() const {
            return {};
        }

        /**
         * @brief Converts a DateTime to UTC by subtracting the zone's base UTC offset.
         *
         * C++ counterpart of the instance form of .NET TimeZoneInfo.ConvertTimeToUtc(DateTime).
         */
        [[nodiscard]] DateTime ConvertTimeToUtc(const DateTime& dt) const {
            return dt.Add(-baseUtcOffset_);
        }

        /**
         * @brief Returns true if this zone has the same base UTC offset and DST support as @p other.
         *
         * C++ counterpart of .NET TimeZoneInfo.HasSameRules(TimeZoneInfo).
         */
        [[nodiscard]] bool HasSameRules(const TimeZoneInfo& other) const {
            return baseUtcOffset_ == other.baseUtcOffset_ &&
                   supportsDst_   == other.supportsDst_;
        }

        /**
         * @brief Returns true if this zone has the same ID as @p other.
         *
         * C++ counterpart of .NET TimeZoneInfo.Equals(TimeZoneInfo).
         */
        [[nodiscard]] bool Equals(const TimeZoneInfo& other) const { return id_ == other.id_; }

        /**
         * @brief Returns a hash code based on the zone ID (case-insensitive).
         *
         * C++ counterpart of .NET TimeZoneInfo.GetHashCode().
         */
        [[nodiscard]] intcs GetHashCode() const {
            std::string lower = id_;
            for (auto& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return static_cast<intcs>(std::hash<std::string>{}(lower));
        }

        /**
         * @brief Returns the display name of this time zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.ToString().
         */
        [[nodiscard]] std::string ToString() const { return displayName_; }

        bool operator==(const TimeZoneInfo& other) const { return Equals(other); }
        bool operator!=(const TimeZoneInfo& other) const { return !Equals(other); }

        // =====================================================================
        // Static properties
        // =====================================================================

        /**
         * @brief Returns the UTC singleton (offset zero, no DST).
         *
         * C++ counterpart of .NET TimeZoneInfo.Utc.
         */
        static const TimeZoneInfo& Utc() {
            static TimeZoneInfo tz("UTC", TimeSpan::Zero,
                                   "Coordinated Universal Time",
                                   "Coordinated Universal Time",
                                   "Coordinated Universal Time", false);
            return tz;
        }

        /**
         * @brief Returns the local system time zone by reading the OS timezone via POSIX localtime_r().
         *
         * C++ counterpart of .NET TimeZoneInfo.Local.
         * The offset reflects the current wall-clock offset (including any active DST).
         * On Emscripten, returns UTC.
         */
        static const TimeZoneInfo& Local();

        // =====================================================================
        // Static methods
        // =====================================================================

        /**
         * @brief Clears the cached local time zone data.
         *
         * C++ counterpart of .NET TimeZoneInfo.ClearCachedData().
         * This implementation is a no-op because Local() uses a block-scope static.
         */
        static void ClearCachedData() { /* local static cannot be reset */ }

        /**
         * @brief Looks up a time zone by IANA ID (e.g. "Europe/Prague").
         *
         * C++ counterpart of .NET TimeZoneInfo.FindSystemTimeZoneById(string).
         * On Linux: checks /usr/share/zoneinfo/ and probes the offset via setenv("TZ").
         * On Windows: uses the IANA→Windows CLDR mapping table.
         * @throws System::TimeZoneNotFoundException if the ID is not found.
         */
        static std::shared_ptr<TimeZoneInfo> FindSystemTimeZoneById(const std::string& id);

        /**
         * @brief Tries to find a time zone by ID; returns false instead of throwing.
         *
         * C++ counterpart of .NET TimeZoneInfo.TryFindSystemTimeZoneById(string, out TimeZoneInfo).
         */
        static bool TryFindSystemTimeZoneById(const std::string& id,
                                              std::shared_ptr<TimeZoneInfo>& result) {
            try {
                result = FindSystemTimeZoneById(id);
                return true;
            } catch (...) {
                return false;
            }
        }

        /**
         * @brief Returns all known system time zones (UTC + Local in this implementation).
         *
         * C++ counterpart of .NET TimeZoneInfo.GetSystemTimeZones().
         */
        static std::vector<std::shared_ptr<TimeZoneInfo>> GetSystemTimeZones();

        /**
         * @brief Returns all known system time zones; skipSorting is ignored.
         *
         * C++ counterpart of .NET TimeZoneInfo.GetSystemTimeZones(bool skipSorting).
         */
        static std::vector<std::shared_ptr<TimeZoneInfo>> GetSystemTimeZones(bool /*skipSorting*/) {
            return GetSystemTimeZones();
        }

        /**
         * @brief Creates a fixed-offset custom time zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.CreateCustomTimeZone(string, TimeSpan, string, string).
         */
        static std::shared_ptr<TimeZoneInfo> CreateCustomTimeZone(
            const std::string& id, const TimeSpan& utcOffset,
            const std::string& displayName, const std::string& standardName)
        {
            return std::shared_ptr<TimeZoneInfo>(
                new TimeZoneInfo(id, utcOffset, displayName, standardName, standardName, false));
        }

        /**
         * @brief Converts @p dt (assumed UTC) to the zone identified by @p destinationTimeZoneId.
         *
         * C++ counterpart of .NET TimeZoneInfo.ConvertTimeBySystemTimeZoneId(DateTime, string).
         */
        static DateTime ConvertTimeBySystemTimeZoneId(const DateTime& dt,
                                                      const std::string& destinationTimeZoneId) {
            auto tz = FindSystemTimeZoneById(destinationTimeZoneId);
            return dt.Add(tz->baseUtcOffset_);
        }

        /**
         * @brief Converts @p dt (assumed UTC) to the zone identified by @p destinationTimeZoneId.
         *
         * C++ counterpart of .NET TimeZoneInfo.ConvertTimeBySystemTimeZoneId(DateTime, string, string).
         */
        static DateTime ConvertTimeBySystemTimeZoneId(const DateTime& dt,
                                                      const std::string& sourceTimeZoneId,
                                                      const std::string& destinationTimeZoneId) {
            auto src = FindSystemTimeZoneById(sourceTimeZoneId);
            auto dst = FindSystemTimeZoneById(destinationTimeZoneId);
            DateTime utc = dt.Add(-src->baseUtcOffset_);
            return utc.Add(dst->baseUtcOffset_);
        }

        /**
         * @brief Converts a DateTimeOffset to the specified destination time zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.ConvertTime(DateTimeOffset, TimeZoneInfo).
         */
        static DateTimeOffset ConvertTime(const DateTimeOffset& dto,
                                          const TimeZoneInfo& destinationTimeZone) {
            return DateTimeOffset(dto.getUtcDateTimeProperty().Add(destinationTimeZone.baseUtcOffset_),
                                  destinationTimeZone.baseUtcOffset_);
        }

        /**
         * @brief Converts @p dt (assumed UTC) to the specified destination time zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.ConvertTime(DateTime, TimeZoneInfo).
         */
        static DateTime ConvertTime(const DateTime& dt, const TimeZoneInfo& destinationTimeZone) {
            return dt.Add(destinationTimeZone.baseUtcOffset_);
        }

        /**
         * @brief Converts @p dt from @p sourceTimeZone to @p destinationTimeZone.
         *
         * C++ counterpart of .NET TimeZoneInfo.ConvertTime(DateTime, TimeZoneInfo, TimeZoneInfo).
         */
        static DateTime ConvertTime(const DateTime& dt,
                                    const TimeZoneInfo& sourceTimeZone,
                                    const TimeZoneInfo& destinationTimeZone) {
            DateTime utc = dt.Add(-sourceTimeZone.baseUtcOffset_);
            return utc.Add(destinationTimeZone.baseUtcOffset_);
        }

        /**
         * @brief Converts a UTC DateTime to the specified destination time zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.ConvertTimeFromUtc(DateTime, TimeZoneInfo).
         */
        static DateTime ConvertTimeFromUtc(const DateTime& dt,
                                           const TimeZoneInfo& destinationTimeZone) {
            return dt.Add(destinationTimeZone.baseUtcOffset_);
        }

        /**
         * @brief Converts a DateTime to UTC using the specified source time zone.
         *
         * C++ counterpart of .NET TimeZoneInfo.ConvertTimeToUtc(DateTime, TimeZoneInfo).
         */
        static DateTime ConvertTimeToUtc(const DateTime& dt, const TimeZoneInfo& sourceTimeZone) {
            return dt.Add(-sourceTimeZone.baseUtcOffset_);
        }

        /**
         * @brief Converts a DateTimeOffset to the zone identified by @p destinationTimeZoneId.
         *
         * C++ counterpart of .NET TimeZoneInfo.ConvertTimeBySystemTimeZoneId(DateTimeOffset, string).
         */
        static DateTimeOffset ConvertTimeBySystemTimeZoneId(const DateTimeOffset& dto,
                                                            const std::string& destinationTimeZoneId) {
            auto tz = FindSystemTimeZoneById(destinationTimeZoneId);
            return ConvertTime(dto, *tz);
        }

        /**
         * @brief Tries to map an IANA timezone ID to a Windows timezone ID.
         *
         * C++ counterpart of .NET TimeZoneInfo.TryConvertIanaIdToWindowsId(string, out string).
         * Uses the CLDR-derived IANA→Windows mapping table.
         * @param ianaId    The IANA timezone identifier (e.g. "Europe/Prague").
         * @param windowsId Receives the Windows timezone name on success.
         * @return true if a mapping was found; false otherwise.
         */
        static bool TryConvertIanaIdToWindowsId(const std::string& ianaId, std::string& windowsId);

        /**
         * @brief Tries to map a Windows timezone ID to an IANA timezone ID.
         *
         * C++ counterpart of .NET TimeZoneInfo.TryConvertWindowsIdToIanaId(string, out string).
         * Returns the first IANA ID that maps to @p windowsId in the CLDR table.
         * @param windowsId The Windows timezone identifier (e.g. "Central Europe Standard Time").
         * @param ianaId    Receives the IANA timezone name on success.
         * @return true if a mapping was found; false otherwise.
         */
        static bool TryConvertWindowsIdToIanaId(const std::string& windowsId, std::string& ianaId);

    };

} // namespace System
