// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <functional>

namespace System {

    /**
     * @brief Represents the method that defines a set of criteria and
     * determines whether the specified object meets those criteria.
     *
     * C++ counterpart of the .NET System.Predicate<T> delegate.
     *
     * @tparam T The type of the object to compare.
     */
    template<typename T>
    using Predicate = std::function<bool(T)>;

} // namespace System
