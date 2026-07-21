// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

namespace System {

    /**
     * @brief A multicast delegate returning void, matching a .NET System.Action<Args...> used as a
     *        field or event (which are multicast via `+=` in C#).
     *
     * Unlike @ref System::ActionT (a single-subscriber `std::function` alias), this holds a list of
     * subscribed handlers, keeping the same `void(Args...)` signature:
     *  - `operator+=` adds a subscriber (C# `+=`);
     *  - `operator=` replaces the whole list with a single handler (C# `field = handler;`) or clears
     *    it (`field = null;` via `= nullptr`);
     *  - invocation calls every subscriber in subscription order over a snapshot of the list, so a
     *    handler that subscribes/unsubscribes during invocation only affects the next invocation
     *    (matching .NET multicast-delegate semantics; also makes invocation re-entrancy safe).
     *
     * C# delegates compare equal by target+method, which is how `event -= someHandler;` finds and
     * removes exactly that subscription. `std::function`/lambdas have no such equality in general, so
     * removal-by-identity here is token-based instead: `Add()` returns a @ref Token identifying that
     * specific subscription, and `Remove(token)` removes it later (e.g. when re-parenting a hierarchy
     * that resubscribes to a different set of ancestors — see CNA::Extended::BaseTransform in
     * cna-extended for the motivating use case). `operator+=` is unchanged and remains the right choice
     * for fire-and-forget subscribers that are never individually removed.
     *
     * @tparam Args The delegate parameter types.
     */
    template<typename... Args>
    class MulticastAction {
    public:
        /** @brief The stored handler type: `void(Args...)`. */
        using HandlerType = std::function<void(Args...)>;

        /** @brief Identifies a single subscription added via @ref Add, for later removal via @ref Remove. */
        using Token = std::uint64_t;

        /** @brief A token value that never identifies a real subscription. */
        static constexpr Token InvalidToken = 0;

        /** @brief Creates an empty multicast action (no subscribers). */
        MulticastAction() = default;

        /**
         * @brief Replaces all subscribers with a single handler (C# `field = handler;`).
         * @param handler The handler to set; an empty handler clears the list.
         * @return A reference to this action.
         */
        MulticastAction& operator=(HandlerType handler) {
            handlers_.clear();
            if (handler) {
                handlers_.emplace_back(nextToken_++, std::move(handler));
            }
            return *this;
        }

        /**
         * @brief Removes all subscribers (C# `field = null;`).
         * @return A reference to this action.
         */
        MulticastAction& operator=(std::nullptr_t) {
            handlers_.clear();
            return *this;
        }

        /**
         * @brief Adds a subscriber to the invocation list (C# `+=`).
         * @param handler The handler to add; an empty handler is ignored.
         * @return A reference to this action.
         */
        MulticastAction& operator+=(HandlerType handler) {
            Add(std::move(handler));
            return *this;
        }

        /**
         * @brief Adds a subscriber to the invocation list, like @ref operator+=, but returns a token
         * that can later be passed to @ref Remove to remove exactly this subscription.
         * @param handler The handler to add; an empty handler is ignored and @ref InvalidToken is
         *        returned.
         * @return A token identifying this subscription, or @ref InvalidToken if @p handler was empty.
         */
        Token Add(HandlerType handler) {
            if (!handler) {
                return InvalidToken;
            }
            const Token token = nextToken_++;
            handlers_.emplace_back(token, std::move(handler));
            return token;
        }

        /**
         * @brief Removes the subscription identified by @p token (C# `event -= someHandler;`, where
         * @p token stands in for delegate identity — see the class-level note on why this is
         * token-based rather than value-based).
         * @param token A token previously returned by @ref Add. @ref InvalidToken (or a token that has
         *        already been removed) is a no-op.
         * @return true if a subscription was removed; false if @p token did not identify one.
         */
        bool Remove(Token token) {
            if (token == InvalidToken) {
                return false;
            }
            const auto it = std::find_if(handlers_.begin(), handlers_.end(),
                [token](const auto& entry) { return entry.first == token; });
            if (it == handlers_.end()) {
                return false;
            }
            handlers_.erase(it);
            return true;
        }

        /**
         * @brief Invokes every subscriber in subscription order. Does nothing when empty.
         * @param args The arguments forwarded to each handler.
         */
        void operator()(Args... args) const {
            const auto snapshot = handlers_;
            for (const auto& [token, handler] : snapshot) {
                handler(args...);
            }
        }

        /**
         * @brief Invokes every subscriber; alias of operator().
         * @param args The arguments forwarded to each handler.
         */
        void Invoke(Args... args) const {
            (*this)(args...);
        }

        /**
         * @brief Tests whether there is at least one subscriber (C# `field != null`).
         * @return True if the invocation list is non-empty.
         */
        explicit operator bool() const {
            return !handlers_.empty();
        }

        /**
         * @brief Gets whether the invocation list is empty.
         * @return True if there are no subscribers.
         */
        [[nodiscard]] bool Empty() const {
            return handlers_.empty();
        }

        /** @brief Removes all subscribers. */
        void Clear() {
            handlers_.clear();
        }

    private:
        std::vector<std::pair<Token, HandlerType>> handlers_;
        Token nextToken_ = InvalidToken + 1;
    };

} // namespace System
