// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#include "System/Math.hpp"

#include <algorithm>
#include <cmath>

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
}
