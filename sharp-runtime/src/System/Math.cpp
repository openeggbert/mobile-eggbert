// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Math.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace System
{
    double Math::Sin(double value)
    {
        return std::sin(value);
    }

    double Math::Cos(double value)
    {
        return std::cos(value);
    }

    double Math::Tan(double value)
    {
        return std::tan(value);
    }

    double Math::Sqrt(double value)
    {
        return std::sqrt(value);
    }

    double Math::Abs(double value)
    {
        return std::fabs(value);
    }

    intcs Math::Abs(intcs value)
    {
        if (value == std::numeric_limits<intcs>::min())
            throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
        return value < 0 ? -value : value;
    }

    intcs Math::Min(intcs a, intcs b)
    {
        return std::min(a, b);
    }

    double Math::Min(double val1, double val2)
    {
        // Matches the IEEE 754:2019 `minimum` function (propagates NaN regardless of which
        // argument it's in, and treats +0 as greater than -0) -- std::min() does neither:
        // std::min(5.0, NaN) == 5.0 (should be NaN, since NaN only propagates when it's the
        // *first* argument to the typical `b < a ? b : a` implementation), and
        // std::min(+0.0, -0.0) == +0.0 (should be -0.0).
        if (val1 != val2)
        {
            if (!std::isnan(val1))
                return val1 < val2 ? val1 : val2;
            return val1;
        }
        return std::signbit(val1) ? val1 : val2;
    }

    intcs Math::Max(intcs a, intcs b)
    {
        return std::max(a, b);
    }

    double Math::Max(double val1, double val2)
    {
        // See Math::Min(double,double) above -- same IEEE 754:2019 `maximum` mismatch with
        // std::max().
        if (val1 != val2)
        {
            if (!std::isnan(val1))
                return val2 < val1 ? val1 : val2;
            return val1;
        }
        return std::signbit(val2) ? val1 : val2;
    }

    intcs Math::Clamp(intcs value, intcs min, intcs max)
    {
        if (min > max)
        {
            throw System::ArgumentException("min cannot be greater than max.");
        }
        if (value < min)
        {
            return min;
        }
        if (value > max)
        {
            return max;
        }
        return value;
    }

    double Math::Clamp(double value, double min, double max)
    {
        if (min > max)
        {
            throw System::ArgumentException("min cannot be greater than max.");
        }
        if (value < min)
        {
            return min;
        }
        if (value > max)
        {
            return max;
        }
        return value;
    }

    longcs Math::Abs(longcs value)
    {
        if (value == std::numeric_limits<longcs>::min())
            throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
        return value < 0 ? -value : value;
    }
    longcs Math::Min(longcs a, longcs b) { return a < b ? a : b; }
    longcs Math::Max(longcs a, longcs b) { return a > b ? a : b; }
    longcs Math::Clamp(longcs value, longcs min, longcs max)
    {
        if (min > max) throw System::ArgumentException("min cannot be greater than max.");
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    double Math::Round(double value)
    {
        // .NET's Math.Round(double) with no digits/mode defaults to MidpointRounding.ToEven
        // (banker's rounding), NOT round-half-away-from-zero - std::round(2.5) == 3.0 but
        // Math.Round(2.5) == 2.0 in .NET.
        return Round(value, MidpointRounding::ToEven);
    }

    double Math::Floor(double value)
    {
        return std::floor(value);
    }

    double Math::Ceiling(double value)
    {
        return std::ceil(value);
    }

    double Math::Pow(double x, double y)
    {
        return std::pow(x, y);
    }

    double Math::Log(double d)    { return std::log(d); }
    double Math::Log(double a, double newBase) { return std::log(a) / std::log(newBase); }
    double Math::Log2(double x)   { return std::log2(x); }
    double Math::Log10(double d)  { return std::log10(d); }
    double Math::Exp(double d)    { return std::exp(d); }

    double Math::Asin(double d)   { return std::asin(d); }
    double Math::Acos(double d)   { return std::acos(d); }
    double Math::Atan(double d)   { return std::atan(d); }
    double Math::Atan2(double y, double x) { return std::atan2(y, x); }

    double Math::Sinh(double value) { return std::sinh(value); }
    double Math::Cosh(double value) { return std::cosh(value); }
    double Math::Tanh(double value) { return std::tanh(value); }

    intcs Math::Sign(intcs value)
    {
        if (value < 0) return -1;
        if (value > 0) return  1;
        return 0;
    }

    intcs Math::Sign(double value)
    {
        if (value < 0.0) return -1;
        if (value > 0.0) return  1;
        if (value == 0.0) return 0;
        throw ArithmeticException("Function does not accept floating point Not-a-Number values.");
    }

    intcs Math::Sign(longcs value)
    {
        if (value < 0) return -1;
        if (value > 0) return  1;
        return 0;
    }

    intcs Math::Sign(float value)
    {
        if (value < 0.0f) return -1;
        if (value > 0.0f) return  1;
        if (value == 0.0f) return 0;
        throw ArithmeticException("Function does not accept floating point Not-a-Number values.");
    }

    double Math::Truncate(double d) { return std::trunc(d); }

    double Math::IEEERemainder(double x, double y)
    {
        return std::remainder(x, y);
    }

    intcs Math::DivRem(intcs a, intcs b, intcs& result)
    {
        // Verified against Math.cs's DivRem(int,int,out int): the C# source has no explicit
        // check for either failure mode -- it relies on the CLR's underlying idiv instruction
        // trapping on both divide-by-zero AND MinValue/-1 overflow, which the runtime then
        // translates into DivideByZeroException / OverflowException respectively. C++ has no
        // such automatic trap-to-exception translation: integer division is undefined behavior
        // on both inputs, and MinValue/-1 specifically raises a real SIGFPE hardware fault on
        // x86 (confirmed via a standalone repro -- this crashed the process, not merely UB in
        // the abstract), matching the same bug class already fixed in Int32::DivRem/
        // Int64::DivRem elsewhere in this codebase but missed here.
        if (b == 0) throw System::DivideByZeroException();
        if (a == std::numeric_limits<intcs>::min() && b == -1)
            throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
        intcs q = a / b;
        result  = a % b;
        return q;
    }

    longcs Math::DivRem(longcs a, longcs b, longcs& result)
    {
        // See Math::DivRem(intcs,intcs,intcs&) above -- same fix, same rationale.
        if (b == 0) throw System::DivideByZeroException();
        if (a == std::numeric_limits<longcs>::min() && b == -1)
            throw System::OverflowException("Negating the minimum value of a twos complement number is invalid.");
        longcs q = a / b;
        result   = a % b;
        return q;
    }

    double Math::MaxMagnitude(double x, double y)
    {
        double ax = std::fabs(x), ay = std::fabs(y);
        if (ax > ay || std::isnan(ax)) return x;
        if (ax == ay) return std::signbit(x) ? y : x;
        return y;
    }

    double Math::MinMagnitude(double x, double y)
    {
        double ax = std::fabs(x), ay = std::fabs(y);
        if (ax < ay || std::isnan(ax)) return x;
        if (ax == ay) return std::signbit(x) ? x : y;
        return y;
    }

    longcs Math::BigMul(intcs a, intcs b)
    {
        return static_cast<longcs>(a) * static_cast<longcs>(b);
    }

    double Math::ScaleB(double x, intcs n)
    {
        return std::scalbn(x, n);
    }

    double Math::Cbrt(double x)       { return std::cbrt(x); }
    double Math::Acosh(double d)      { return std::acosh(d); }
    double Math::Asinh(double d)      { return std::asinh(d); }
    double Math::Atanh(double d)      { return std::atanh(d); }

    double Math::Round(double value, intcs digits)
    {
        return Round(value, digits, MidpointRounding::ToEven);
    }

    double Math::CopySign(double x, double y)           { return std::copysign(x, y); }
    double Math::BitIncrement(double x)                 { return std::nextafter(x, std::numeric_limits<double>::infinity()); }
    double Math::BitDecrement(double x)                 { return std::nextafter(x, -std::numeric_limits<double>::infinity()); }
    double Math::FusedMultiplyAdd(double x, double y, double z) { return std::fma(x, y, z); }
}