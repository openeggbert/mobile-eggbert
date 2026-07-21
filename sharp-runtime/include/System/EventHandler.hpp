// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

// NOTE:
// In .NET, EventHandler<TEventArgs> is only a delegate type.
// In this C++ port, it also stores subscribed handlers and provides Raise()/Invoke().
// This is intentional, because C++ has no equivalent of the C# "event" keyword.
// The goal is to keep ported XNA/.NET code visually as close as possible to the original C# source.

#include <algorithm>
#include <functional>
#include <vector>
#include <utility>
#include <cstddef>

#include "System/Object.hpp"

namespace System
{
    /**
     * @brief Represents an event-like handler collection for a specific event argument type.
     *
     * In original C#/.NET, EventHandler<TEventArgs> is only a delegate type:
     *
     * @code
     * public delegate void EventHandler<TEventArgs>(object? sender, TEventArgs e);
     * @endcode
     *
     * That means in .NET it represents the signature of a single callback only.
     *
     * In this C++ port, EventHandler<TEventArgs> intentionally combines:
     * - the callback signature
     * - the list of subscribed handlers
     * - the logic for invoking all subscribed handlers
     *
     * This design differs from .NET because C++ does not have the C# keyword
     * @c event. In C#, the usual pattern is:
     *
     * @code
     * public event EventHandler<MyEventArgs> SomethingHappened;
     * @endcode
     *
     * where:
     * - EventHandler<TEventArgs> is the delegate type
     * - event is a language feature that stores and manages subscribed delegates
     *
     * Since standard C++ has no direct equivalent of the @c event keyword,
     * this class absorbs that behavior into a single type so that ported code
     * can stay visually close to the original C# source.
     *
     * Thanks to this design, C++ code can keep declarations such as:
     *
     * @code
     * System::EventHandler<ExitingEventArgs> Exiting;
     * @endcode
     *
     * and later invoke them with:
     *
     * @code
     * Exiting.Raise(this, args);
     * @endcode
     *
     * This is not a perfect architectural match to .NET internals, but it is a
     * practical compatibility layer for source-porting XNA/.NET code to C++.
     *
     * .NET's non-generic @c EventHandler delegate (@c sender / @c EventArgs) is covered by
     * instantiating this template as @c EventHandler<System::EventArgs>. The newer two-type-parameter
     * @c EventHandler<TSender, TEventArgs> delegate (which parameterizes the sender type instead of
     * fixing it to @c object) is intentionally not modeled — it is a recent BCL addition not used by
     * ported XNA/.NET game code, and this class always passes the sender as @c Object*, matching the
     * non-generic and single-generic .NET delegates.
     *
     * @tparam TEventArgs The type of event arguments passed to subscribed handlers.
     */
    template<typename TEventArgs>
    class EventHandler
    {
    public:
        /**
         * @brief Type of one subscribed event callback.
         *
         * The callback receives:
         * - sender: pointer to the object that raised the event
         * - e: event arguments
         */
        using HandlerType = std::function<void(Object* sender, const TEventArgs& e)>;

        /** Token type returned by Add(); used to remove a specific handler via Remove(). */
        using Token = std::size_t;

    private:
        std::vector<std::pair<Token, HandlerType>> handlers_;
        Token nextToken_ = 0;

    public:
        /**
         * @brief Constructs an empty event handler collection.
         */
        EventHandler() = default;

        /**
         * @brief Destroys the event handler collection.
         */
        ~EventHandler() = default;

        /**
         * @brief Adds a new subscribed handler.
         *
         * This operator is used to mimic the common C# event subscription style
         * as closely as possible. The token is discarded; use Add() when you need
         * to remove the handler later.
         *
         * @param handler Handler to add.
         * @return Reference to this instance.
         */
        EventHandler& operator+=(HandlerType handler)
        {
            Add(std::move(handler));
            return *this;
        }

        /**
         * @brief Adds a new subscribed handler and returns a removal token.
         *
         * Pass the returned token to Remove() to unsubscribe this specific handler.
         *
         * @param handler Handler to add.
         * @return Token that identifies this subscription.
         */
        Token Add(HandlerType handler)
        {
            const Token token = nextToken_++;
            handlers_.emplace_back(token, std::move(handler));
            return token;
        }

        /**
         * @brief Removes the handler identified by the given token.
         *
         * Has no effect if the token is not found (already removed or never added).
         *
         * @param token Token returned by Add().
         */
        void Remove(Token token)
        {
            handlers_.erase(
                std::remove_if(handlers_.begin(), handlers_.end(),
                    [token](const std::pair<Token, HandlerType>& entry) {
                        return entry.first == token;
                    }),
                handlers_.end()
            );
        }

        /**
         * @brief Returns whether no handlers are currently subscribed.
         *
         * @return true if there are no subscribed handlers, otherwise false.
         */
        [[nodiscard]] bool Empty() const
        {
            return handlers_.empty();
        }

        /**
         * @brief Raises the event and invokes all subscribed handlers.
         *
         * All handlers are invoked in the order in which they were added.
         *
         * Invokes a snapshot of the handler list taken at the start of this call, matching
         * C# multicast delegate invocation semantics: a handler that calls Add()/Remove()/Clear()
         * on this same EventHandler while it is being raised only affects the *next* Raise() call,
         * not the one currently in progress. Without this, a handler removing itself (or another
         * handler) mid-raise mutates the live handler list out from under this loop, which can
         * dereference an already-destroyed std::function (undefined behavior; observed in
         * practice as an escaping std::bad_function_call).
         *
         * @param sender Object that raised the event.
         * @param e Event arguments.
         */
        void Raise(Object* sender, const TEventArgs& e)
        {
            auto snapshot = handlers_;
            for (auto& entry : snapshot)
            {
                entry.second(sender, e);
            }
        }
    };
}