// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <string>
#include "System/DateTime.hpp"
#include "System/Decimal.hpp"
#include "System/TypeCode.hpp"

namespace System {

    /**
     * @brief Defines methods that convert the value of the implementing type
     * to a common language runtime type.
     *
     * C++ counterpart of .NET System.IConvertible. Real .NET's conversion methods each take an
     * `IFormatProvider` parameter for culture-aware conversions (e.g.
     * `bool ToBoolean(IFormatProvider provider)`); this port omits it, a deliberate
     * simplification since implementers so far (DBNull) always throw regardless of provider.
     * Reintroduce the parameter if a future culture-sensitive implementer needs it.
     */
    class IConvertible {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IConvertible() = default;
        /** @brief Returns the TypeCode for this instance. */
        [[nodiscard]] virtual TypeCode GetTypeCode() const = 0;
        /** @brief Converts this value to a Boolean. */
        [[nodiscard]] virtual bool ToBoolean() const = 0;
        /** @brief Converts this value to a Unicode character. */
        [[nodiscard]] virtual char ToChar() const = 0;
        /** @brief Converts this value to an 8-bit signed integer. */
        [[nodiscard]] virtual int8_t ToSByte() const = 0;
        /** @brief Converts this value to an 8-bit unsigned integer. */
        [[nodiscard]] virtual uint8_t ToByte() const = 0;
        /** @brief Converts this value to a 16-bit signed integer. */
        [[nodiscard]] virtual int16_t ToInt16() const = 0;
        /** @brief Converts this value to a 16-bit unsigned integer. */
        [[nodiscard]] virtual uint16_t ToUInt16() const = 0;
        /** @brief Converts this value to a 32-bit signed integer. */
        [[nodiscard]] virtual int32_t ToInt32() const = 0;
        /** @brief Converts this value to a 32-bit unsigned integer. */
        [[nodiscard]] virtual uint32_t ToUInt32() const = 0;
        /** @brief Converts this value to a 64-bit signed integer. */
        [[nodiscard]] virtual int64_t ToInt64() const = 0;
        /** @brief Converts this value to a 64-bit unsigned integer. */
        [[nodiscard]] virtual uint64_t ToUInt64() const = 0;
        /** @brief Converts this value to a single-precision floating-point number. */
        [[nodiscard]] virtual float ToSingle() const = 0;
        /** @brief Converts this value to a double-precision floating-point number. */
        [[nodiscard]] virtual double ToDouble() const = 0;
        /** @brief Converts this value to a Decimal number. */
        [[nodiscard]] virtual Decimal ToDecimal() const = 0;
        /** @brief Converts this value to a DateTime. */
        [[nodiscard]] virtual DateTime ToDateTime() const = 0;
        /** @brief Converts this value to its string representation. */
        [[nodiscard]] virtual std::string ToString() const = 0;
    };

} // namespace System
