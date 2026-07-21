// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <complex>
#include <cmath>
#include <string>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System::Numerics {

    using SharpRuntime::Single;

    /**
     * @brief Represents a complex number.
     *
     * Wraps std::complex<double>. Partial C++ counterpart of .NET System.Numerics.Complex.
     *
     * @note Status: Implemented
     */
    struct Complex {
        std::complex<double> value;

        /** Constructs the complex number 0 + 0i. */
        Complex() = default;
        /** Constructs the complex number @p real + @p imaginary * i. */
        Complex(double real, double imaginary) : value(real, imaginary) {}
        /** Constructs from a std::complex<double>. */
        explicit Complex(std::complex<double> v) : value(v) {}

        /** @return The real part of this complex number. */
        [[nodiscard]] double getRealProperty()      const { return value.real(); }
        /** @return The imaginary part of this complex number. */
        [[nodiscard]] double getImaginaryProperty() const { return value.imag(); }
        /** @return The magnitude (absolute value) ||z||. */
        [[nodiscard]] double getMagnitudeProperty() const { return std::abs(value); }
        /** @return The phase angle (argument) of this complex number in radians. */
        [[nodiscard]] double getPhaseProperty()     const { return std::arg(value); }

        /** Complex addition. */
        Complex operator+(const Complex& o) const { return Complex(value + o.value); }
        /** Complex subtraction. */
        Complex operator-(const Complex& o) const { return Complex(value - o.value); }
        /** Complex multiplication. */
        Complex operator*(const Complex& o) const { return Complex(value * o.value); }
        /** Complex division. */
        Complex operator/(const Complex& o) const { return Complex(value / o.value); }
        /** Unary negation. */
        Complex operator-()                 const { return Complex(-value); }
        /** Equality comparison. */
        bool operator==(const Complex& o)   const { return value == o.value; }
        /** Inequality comparison. */
        bool operator!=(const Complex& o)   const { return value != o.value; }

        /** @return The complex conjugate z* = real - imaginary*i. */
        [[nodiscard]] Complex Conjugate() const { return Complex(std::conj(value)); }

        /** @return The absolute value of @p c as a real Complex (imaginary part = 0). */
        static Complex Abs(const Complex& c)                          { return Complex(std::abs(c.value), 0); }
        /** @return The absolute value of @p c as a plain double. */
        static double  AbsD(const Complex& c)                         { return std::abs(c.value); }
        /** @return The complex square root of @p c. */
        static Complex Sqrt(const Complex& c)                         { return Complex(std::sqrt(c.value)); }
        /** @return e raised to the power @p c. */
        static Complex Exp(const Complex& c)                          { return Complex(std::exp(c.value)); }
        /** @return Natural logarithm of @p c. */
        static Complex Log(const Complex& c)                          { return Complex(std::log(c.value)); }
        /** @return @p b raised to the complex power @p e. */
        static Complex Pow(const Complex& b, const Complex& e)        { return Complex(std::pow(b.value, e.value)); }
        /** @return Complex sine of @p c. */
        static Complex Sin(const Complex& c)                          { return Complex(std::sin(c.value)); }
        /** @return Complex cosine of @p c. */
        static Complex Cos(const Complex& c)                          { return Complex(std::cos(c.value)); }
        /** @return Complex tangent of @p c. */
        static Complex Tan(const Complex& c)                          { return Complex(std::tan(c.value)); }
        /** @return Complex arcsine of @p c. */
        static Complex Asin(const Complex& c)                         { return Complex(std::asin(c.value)); }
        /** @return Complex arccosine of @p c. */
        static Complex Acos(const Complex& c)                         { return Complex(std::acos(c.value)); }
        /** @return Complex arctangent of @p c. */
        static Complex Atan(const Complex& c)                         { return Complex(std::atan(c.value)); }
        /** @return Complex hyperbolic sine of @p c. */
        static Complex Sinh(const Complex& c)                         { return Complex(std::sinh(c.value)); }
        /** @return Complex hyperbolic cosine of @p c. */
        static Complex Cosh(const Complex& c)                         { return Complex(std::cosh(c.value)); }
        /** @return Complex hyperbolic tangent of @p c. */
        static Complex Tanh(const Complex& c)                         { return Complex(std::tanh(c.value)); }
        /** @return Natural logarithm of @p c to the specified @p baseValue. */
        static Complex Log(const Complex& c, double baseValue)        { return Complex(std::log(c.value) / std::log(baseValue)); }
        /** @return The reciprocal 1/c. Returns Zero if c is zero (matching .NET's explicit zero-value special case). */
        static Complex Reciprocal(const Complex& c) {
            if (c.value.real() == 0.0 && c.value.imag() == 0.0) return Zero;
            return Complex(1.0 / c.value);
        }
        /** @return Complex number from polar coordinates (magnitude, phase in radians). */
        static Complex FromPolarCoordinates(double magnitude, double phase) {
            return Complex(std::polar(magnitude, phase));
        }

        static const Complex Zero;         ///< The complex number 0 + 0i.
        static const Complex One;          ///< The complex number 1 + 0i.
        static const Complex ImaginaryOne; ///< The complex number 0 + 1i.

        /** @return A string of the form "&lt;real; imaginary&gt;". */
        [[nodiscard]] std::string ToString() const {
            auto r = value.real(), i = value.imag();
            return std::string("<") + std::to_string(r) + "; " + std::to_string(i) + ">";
        }
    };

    inline const Complex Complex::Zero         { 0.0, 0.0 };
    inline const Complex Complex::One          { 1.0, 0.0 };
    inline const Complex Complex::ImaginaryOne { 0.0, 1.0 };

} // namespace System::Numerics
