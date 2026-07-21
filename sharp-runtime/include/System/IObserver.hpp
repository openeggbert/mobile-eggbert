// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <exception>

namespace System {

    /**
     * @brief Provides a mechanism for receiving push-based notifications.
     *
     * C++ counterpart of .NET System.IObserver<T> interface.
     * Implement this interface to act as an observer in the observer design
     * pattern. Use together with IObservable<T>, which represents the provider
     * (source of notifications).
     *
     * @note .NET declares this interface contravariant (`IObserver<in T>`), so
     *   `IObserver<Base>` is usable wherever `IObserver<Derived>` is expected.
     *   C++ class templates have no equivalent variance mechanism, so
     *   `IObserver<Derived>` and `IObserver<Base>` are unrelated types here.
     * @tparam T The object that provides notification information.
     */
    template<typename T>
    class IObserver {
    public:
        /** @brief Virtual destructor for safe polymorphic destruction. */
        virtual ~IObserver() = default;

        /**
         * @brief Provides the observer with new data.
         *
         * C++ counterpart of .NET IObserver<T>.OnNext(T).
         * @param value The current notification information.
         */
        virtual void OnNext(const T& value) = 0;

        /**
         * @brief Notifies the observer that the provider has experienced an
         * error condition.
         *
         * C++ counterpart of .NET IObserver<T>.OnError(Exception).
         * After this call the provider guarantees no further OnNext or
         * OnCompleted calls will be made.
         * @param error An object that provides additional information about
         *              the error.
         */
        virtual void OnError(const std::exception& error) = 0;

        /**
         * @brief Notifies the observer that the provider has finished sending
         * push-based notifications.
         *
         * C++ counterpart of .NET IObserver<T>.OnCompleted().
         * After this call the provider guarantees no further OnNext or OnError
         * calls will be made.
         */
        virtual void OnCompleted() = 0;
    };

} // namespace System
