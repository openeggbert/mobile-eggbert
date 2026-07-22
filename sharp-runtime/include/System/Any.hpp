// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#pragma once

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace System
{
    /**
     * @brief A minimal type-erased value container.
     *
     * C++14-compatible replacement for std::any (C++17) and this port's general-purpose
     * substitute for .NET's `object?` wherever a type needs to hold an arbitrary caller-supplied
     * value (e.g. IAsyncResult::getAsyncStateProperty(), ContentManager's asset/reader caches).
     * Supports exactly what those callers need: construct/assign from any copyable value, and
     * retrieve it back via any_cast() with an exact-type check.
     */
    class Any
    {
    public:
        Any() = default;

        template <typename T>
        Any(T value) // NOLINT(*-explicit-constructor)
            : holder_(new Holder<typename std::decay<T>::type>(std::move(value)))
        {
        }

        Any(const Any& other)
            : holder_(other.holder_ ? other.holder_->clone() : nullptr)
        {
        }

        Any(Any&&) noexcept = default;

        Any& operator=(const Any& other)
        {
            holder_.reset(other.holder_ ? other.holder_->clone() : nullptr);
            return *this;
        }

        Any& operator=(Any&&) noexcept = default;

        template <typename T>
        Any& operator=(T value)
        {
            holder_.reset(new Holder<typename std::decay<T>::type>(std::move(value)));
            return *this;
        }

        /** @brief Returns true if this instance currently holds a value. */
        [[nodiscard]] bool HasValue() const noexcept { return holder_ != nullptr; }

    private:
        struct HolderBase
        {
            virtual ~HolderBase() = default;
            [[nodiscard]] virtual HolderBase* clone() const = 0;
        };

        template <typename T>
        struct Holder : HolderBase
        {
            explicit Holder(T v) : value(std::move(v)) {}
            [[nodiscard]] HolderBase* clone() const override { return new Holder(value); }
            T value;
        };

        std::unique_ptr<HolderBase> holder_;

        template <typename T>
        friend T any_cast(const Any& a);

        template <typename T>
        friend T* any_cast(Any* a);
    };

    /**
     * @brief Returns a copy of the value held by @p a.
     * @throws std::bad_cast if @p a does not hold exactly type T.
     */
    template <typename T>
    T any_cast(const Any& a)
    {
        auto* h = dynamic_cast<Any::Holder<typename std::decay<T>::type>*>(a.holder_.get());
        if (!h)
        {
            throw std::bad_cast();
        }
        return h->value;
    }

    /** @brief Returns a pointer to the value held by @p a, or nullptr if it does not hold exactly type T. */
    template <typename T>
    T* any_cast(Any* a)
    {
        if (!a)
        {
            return nullptr;
        }
        auto* h = dynamic_cast<Any::Holder<typename std::decay<T>::type>*>(a->holder_.get());
        return h ? &h->value : nullptr;
    }
}
