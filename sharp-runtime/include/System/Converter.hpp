// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>

namespace System {

/**
 * @brief Represents a method that converts an object from one type to another type.
 *
 * C++ counterpart of .NET System.Converter&lt;TInput, TOutput&gt; delegate.
 * Maps to std::function&lt;TOutput(TInput)&gt;.
 *
 * @tparam TInput  The type of object that is to be converted.
 * @tparam TOutput The type the input object is to be converted to.
 */
template<typename TInput, typename TOutput>
using Converter = std::function<TOutput(TInput)>;

} // namespace System
