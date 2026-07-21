// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <atomic>
#include <chrono>
#include <cmath>
#include <limits>
#include <sstream>
#include <string>

#include "IComparable.hpp"
#include "IEquatable.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {
    using SharpRuntime::intcs;
    using SharpRuntime::longcs;

    /**
     * @brief Represents a duration of time, which can be either positive or negative.
     *
     * TimeSpan is stored internally as a number of ticks, where a tick represents
     * 100 nanoseconds. This allows precise representation of hours, minutes, and
     * days. Months and years are not directly representable because calendar
     * arithmetic depends on which specific dates are involved.
     *
     * C++ counterpart of .NET System.TimeSpan.
     */
    struct TimeSpan : IEquatable<TimeSpan>, IComparable<TimeSpan> {
    private:
        static std::atomic<int> copy_count;
        static std::atomic<int> move_count;

    public:
        /** Returns how many times a TimeSpan has been copy-constructed (diagnostic). */
        static int getCopyCount();
        /** Returns how many times a TimeSpan has been move-constructed (diagnostic). */
        static int getMoveCount();

        /** Resets the copy-construction counter to zero. */
        static void resetCopyCount();
        /** Resets the move-construction counter to zero. */
        static void resetMoveCount();

    public:
        /**
         * @brief Defines the number of nanoseconds in 1 tick.
         */
        static constexpr longcs NanosecondsPerTick = 100;

#ifdef XNA5
        /**
         * @brief Defines the number of ticks in 1 nanosecond.
         */
        static constexpr double TicksPerNanosecond = 0.01f;
#endif
        /**
         * @brief Defines the number of ticks in 1 microsecond.
         */
        static constexpr longcs TicksPerMicrosecond = 10;

        /**
         * @brief Defines the number of ticks in 1 millisecond.
         */
        static constexpr longcs TicksPerMillisecond = TicksPerMicrosecond * 1000; // 10,000
        /**
         * @brief Defines the number of ticks in 1 second.
         */
        static constexpr longcs TicksPerSecond = TicksPerMillisecond * 1000; // 10,000,000
        /**
         * @brief Defines the number of ticks in 1 minute.
         */
        static constexpr longcs TicksPerMinute = TicksPerSecond * 60; // 600,000,000
        /**
         * @brief Defines the number of ticks in 1 hour.
         */
        static constexpr longcs TicksPerHour = TicksPerMinute * 60; // 36,000,000,000
        /**
         * @brief Defines the number of ticks in 1 day.
         */
        static constexpr longcs TicksPerDay = TicksPerHour * 24; // 864,000,000,000

    private:
        static constexpr longcs MaxSeconds = SharpRuntime::LONGCS_MAX / TicksPerSecond;
        static constexpr longcs MinSeconds = SharpRuntime::LONGCS_MIN / TicksPerSecond;

        static constexpr longcs MaxMilliSeconds = SharpRuntime::LONGCS_MAX / TicksPerMillisecond;
        static constexpr longcs MinMilliSeconds = SharpRuntime::LONGCS_MIN / TicksPerMillisecond;

        static constexpr longcs MaxMicroSeconds = SharpRuntime::LONGCS_MAX / TicksPerMicrosecond;
        static constexpr longcs MinMicroSeconds = SharpRuntime::LONGCS_MIN / TicksPerMicrosecond;

        static constexpr longcs TicksPerTenthSecond = TicksPerMillisecond * 100;

    public:
        /** @brief A TimeSpan of zero duration. C++ counterpart of .NET TimeSpan.Zero. */
        static const TimeSpan Zero;

        /** @brief The maximum representable TimeSpan. C++ counterpart of .NET TimeSpan.MaxValue. */
        static const TimeSpan MaxValue;

        /** @brief The minimum representable TimeSpan. C++ counterpart of .NET TimeSpan.MinValue. */
        static const TimeSpan MinValue;

    private:
        longcs ticks_internal;

    public:
        /**
         * @brief Initializes a new instance of the TimeSpan structure to zero ticks.
         */
        TimeSpan();

    public:
        /**
         * @brief Initializes a new instance of the TimeSpan structure to a specified number of ticks.
         *
         * @param ticks A time period expressed in 100-nanosecond units.
         */
        TimeSpan(longcs ticks);

    public:
        /**
         * @brief Initializes a new instance of the TimeSpan structure to a specified number of hours, minutes, and seconds.
         *
         * @param hours Number of hours.
         * @param minutes Number of minutes.
         * @param seconds Number of seconds.
         */
        TimeSpan(intcs hours, intcs minutes, intcs seconds);

        /**
         * @brief Constructs a TimeSpan object using specified time components.
         *
         * Creates a TimeSpan instance based on the given number of days, hours, minutes, seconds,
         * milliseconds, and microseconds.
         *
         * @param days The number of days.
         * @param hours The number of hours.
         * @param minutes The number of minutes.
         * @param seconds The number of seconds.
         * @param milliseconds The number of milliseconds.
         * @param microseconds The number of microseconds.
         *
         * @details The provided values are converted into internal tick units to initialize this instance.
         *
         * @throws System::ArgumentOutOfRangeException If the calculated TimeSpan falls outside the acceptable range.
         * The acceptable range is between TimeSpan at least MinValue and at most MaxValue.
         */
    public:
        TimeSpan(intcs days, intcs hours, intcs minutes, intcs seconds, intcs milliseconds = 0,
                 intcs microseconds = 0);

    public:
        /** Copy-assignment operator. */
        TimeSpan &operator=(const TimeSpan &);

        /** Copy constructor. */
        TimeSpan(const TimeSpan& other);

        /** Move constructor. */
        TimeSpan(TimeSpan&& other) noexcept;

        /** Move-assignment operator. */
        TimeSpan& operator=(TimeSpan&& other) noexcept;

    private:
        static longcs TimeToTicks(intcs days, intcs hours, intcs minutes, intcs seconds, intcs milliseconds,
                                  intcs microseconds);

    public:
        /** Returns the total number of ticks (100-nanosecond units) in this TimeSpan. */
        [[nodiscard]] longcs getTicksProperty() const;

    public:
        /** Returns the days component of this TimeSpan. */
        [[nodiscard]] intcs getDaysProperty() const;

    public:
        /** Returns the hours component of this TimeSpan. */
        [[nodiscard]] intcs getHoursProperty() const;

    public:
        /** Returns the milliseconds component of this TimeSpan. */
        [[nodiscard]] intcs getMillisecondsProperty() const;

        /**
         * @brief Retrieves the microseconds portion of the time span.
         *
         * Provides access to the microsecond component of the current TimeSpan instance.
         *
         * @return The number of whole microseconds in the time span.
         *
         * @details The `Microseconds` property returns only complete microseconds, while
         * `TotalMicroseconds` provides both whole and fractional values.
         */

    public:
        /** Returns the microseconds component of this TimeSpan. */
        [[nodiscard]] intcs getMicrosecondsProperty() const;

        /**
         * @brief Retrieves the nanosecond portion of the time span.
         *
         * Provides access to the nanosecond component of the current TimeSpan instance.
         *
         * @return The number of whole nanoseconds in the time span.
         *
         * @details The `Nanoseconds` property returns only full nanoseconds, while
         * `TotalNanoseconds` provides both complete and fractional values.
         */

    public:
        /** Returns the nanoseconds component of this TimeSpan. */
        [[nodiscard]] intcs getNanosecondsProperty() const;

    public:
        /** Returns the minutes component of this TimeSpan. */
        [[nodiscard]] intcs getMinutesProperty() const;

    public:
        /** Returns the seconds component of this TimeSpan. */
        [[nodiscard]] intcs getSecondsProperty() const;

    public:
        /** Returns the total number of days, including fractional days. */
        [[nodiscard]] double getTotalDaysProperty() const;

    public:
        /** Returns the total number of hours, including fractional hours. */
        [[nodiscard]] double getTotalHoursProperty() const;

    public:
        /** Returns the total number of milliseconds, including fractional milliseconds. */
        [[nodiscard]] double getTotalMillisecondsProperty() const;

        /**
         * @brief Returns the time span value in whole and fractional microseconds.
         *
         * Converts the internal tick-based representation into microseconds,
         * including both complete and partial microseconds.
         *
         * @return The total number of microseconds, including fractional values.
         *
         * @details The `TotalMicroseconds` property provides both whole and fractional microseconds,
         * while `Microseconds` only returns complete microseconds.
         */

    public:
        /** Returns the total number of microseconds, including fractional microseconds. */
        [[nodiscard]] double getTotalMicrosecondsProperty() const;

        /**
         * @brief Returns the time span value in whole and fractional nanoseconds.
         *
         * Converts the internal tick-based representation into nanoseconds,
         * including both complete and fractional values.
         *
         * @return The total number of nanoseconds, including fractional values.
         *
         * @details The `TotalNanoseconds` property provides both whole and fractional nanoseconds,
         * while `Nanoseconds` only returns complete nanoseconds.
         */
    public:
        /** Returns the total number of nanoseconds, including fractional nanoseconds. */
        [[nodiscard]] double getTotalNanosecondsProperty() const;

    public:
        /** Returns the total number of minutes, including fractional minutes. */
        [[nodiscard]] double getTotalMinutesProperty() const;

    public:
        /** Returns the total number of seconds, including fractional seconds. */
        [[nodiscard]] double getTotalSecondsProperty() const;

        /**
         * @brief Adds the specified TimeSpan to the current TimeSpan instance.
         *
         * Combines the time duration of the current instance with that of the specified
         * `TimeSpan` parameter, producing a new `TimeSpan` that represents their sum.
         *
         * @param ts A reference to the `TimeSpan` object to be added.
         *
         * @return A new `TimeSpan` representing the combined duration of the two instances.
         *
         * @throws OverflowException If the resulting time span exceeds the valid range for a `TimeSpan`.
         *
         * @details This method ensures that the resulting time span does not exceed the
         * limits supported by the `TimeSpan`. If an overflow occurs during the addition,
         * an `OverflowException` is thrown.
         */
    public:
        [[nodiscard]] TimeSpan Add(const TimeSpan &ts) const;

        /**
         * @brief Compares two TimeSpan instances to determine their relative values.
         *
         * Determines whether the first TimeSpan is shorter than, equal to, or longer than
         * the second TimeSpan, based on their internal tick counts.
         *
         * @param t1 The first TimeSpan instance to compare.
         * @param t2 The second TimeSpan instance to compare.
         * @return An integer representing the comparison result:
         *         - Returns 1 if t1 is greater than t2.
         *         - Returns -1 if t1 is less than t2.
         *         - Returns 0 if t1 is equal to t2.
         */
    public:
        static intcs Compare(const TimeSpan &t1, const TimeSpan &t2);

    public:
        /** @copydoc IComparable::CompareTo */
        [[nodiscard]] intcs CompareTo(const TimeSpan &value) const override;

    public:
        /** Returns a TimeSpan that represents the specified number of days. */
        static TimeSpan FromDays(double value);

        /**
         * @brief Retrieves the absolute duration of the current TimeSpan instance.
         *
         * This method calculates and returns a new TimeSpan instance representing
         * the absolute value of the current TimeSpan's duration. If the TimeSpan
         * represents a negative duration, it is converted to its corresponding positive value.
         *
         * @return A TimeSpan instance representing the absolute duration.
         *
         * @throws OverflowException If the TimeSpan is set to its minimum value,
         * which cannot be represented as a positive value due to overflow.
         */
    public:
        [[nodiscard]] TimeSpan Duration() const;

        /**
         * @brief Determines whether the current TimeSpan instance is equal to another specified TimeSpan instance.
         *
         * Compares the internal tick count of two TimeSpan objects to determine equality.
         * Two TimeSpan instances are considered equal if their internal tick values are the same.
         *
         * @param obj The TimeSpan instance to compare with the current instance.
         * @return True if the specified TimeSpan instance has the same tick count as the current instance; otherwise, false.
         */
    public:
        [[nodiscard]] bool Equals(const TimeSpan &obj) const override;

    public:
        /** Returns true if @p t1 and @p t2 have the same tick count. */
        static bool Equals(const TimeSpan &t1, const TimeSpan &t2);

    public:
        /** Returns a hash code for this TimeSpan, based on its tick count. */
        [[nodiscard]] intcs GetHashCode() const noexcept;

    public:
        /** Returns a TimeSpan that represents the specified number of hours. */
        static TimeSpan FromHours(double value);

    private:
        static TimeSpan Interval(double value, double scale);

    private:
        static TimeSpan IntervalFromDoubleTicks(double ticks);

        /**
         * @brief Creates a TimeSpan object representing a specific number of milliseconds.
         *
         * Converts the given value, representing a time duration in milliseconds, into a TimeSpan instance.
         *
         * @param value The number of milliseconds to be represented as a TimeSpan. This can be positive or negative.
         * @return A TimeSpan object corresponding to the provided number of milliseconds.
         *
         * @details If the `value` is fractional, the fractional part is converted into ticks, as TimeSpan uses ticks
         * for internal representation. A single tick equals 100 nanoseconds, which provides precision in the conversion.
         */
    public:
        static TimeSpan FromMilliseconds(double value);

    public:
        /** Returns a TimeSpan that represents the specified number of microseconds. */
        static TimeSpan FromMicroseconds(double value);

    public:
        /** Returns a TimeSpan that represents the specified number of minutes. */
        static TimeSpan FromMinutes(double value);

    public:
        /** Returns a new TimeSpan with the negated tick value. */
        [[nodiscard]] TimeSpan Negate() const;

    public:
        /** Returns a TimeSpan that represents the specified number of seconds. */
        static TimeSpan FromSeconds(double value);

    public:
        /**
         * @brief Subtracts @p ts from the current TimeSpan and returns the result.
         * @param ts The TimeSpan to subtract.
         */
        [[nodiscard]] TimeSpan Subtract(const TimeSpan &ts) const;

    public:
        /**
         * @brief Returns a new TimeSpan scaled by @p factor.
         * @param factor Scalar multiplier.
         */
        TimeSpan Multiply(double factor) const;

    public:
        /**
         * @brief Returns a new TimeSpan divided by scalar @p divisor.
         * @param divisor Scalar divisor.
         */
        TimeSpan Divide(double divisor) const;

    public:
        /**
         * @brief Returns the ratio of this TimeSpan to @p ts.
         * @param ts Divisor TimeSpan (may be zero, producing infinity or NaN).
         */
        [[nodiscard]] double Divide(const TimeSpan &ts) const;

    public:
        /** Returns a TimeSpan that represents the specified number of ticks. */
        static TimeSpan FromTicks(longcs value);

        //[MethodImpl(MethodImplOptions.AggressiveInlining)]
    private:
        static longcs TimeToTicks(intcs hour, intcs minute, intcs second);

    public:
        /** Returns the string representation of this TimeSpan (format: [-]d.hh:mm:ss.fffffff). */
        [[nodiscard]] std::string ToString() const;

        /**
         * Returns the TimeSpan formatted according to @p format.
         * Tokens: d (days), hh/h (hours), mm/m (minutes), ss/s (seconds), f–fffffff (fractional seconds).
         * Literal text may be enclosed in single quotes.
         */
        [[nodiscard]] std::string ToString(const std::string& format) const;

        /**
         * Parses a TimeSpan from a string in the form [-][d.]hh:mm:ss[.fffffff].
         * @throws System::FormatException if the string cannot be parsed.
         */
        [[nodiscard]] static TimeSpan Parse(const std::string& s);

        /** Tries to parse a TimeSpan string; returns false without throwing on failure. */
        static bool TryParse(const std::string& s, TimeSpan& result);

    public:
        /** Unary minus: returns the negated TimeSpan. */
        TimeSpan operator-() const;

    public:
        /** Subtracts @p t2 from this TimeSpan. */
        TimeSpan operator-(const TimeSpan &t2) const;

    public:
        /** Unary plus: returns this TimeSpan unchanged. */
        TimeSpan operator+() const;

    public:
        /** Adds @p t2 to this TimeSpan. */
        TimeSpan operator+(const TimeSpan &t2) const;

    public:
        /** Multiplies this TimeSpan by scalar @p factor. */
        TimeSpan operator*(double factor) const;

        /** Divides this TimeSpan by scalar @p divisor. */
        TimeSpan operator/(double divisor) const;

        // Performing division using floating-point arithmetic allows the result to be infinity,
        // which makes sense when dividing a non-zero TimeSpan by TimeSpan.Zero.
        // For example, TimeSpan.FromHours(1) / TimeSpan.Zero asks how many zero-length intervals fit into one hour,
        // and mathematically, the answer is infinity.
        // Dividing TimeSpan.Zero by TimeSpan.Zero returns NaN, which may be less practical,
        // but it is no less valid than throwing an exception.

    public:
        /** Returns the ratio of this TimeSpan to @p t2 as a double. */
        double operator/(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan equals @p t2. */
        bool operator==(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan is not equal to @p t2. */
        bool operator!=(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan is less than @p t2. */
        bool operator<(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan is less than or equal to @p t2. */
        bool operator<=(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan is greater than @p t2. */
        bool operator>(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan is greater than or equal to @p t2. */
        bool operator>=(const TimeSpan &t2) const;
    };


    /**
     * @brief Multiplies a scalar @p factor by a @p timeSpan.
     * @param factor Scalar multiplier.
     * @param timeSpan The TimeSpan to scale.
     * @return A new TimeSpan scaled by @p factor.
     */
    TimeSpan operator*(double factor, const TimeSpan &timeSpan);
} // System
