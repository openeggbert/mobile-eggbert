// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include "System/Tuple.hpp"

namespace System {

/**
 * @brief Provides extension-like free functions for Tuple instances to interop with
 *        std::tuple (C++ equivalent of ValueTuple) and to support deconstruction.
 *
 * C++ counterpart of .NET System.TupleExtensions.
 */
struct TupleExtensions {
    TupleExtensions() = delete;

    // -----------------------------------------------------------------------
    // ToValueTuple (TupleN → std::tuple)
    // -----------------------------------------------------------------------

    /** @brief Converts a Tuple1 to an equivalent std::tuple. */
    template<typename T1>
    [[nodiscard]] static std::tuple<T1> ToValueTuple(const Tuple1<T1>& t) {
        return t.ToStdTuple();
    }

    /** @brief Converts a Tuple2 to an equivalent std::tuple (ValueTuple equivalent). */
    template<typename T1, typename T2>
    [[nodiscard]] static std::tuple<T1, T2> ToValueTuple(const Tuple2<T1, T2>& t) {
        return t.ToStdTuple();
    }

    /** @brief Converts a Tuple3 to an equivalent std::tuple. */
    template<typename T1, typename T2, typename T3>
    [[nodiscard]] static std::tuple<T1, T2, T3> ToValueTuple(const Tuple3<T1, T2, T3>& t) {
        return t.ToStdTuple();
    }

    /** @brief Converts a Tuple4 to an equivalent std::tuple. */
    template<typename T1, typename T2, typename T3, typename T4>
    [[nodiscard]] static std::tuple<T1, T2, T3, T4> ToValueTuple(const Tuple4<T1, T2, T3, T4>& t) {
        return t.ToStdTuple();
    }

    /** @brief Converts a Tuple5 to an equivalent std::tuple. */
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    [[nodiscard]] static std::tuple<T1,T2,T3,T4,T5>
    ToValueTuple(const Tuple5<T1,T2,T3,T4,T5>& t) {
        return t.ToStdTuple();
    }

    /** @brief Converts a Tuple6 to an equivalent std::tuple. */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    [[nodiscard]] static std::tuple<T1,T2,T3,T4,T5,T6>
    ToValueTuple(const Tuple6<T1,T2,T3,T4,T5,T6>& t) {
        return t.ToStdTuple();
    }

    /** @brief Converts a Tuple7 to an equivalent std::tuple. */
    template<typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
    [[nodiscard]] static std::tuple<T1,T2,T3,T4,T5,T6,T7>
    ToValueTuple(const Tuple7<T1,T2,T3,T4,T5,T6,T7>& t) {
        return t.ToStdTuple();
    }

    // -----------------------------------------------------------------------
    // ToTuple (std::tuple → TupleN)
    // -----------------------------------------------------------------------

    /** @brief Converts a 1-element std::tuple to an equivalent Tuple1. */
    template<typename T1>
    [[nodiscard]] static Tuple1<T1> ToTuple(const std::tuple<T1>& t) {
        return Tuple1<T1>(std::get<0>(t));
    }

    /** @brief Converts a std::tuple to an equivalent Tuple2. */
    template<typename T1, typename T2>
    [[nodiscard]] static Tuple2<T1, T2> ToTuple(const std::tuple<T1, T2>& t) {
        return {std::get<0>(t), std::get<1>(t)};
    }

    /** @brief Converts a std::tuple to an equivalent Tuple3. */
    template<typename T1, typename T2, typename T3>
    [[nodiscard]] static Tuple3<T1, T2, T3> ToTuple(const std::tuple<T1, T2, T3>& t) {
        return {std::get<0>(t), std::get<1>(t), std::get<2>(t)};
    }

    /** @brief Converts a std::tuple to an equivalent Tuple4. */
    template<typename T1, typename T2, typename T3, typename T4>
    [[nodiscard]] static Tuple4<T1, T2, T3, T4> ToTuple(const std::tuple<T1, T2, T3, T4>& t) {
        return {std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t)};
    }

    /** @brief Converts a std::tuple to an equivalent Tuple5. */
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    [[nodiscard]] static Tuple5<T1,T2,T3,T4,T5>
    ToTuple(const std::tuple<T1,T2,T3,T4,T5>& t) {
        return {std::get<0>(t), std::get<1>(t), std::get<2>(t),
                std::get<3>(t), std::get<4>(t)};
    }

    /** @brief Converts a std::tuple to an equivalent Tuple6. */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    [[nodiscard]] static Tuple6<T1,T2,T3,T4,T5,T6>
    ToTuple(const std::tuple<T1,T2,T3,T4,T5,T6>& t) {
        return {std::get<0>(t), std::get<1>(t), std::get<2>(t),
                std::get<3>(t), std::get<4>(t), std::get<5>(t)};
    }

    /** @brief Converts a std::tuple to an equivalent Tuple7. */
    template<typename T1, typename T2, typename T3, typename T4,
             typename T5, typename T6, typename T7>
    [[nodiscard]] static Tuple7<T1,T2,T3,T4,T5,T6,T7>
    ToTuple(const std::tuple<T1,T2,T3,T4,T5,T6,T7>& t) {
        return {std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t),
                std::get<4>(t), std::get<5>(t), std::get<6>(t)};
    }
};

} // namespace System
