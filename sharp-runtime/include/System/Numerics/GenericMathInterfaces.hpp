// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
//
// Stub interfaces from System.Numerics generic math (.NET 7+).
// C++ templates naturally cover the same use cases; these stubs exist for API name compatibility.
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Numerics {

    using SharpRuntime::intcs;

    /** Base interface for all numeric types, mirroring .NET INumberBase<TSelf>. */
    template<typename TSelf>
    struct INumberBase {
        static constexpr intcs Radix = 2; ///< Radix (base) of the numeric representation.
        virtual ~INumberBase() = default;
    };

    /** Interface for general numeric types, mirroring .NET INumber<TSelf>. */
    template<typename TSelf>
    struct INumber : INumberBase<TSelf> {};

    /** Interface for signed numeric types, mirroring .NET ISignedNumber<TSelf>. */
    template<typename TSelf>
    struct ISignedNumber : INumber<TSelf> {};

    /** Interface for unsigned numeric types, mirroring .NET IUnsignedNumber<TSelf>. */
    template<typename TSelf>
    struct IUnsignedNumber : INumber<TSelf> {};

    /** Interface for floating-point numeric types, mirroring .NET IFloatingPoint<TSelf>. */
    template<typename TSelf>
    struct IFloatingPoint : INumber<TSelf> {};

    /** Interface for IEEE 754 floating-point types, mirroring .NET IFloatingPointIeee754<TSelf>. */
    template<typename TSelf>
    struct IFloatingPointIeee754 : IFloatingPoint<TSelf> {};

    /** Interface for binary integer types, mirroring .NET IBinaryInteger<TSelf>. */
    template<typename TSelf>
    struct IBinaryInteger : INumber<TSelf> {};

    /** Interface for binary numeric types, mirroring .NET IBinaryNumber<TSelf>. */
    template<typename TSelf>
    struct IBinaryNumber : INumberBase<TSelf> {};

    // Arithmetic operator interfaces (C++ satisfies these implicitly via operators)

    /** Stub interface for addition operators, mirroring .NET IAdditionOperators<TLeft,TRight,TResult>. */
    template<typename TLeft, typename TRight, typename TResult>
    struct IAdditionOperators {};

    /** Stub interface for subtraction operators, mirroring .NET ISubtractionOperators<TLeft,TRight,TResult>. */
    template<typename TLeft, typename TRight, typename TResult>
    struct ISubtractionOperators {};

    /** Stub interface for multiplication operators, mirroring .NET IMultiplyOperators<TLeft,TRight,TResult>. */
    template<typename TLeft, typename TRight, typename TResult>
    struct IMultiplyOperators {};

    /** Stub interface for division operators, mirroring .NET IDivisionOperators<TLeft,TRight,TResult>. */
    template<typename TLeft, typename TRight, typename TResult>
    struct IDivisionOperators {};

    /** Stub interface for modulus operators, mirroring .NET IModulusOperators<TLeft,TRight,TResult>. */
    template<typename TLeft, typename TRight, typename TResult>
    struct IModulusOperators {};

    /** Stub interface for unary negation, mirroring .NET IUnaryNegationOperators<TSelf>. */
    template<typename TSelf>
    struct IUnaryNegationOperators {};

    /** Stub interface for unary plus, mirroring .NET IUnaryPlusOperators<TSelf>. */
    template<typename TSelf>
    struct IUnaryPlusOperators {};

    /** Stub interface for comparison operators, mirroring .NET IComparisonOperators<TLeft,TRight>. */
    template<typename TLeft, typename TRight>
    struct IComparisonOperators {};

    /** Stub interface for bitwise operators, mirroring .NET IBitwiseOperators<TLeft,TRight,TResult>. */
    template<typename TLeft, typename TRight, typename TResult>
    struct IBitwiseOperators {};

    /** Stub interface for shift operators, mirroring .NET IShiftOperators<TLeft,TRight,TResult>. */
    template<typename TLeft, typename TRight, typename TResult>
    struct IShiftOperators {};

    /** Stub interface for increment operators, mirroring .NET IIncrementOperators<TSelf>. */
    template<typename TSelf>
    struct IIncrementOperators {};

    /** Stub interface for decrement operators, mirroring .NET IDecrementOperators<TSelf>. */
    template<typename TSelf>
    struct IDecrementOperators {};

    /** Stub interface exposing MinValue/MaxValue constants, mirroring .NET IMinMaxValue<TSelf>. */
    template<typename TSelf>
    struct IMinMaxValue {
        static TSelf MinValue(); ///< Returns the minimum representable value of TSelf.
        static TSelf MaxValue(); ///< Returns the maximum representable value of TSelf.
    };

    /** Stub interface exposing the additive identity, mirroring .NET IAdditiveIdentity<TSelf>. */
    template<typename TSelf>
    struct IAdditiveIdentity { static TSelf AdditiveIdentity(); ///< Returns the additive identity (zero) of TSelf.
    };

    /** Stub interface exposing the multiplicative identity, mirroring .NET IMultiplicativeIdentity<TSelf>. */
    template<typename TSelf>
    struct IMultiplicativeIdentity { static TSelf MultiplicativeIdentity(); ///< Returns the multiplicative identity (one) of TSelf.
    };

    /** Stub interface for equality operators, mirroring .NET IEqualityOperators<TSelf,TOther,TResult>. */
    template<typename TSelf, typename TOther, typename TResult>
    struct IEqualityOperators {};

    /** Stub interface for trigonometric functions, mirroring .NET ITrigonometricFunctions<TSelf>. */
    template<typename TSelf>
    struct ITrigonometricFunctions {
        static TSelf Acos(TSelf x);   ///< Returns the arc cosine of x, in radians.
        static TSelf AcosPi(TSelf x); ///< Returns the arc cosine of x, divided by pi.
        static TSelf Asin(TSelf x);   ///< Returns the arc sine of x, in radians.
        static TSelf AsinPi(TSelf x); ///< Returns the arc sine of x, divided by pi.
        static TSelf Atan(TSelf x);   ///< Returns the arc tangent of x, in radians.
        static TSelf AtanPi(TSelf x); ///< Returns the arc tangent of x, divided by pi.
        static TSelf Cos(TSelf x);    ///< Returns the cosine of x (radians).
        static TSelf CosPi(TSelf x);  ///< Returns the cosine of x*pi.
        static TSelf Sin(TSelf x);    ///< Returns the sine of x (radians).
        static TSelf SinPi(TSelf x);  ///< Returns the sine of x*pi.
        static TSelf Tan(TSelf x);    ///< Returns the tangent of x (radians).
        static TSelf TanPi(TSelf x);  ///< Returns the tangent of x*pi.
    };

    /** Stub interface for hyperbolic functions, mirroring .NET IHyperbolicFunctions<TSelf>. */
    template<typename TSelf>
    struct IHyperbolicFunctions {
        static TSelf Acosh(TSelf x); ///< Returns the inverse hyperbolic cosine of x.
        static TSelf Asinh(TSelf x); ///< Returns the inverse hyperbolic sine of x.
        static TSelf Atanh(TSelf x); ///< Returns the inverse hyperbolic tangent of x.
        static TSelf Cosh(TSelf x);  ///< Returns the hyperbolic cosine of x.
        static TSelf Sinh(TSelf x);  ///< Returns the hyperbolic sine of x.
        static TSelf Tanh(TSelf x);  ///< Returns the hyperbolic tangent of x.
    };

    /** Stub interface for logarithmic functions, mirroring .NET ILogarithmicFunctions<TSelf>. */
    template<typename TSelf>
    struct ILogarithmicFunctions {
        static TSelf Log(TSelf x);                  ///< Returns the natural (base e) logarithm of x.
        static TSelf Log(TSelf x, TSelf newBase);    ///< Returns the logarithm of x in the given base.
        static TSelf Log2(TSelf x);                  ///< Returns the base-2 logarithm of x.
        static TSelf Log10(TSelf x);                 ///< Returns the base-10 logarithm of x.
    };

    /** Stub interface for exponential functions, mirroring .NET IExponentialFunctions<TSelf>. */
    template<typename TSelf>
    struct IExponentialFunctions {
        static TSelf Exp(TSelf x);   ///< Returns e raised to the power x.
        static TSelf Exp2(TSelf x);  ///< Returns 2 raised to the power x.
        static TSelf Exp10(TSelf x); ///< Returns 10 raised to the power x.
    };

    /** Stub interface for power functions, mirroring .NET IPowerFunctions<TSelf>. */
    template<typename TSelf>
    struct IPowerFunctions {
        static TSelf Pow(TSelf x, TSelf y); ///< Returns x raised to the power y.
    };

    /** Stub interface for root functions, mirroring .NET IRootFunctions<TSelf>. */
    template<typename TSelf>
    struct IRootFunctions {
        static TSelf Cbrt(TSelf x);            ///< Returns the cube root of x.
        static TSelf Hypot(TSelf x, TSelf y);  ///< Returns sqrt(x*x + y*y), computed without undue overflow/underflow.
        static TSelf RootN(TSelf x, intcs n);  ///< Returns the n-th root of x.
        static TSelf Sqrt(TSelf x);            ///< Returns the square root of x.
    };

    /** Stub interface exposing E/Pi/Tau constants, mirroring .NET IFloatingPointConstants<TSelf>. */
    template<typename TSelf>
    struct IFloatingPointConstants {
        static TSelf E();   ///< Returns the mathematical constant e.
        static TSelf Pi();  ///< Returns the mathematical constant pi.
        static TSelf Tau(); ///< Returns the mathematical constant tau (2*pi).
    };

    /**
     * @brief Stub interface aggregating the IEEE 754 binary floating-point contract, mirroring
     * .NET IBinaryFloatingPointIeee754<TSelf>. Combines IFloatingPointIeee754, IFloatingPointConstants,
     * and the transcendental function families; C++ types satisfy the equivalent contract via
     * <cmath> free functions rather than interface conformance.
     */
    template<typename TSelf>
    struct IBinaryFloatingPointIeee754
        : IFloatingPointIeee754<TSelf>,
          IFloatingPointConstants<TSelf>,
          IExponentialFunctions<TSelf>,
          IHyperbolicFunctions<TSelf>,
          ILogarithmicFunctions<TSelf>,
          IPowerFunctions<TSelf>,
          IRootFunctions<TSelf>,
          ITrigonometricFunctions<TSelf>
    {};

} // namespace System::Numerics
