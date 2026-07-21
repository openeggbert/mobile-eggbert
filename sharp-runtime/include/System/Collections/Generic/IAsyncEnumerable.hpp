// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <memory>
#include "System/Collections/Generic/IAsyncEnumerator.hpp"
#include "System/Threading/CancellationToken.hpp"

namespace System::Collections::Generic {

/**
 * @brief Exposes an asynchronous enumerator that provides asynchronous iteration over a sequence of values.
 *
 * C++ counterpart of .NET System.Collections.Generic.IAsyncEnumerable<T>.
 * In C++, async behaviour is approximated synchronously — GetAsyncEnumerator() returns
 * a shared_ptr to IAsyncEnumerator<T> which iterates synchronously.
 *
 * @tparam T The type of objects to enumerate.
 */
template<typename T>
class IAsyncEnumerable {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IAsyncEnumerable() = default;

    /**
     * @brief Returns an asynchronous enumerator that iterates through the collection.
     *
     * C++ counterpart of .NET IAsyncEnumerable<T>.GetAsyncEnumerator(CancellationToken).
     * @param cancellationToken A CancellationToken that may be used to cancel the iteration.
     * @return A shared_ptr to an IAsyncEnumerator<T> for the collection.
     */
    [[nodiscard]] virtual std::shared_ptr<IAsyncEnumerator<T>> GetAsyncEnumerator(
        System::Threading::CancellationToken cancellationToken = {}) = 0;
};

} // namespace System::Collections::Generic
