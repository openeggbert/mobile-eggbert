// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <functional>
#include "System/Span.hpp"

namespace System {

    /**
     * @brief Encapsulates a method that has no parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action.
     */
    using Action = std::function<void()>;

    /**
     * @brief Encapsulates a method that has one parameter and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T>.
     * @tparam T The type of the parameter.
     */
    template<typename T>
    using ActionT = std::function<void(T)>;

    /**
     * @brief Encapsulates a method that has two parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1, T2>.
     */
    template<typename T1, typename T2>
    using ActionT2 = std::function<void(T1, T2)>;

    /**
     * @brief Encapsulates a method that has three parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1, T2, T3>.
     */
    template<typename T1, typename T2, typename T3>
    using ActionT3 = std::function<void(T1, T2, T3)>;

    /**
     * @brief Encapsulates a method that has four parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1, T2, T3, T4>.
     */
    template<typename T1, typename T2, typename T3, typename T4>
    using ActionT4 = std::function<void(T1, T2, T3, T4)>;

    /**
     * @brief Encapsulates a method that has five parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1, T2, T3, T4, T5>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    using ActionT5 = std::function<void(T1, T2, T3, T4, T5)>;

    /**
     * @brief Encapsulates a method that has six parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1, T2, T3, T4, T5, T6>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    using ActionT6 = std::function<void(T1, T2, T3, T4, T5, T6)>;

    /**
     * @brief Encapsulates a method that has seven parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1, T2, T3, T4, T5, T6, T7>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7>
    using ActionT7 = std::function<void(T1, T2, T3, T4, T5, T6, T7)>;

    /**
     * @brief Encapsulates a method that has eight parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1, T2, T3, T4, T5, T6, T7, T8>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8>
    using ActionT8 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8)>;

    /**
     * @brief Encapsulates a method that has nine parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1, T2, T3, T4, T5, T6, T7, T8, T9>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9>
    using ActionT9 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9)>;

    /**
     * @brief Encapsulates a method that has ten parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1..T10>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10>
    using ActionT10 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10)>;

    /**
     * @brief Encapsulates a method that has eleven parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1..T11>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11>
    using ActionT11 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11)>;

    /**
     * @brief Encapsulates a method that has twelve parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1..T12>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
    using ActionT12 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)>;

    /**
     * @brief Encapsulates a method that has thirteen parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1..T13>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename T13>
    using ActionT13 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12,
                                         T13)>;

    /**
     * @brief Encapsulates a method that has fourteen parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1..T14>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename T13, typename T14>
    using ActionT14 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12,
                                         T13, T14)>;

    /**
     * @brief Encapsulates a method that has fifteen parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1..T15>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename T13, typename T14, typename T15>
    using ActionT15 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12,
                                         T13, T14, T15)>;

    /**
     * @brief Encapsulates a method that has sixteen parameters and does not return a value.
     *
     * C++ counterpart of .NET System.Action<T1..T16>.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename T13, typename T14, typename T15, typename T16>
    using ActionT16 = std::function<void(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12,
                                         T13, T14, T15, T16)>;

    // -------------------------------------------------------------------------

    /**
     * @brief Represents a method that compares two objects of the same type.
     *
     * C++ counterpart of .NET System.Comparison<T>. Returns negative if x < y,
     * zero if x == y, positive if x > y.
     * @tparam T The type of the objects to compare.
     */
    template<typename T>
    using Comparison = std::function<int(T, T)>;

    /**
     * @brief Encapsulates a method that converts an object from one type to another.
     *
     * C++ counterpart of .NET System.Converter<TInput, TOutput>.
     * @tparam TInput  The type to convert from.
     * @tparam TOutput The type to convert to.
     */
    template<typename TInput, typename TOutput>
    using Converter = std::function<TOutput(TInput)>;

} // namespace System

namespace System::Buffers {

    /**
     * @brief Encapsulates a method that receives a Span<T> and an additional argument.
     *
     * C++ counterpart of .NET System.Buffers.SpanAction<T, TArg>.
     * @tparam T    The element type of the span.
     * @tparam TArg The type of the additional argument.
     */
    template<typename T, typename TArg>
    using SpanAction = std::function<void(System::Span<T>, TArg)>;

    /**
     * @brief Encapsulates a method that receives a ReadOnlySpan<T> and an additional argument.
     *
     * C++ counterpart of .NET System.Buffers.ReadOnlySpanAction<T, TArg>.
     * @tparam T    The element type of the span.
     * @tparam TArg The type of the additional argument.
     */
    template<typename T, typename TArg>
    using ReadOnlySpanAction = std::function<void(System::ReadOnlySpan<T>, TArg)>;

} // namespace System::Buffers
