// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <limits>
#include <string>

#include "IComparable.hpp"
#include "IEquatable.hpp"
#include "System/ArgumentException.hpp"
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
    public:
        /**
         * @brief Defines the number of ticks in 1 millisecond.
         */
        static constexpr longcs TicksPerMillisecond = 10000; // 10 * 1000

        /**
         * @brief Defines the number of ticks in 1 second.
         */
        static constexpr longcs TicksPerSecond = TicksPerMillisecond * 1000; // 10,000,000

        /**
         * @brief Defines the number of ticks in 1 minute.
         */
        static constexpr longcs TicksPerMinute = TicksPerSecond * 60; // 600,000,000

    private:
        static constexpr longcs MaxMilliSeconds = SharpRuntime::LONGCS_MAX / TicksPerMillisecond;
        static constexpr longcs MinMilliSeconds = SharpRuntime::LONGCS_MIN / TicksPerMillisecond;

    public:
        /** @brief A TimeSpan of zero duration. C++ counterpart of .NET TimeSpan.Zero. */
        static const TimeSpan Zero;

        /** @brief The maximum representable TimeSpan. C++ counterpart of .NET TimeSpan.MaxValue. */
        static const TimeSpan MaxValue;

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
        /** Copy-assignment operator. */
        TimeSpan &operator=(const TimeSpan &);

        /** Copy constructor. */
        TimeSpan(const TimeSpan& other);

        /** Move constructor. */
        TimeSpan(TimeSpan&& other) noexcept;

        /** Move-assignment operator. */
        TimeSpan& operator=(TimeSpan&& other) noexcept;

    public:
        /** Returns the total number of ticks (100-nanosecond units) in this TimeSpan. */
        [[nodiscard]] longcs getTicksProperty() const;

    public:
        /** Returns the total number of milliseconds, including fractional milliseconds. */
        [[nodiscard]] double getTotalMillisecondsProperty() const;

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
        /** Returns a TimeSpan that represents the specified number of seconds. */
        static TimeSpan FromSeconds(double value);

    public:
        /**
         * @brief Subtracts @p ts from the current TimeSpan and returns the result.
         * @param ts The TimeSpan to subtract.
         */
        [[nodiscard]] TimeSpan Subtract(const TimeSpan &ts) const;

    public:
        /** Returns a TimeSpan that represents the specified number of ticks. */
        static TimeSpan FromTicks(longcs value);

    public:
        /** Subtracts @p t2 from this TimeSpan. */
        TimeSpan operator-(const TimeSpan &t2) const;

    public:
        /** Adds @p t2 to this TimeSpan. */
        TimeSpan operator+(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan equals @p t2. */
        bool operator==(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan is not equal to @p t2. */
        bool operator!=(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan is greater than @p t2. */
        bool operator>(const TimeSpan &t2) const;

    public:
        /** Returns true if this TimeSpan is greater than or equal to @p t2. */
        bool operator>=(const TimeSpan &t2) const;
    };
} // System
