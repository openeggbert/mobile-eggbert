// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <bit>
#include <cstdint>

#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Collections/Generic/IComparer.hpp"
#include "System/Half.hpp"

namespace System::Numerics {

    namespace Detail {
        /**
         * IEEE 754 floating-point numbers use sign-magnitude, not two's complement, so when both
         * operands are negative (as signed integers of the same width as the float), the ordinary
         * integer order is the reverse of totalOrder; ties fall out of the equality check either way.
         */
        template<typename TInt>
        [[nodiscard]] inline SharpRuntime::intcs CompareTotalOrderInteger(TInt x, TInt y) noexcept
        {
            if (x < 0 && y < 0)
                return y < x ? -1 : (y > x ? 1 : 0);
            return x < y ? -1 : (x > y ? 1 : 0);
        }
    }

    /**
     * @brief Compares floating-point numbers using the IEEE 754 totalOrder predicate.
     *
     * C++ counterpart of .NET System.Numerics.TotalOrderIeee754Comparer<T>. .NET constrains T to
     * IFloatingPointIeee754<T> via generic math and dispatches on typeof(T) at the call site; this
     * port instead specializes directly for the IEEE 754 binary floating-point types this runtime
     * supports (float, double, System::Half).
     */
    template<typename T>
    struct TotalOrderIeee754Comparer;

    /** @brief float specialization: totalOrder over the IEEE 754 binary32 bit pattern. */
    template<>
    struct TotalOrderIeee754Comparer<float> : System::Collections::Generic::IComparer<float>
    {
        [[nodiscard]] SharpRuntime::intcs Compare(const float& x, const float& y) const override
        {
            return Detail::CompareTotalOrderInteger(std::bit_cast<int32_t>(x), std::bit_cast<int32_t>(y));
        }
    };

    /** @brief double specialization: totalOrder over the IEEE 754 binary64 bit pattern. */
    template<>
    struct TotalOrderIeee754Comparer<double> : System::Collections::Generic::IComparer<double>
    {
        [[nodiscard]] SharpRuntime::intcs Compare(const double& x, const double& y) const override
        {
            return Detail::CompareTotalOrderInteger(std::bit_cast<int64_t>(x), std::bit_cast<int64_t>(y));
        }
    };

    /** @brief System::Half specialization: totalOrder over the IEEE 754 binary16 bit pattern. */
    template<>
    struct TotalOrderIeee754Comparer<System::Half> : System::Collections::Generic::IComparer<System::Half>
    {
        [[nodiscard]] SharpRuntime::intcs Compare(const System::Half& x, const System::Half& y) const override
        {
            return Detail::CompareTotalOrderInteger(static_cast<int16_t>(x.bits), static_cast<int16_t>(y.bits));
        }
    };

} // namespace System::Numerics
