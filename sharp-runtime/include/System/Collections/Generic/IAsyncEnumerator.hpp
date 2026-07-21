// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

namespace System::Collections::Generic {

/**
 * @brief Supports a simple asynchronous iteration over a generic collection.
 *
 * C++ counterpart of .NET System.Collections.Generic.IAsyncEnumerator<T>.
 * In C++, async behaviour is approximated synchronously — MoveNextAsync() returns bool
 * directly (no future/coroutine overhead) and DisposeAsync() is a no-op by default.
 *
 * @tparam T The type of objects to enumerate.
 */
template<typename T>
class IAsyncEnumerator {
public:
    /** @brief Virtual destructor for safe polymorphic destruction. */
    virtual ~IAsyncEnumerator() = default;

    /**
     * @brief Advances the enumerator to the next element of the collection.
     *
     * C++ counterpart of .NET IAsyncEnumerator<T>.MoveNextAsync().
     * @return true if another element is available; false if the enumerator has passed the end.
     */
    virtual bool MoveNextAsync() = 0;

    /**
     * @brief Gets the element at the current position of the enumerator.
     *
     * C++ counterpart of .NET IAsyncEnumerator<T>.Current.
     * @return A const reference to the current element.
     */
    [[nodiscard]] virtual const T& getCurrentProperty() const = 0;

    /**
     * @brief Performs application-defined tasks associated with freeing resources asynchronously.
     *
     * C++ counterpart of .NET IAsyncDisposable.DisposeAsync().
     * Default implementation is a no-op.
     */
    virtual void DisposeAsync() {}
};

} // namespace System::Collections::Generic
