// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include "System/Span.hpp"

namespace System::Buffers {

    /**
     * @brief Encapsulates a method that receives a span of objects of type T
     * and a state object of type TArg.
     *
     * C++ counterpart of .NET System.SpanAction&lt;T, TArg&gt; delegate.
     * Represented as a std::function that takes a Span&lt;T&gt; and a TArg by value.
     *
     * @tparam T    Element type of the span.
     * @tparam TArg Type of the state argument.
     */
    template<typename T, typename TArg>
    using SpanAction = std::function<void(System::Span<T>, TArg)>;

} // namespace System::Buffers
