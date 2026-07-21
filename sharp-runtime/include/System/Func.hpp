// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <functional>

namespace System {

    /**
     * @brief Encapsulates a method that has no parameters and returns a value
     * of type R.
     *
     * C++ counterpart of the .NET System.Func<TResult> delegate.
     */
    template<typename R>
    using Func = std::function<R()>;

    /**
     * @brief Encapsulates a method that has one parameter and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T, TResult> delegate.
     */
    template<typename T, typename R>
    using FuncT = std::function<R(T)>;

    /**
     * @brief Encapsulates a method that has two parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1, T2, TResult> delegate.
     */
    template<typename T1, typename T2, typename R>
    using FuncT2 = std::function<R(T1, T2)>;

    /**
     * @brief Encapsulates a method that has three parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1, T2, T3, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename R>
    using FuncT3 = std::function<R(T1, T2, T3)>;

    /**
     * @brief Encapsulates a method that has four parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1, T2, T3, T4, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename R>
    using FuncT4 = std::function<R(T1, T2, T3, T4)>;

    /**
     * @brief Encapsulates a method that has five parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1, T2, T3, T4, T5, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename R>
    using FuncT5 = std::function<R(T1, T2, T3, T4, T5)>;

    /**
     * @brief Encapsulates a method that has six parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T6, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename R>
    using FuncT6 = std::function<R(T1, T2, T3, T4, T5, T6)>;

    /**
     * @brief Encapsulates a method that has seven parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T7, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename R>
    using FuncT7 = std::function<R(T1, T2, T3, T4, T5, T6, T7)>;

    /**
     * @brief Encapsulates a method that has eight parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T8, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename R>
    using FuncT8 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8)>;

    /**
     * @brief Encapsulates a method that has nine parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T9, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename R>
    using FuncT9 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8, T9)>;

    /**
     * @brief Encapsulates a method that has ten parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T10, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename R>
    using FuncT10 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10)>;

    /**
     * @brief Encapsulates a method that has eleven parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T11, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename R>
    using FuncT11 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11)>;

    /**
     * @brief Encapsulates a method that has twelve parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T12, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename R>
    using FuncT12 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)>;

    /**
     * @brief Encapsulates a method that has thirteen parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T13, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename T13, typename R>
    using FuncT13 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13)>;

    /**
     * @brief Encapsulates a method that has fourteen parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T14, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename T13, typename T14, typename R>
    using FuncT14 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,
                                     T14)>;

    /**
     * @brief Encapsulates a method that has fifteen parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T15, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename T13, typename T14, typename T15, typename R>
    using FuncT15 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,
                                     T14, T15)>;

    /**
     * @brief Encapsulates a method that has sixteen parameters and returns a value.
     *
     * C++ counterpart of the .NET System.Func<T1..T16, TResult> delegate.
     */
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
             typename T7, typename T8, typename T9, typename T10, typename T11, typename T12,
             typename T13, typename T14, typename T15, typename T16, typename R>
    using FuncT16 = std::function<R(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13,
                                     T14, T15, T16)>;

} // namespace System
