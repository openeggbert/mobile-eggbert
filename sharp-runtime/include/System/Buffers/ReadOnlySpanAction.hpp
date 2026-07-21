// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/ReadOnlySpan.hpp"

namespace System::Buffers {

    /**
     * @brief Encapsulates a method that receives a read-only span of objects of type T
     * and a state object of type TArg.
     *
     * C++ counterpart of .NET System.ReadOnlySpanAction&lt;T, TArg&gt; delegate.
     * Represented as a std::function that takes a ReadOnlySpan&lt;T&gt; and a TArg by value.
     *
     * @tparam T    Element type of the span.
     * @tparam TArg Type of the state argument.
     */
    template<typename T, typename TArg>
    using ReadOnlySpanAction = std::function<void(System::ReadOnlySpan<T>, TArg)>;

} // namespace System::Buffers
