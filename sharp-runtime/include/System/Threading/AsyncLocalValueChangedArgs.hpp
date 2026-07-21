// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <utility>

namespace System::Threading {

    /**
     * @brief Provides data change information to AsyncLocal<T> instances registered for change notifications.
     *
     * C++ counterpart of .NET System.Threading.AsyncLocalValueChangedArgs<T>.
     */
    template<typename T>
    struct AsyncLocalValueChangedArgs {
        /** @brief Constructs the change-notification args. */
        AsyncLocalValueChangedArgs(T previousValue, T currentValue, bool threadContextChanged)
            : previousValue_(std::move(previousValue)),
              currentValue_(std::move(currentValue)),
              threadContextChanged_(threadContextChanged) {}

        /** @brief Returns the data's previous value. */
        [[nodiscard]] const T& getPreviousValueProperty() const { return previousValue_; }
        /** @brief Returns the data's current value. */
        [[nodiscard]] const T& getCurrentValueProperty() const { return currentValue_; }
        /** @brief Returns true if the value changed because of a change of execution context. */
        [[nodiscard]] bool getThreadContextChangedProperty() const { return threadContextChanged_; }

    private:
        T previousValue_;
        T currentValue_;
        bool threadContextChanged_;
    };

} // namespace System::Threading
