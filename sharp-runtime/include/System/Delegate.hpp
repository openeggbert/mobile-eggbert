// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <any>
#include <functional>
#include <memory>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace System {

/**
 * @brief Base class for all delegate types.
 *
 * C++ counterpart of .NET System.Delegate. In .NET, Delegate is the abstract root
 * of all compiler-generated delegate types. In C++ there is no delegate syntax, so
 * this class provides the core runtime API: Combine, Remove, GetInvocationList,
 * HasSingleTarget, and type-erased void invocation via Invoke().
 *
 * DynamicInvoke always throws NotImplementedException — C++ has no late-bound
 * object[] invocation equivalent.
 *
 * When GetInvocationList() is called on a single-target delegate the object must
 * already be heap-allocated and owned by a std::shared_ptr (std::bad_weak_ptr is
 * thrown otherwise).
 */
class Delegate : public std::enable_shared_from_this<Delegate> {
public:
    /** @brief Type-erased void callable stored in single-target delegates. */
    using ErasedInvoke = std::function<void()>;

private:
    struct MulticastTag {};

    ErasedInvoke                           invoke_;
    std::vector<std::shared_ptr<Delegate>> invocationList_;

    Delegate(MulticastTag, std::vector<std::shared_ptr<Delegate>> list)
        : invocationList_(std::move(list)) {}

public:
    /** @brief Constructs an empty (no-op) delegate. */
    Delegate() = default;

    /**
     * @brief Constructs a single-target delegate wrapping the given callable.
     * @param invoke Type-erased callable to invoke via Invoke().
     */
    explicit Delegate(ErasedInvoke invoke) : invoke_(std::move(invoke)) {}

    virtual ~Delegate() = default;

    // -------------------------------------------------------------------------

    /**
     * @brief Invokes all targets in the invocation list in order.
     * For a single-target delegate, invokes the wrapped callable.
     */
    virtual void Invoke() const;

    /** @brief Calls Invoke(). */
    void operator()() const { Invoke(); }

    // -------------------------------------------------------------------------

    /**
     * @brief Gets a value indicating whether the delegate has a single invocation target.
     * @return true if there is at most one target.
     */
    [[nodiscard]] bool getHasSingleTargetProperty() const;

    /**
     * @brief Returns the invocation list.
     * For multicast delegates this returns the stored list.
     * For a single-target delegate this returns a one-element vector containing
     * a shared_ptr to this object (requires that this is managed by a shared_ptr).
     */
    [[nodiscard]] std::vector<std::shared_ptr<Delegate>> GetInvocationList() const;

    /**
     * @brief Creates a shallow copy of this delegate.
     * @return A new shared_ptr<Delegate> with the same invoke target or invocation list.
     */
    [[nodiscard]] virtual std::shared_ptr<Delegate> Clone() const;

    /**
     * @brief Determines whether this delegate equals other.
     *
     * C++ counterpart of .NET MulticastDelegate.Equals(object).
     * Compares the invocation lists element-by-element by pointer identity.
     * Single-target delegates compare as equal only when they wrap the same
     * underlying shared_ptr-managed object.
     *
     * @return true if both invocation lists have the same length and
     *         corresponding entries are pointer-identical.
     */
    [[nodiscard]] virtual bool Equals(const Delegate& other) const;

    /**
     * @brief Returns a hash code for this delegate.
     *
     * C++ counterpart of .NET MulticastDelegate.GetHashCode().
     * The hash is derived from the invocation list contents:
     *   - An empty (no-target) delegate returns 0.
     *   - A single-target delegate hashes its internal callable address.
     *   - A multicast delegate XOR-folds the pointer hashes of its entries.
     */
    [[nodiscard]] std::size_t GetHashCode() const noexcept;

    /**
     * @brief Gets the class instance on which the current delegate invokes the instance method.
     *
     * C++ counterpart of .NET Delegate.Target.
     * Always returns nullptr — sharp-runtime does not track the bound target object separately
     * from the callable, and CLR reflection is not available.
     * @return nullptr.
     */
    [[nodiscard]] virtual void* getTargetProperty() const noexcept { return nullptr; }

    /**
     * @brief Not implemented — always throws NotImplementedException.
     * C++ has no equivalent of .NET late-bound object[] invocation.
     */
    virtual std::any DynamicInvoke(const std::vector<std::any>& args);

    // -------------------------------------------------------------------------

    /**
     * @brief Concatenates the invocation lists of a and b.
     * @return Combined delegate; if either is null the other is returned.
     *         Returns null if both are null.
     */
    static std::shared_ptr<Delegate> Combine(
        std::shared_ptr<Delegate> a, std::shared_ptr<Delegate> b);

    /**
     * @brief Concatenates the invocation lists of all delegates in the vector.
     * @return Combined delegate, or null if the vector is empty or all entries are null.
     */
    static std::shared_ptr<Delegate> Combine(
        const std::vector<std::shared_ptr<Delegate>>& delegates);

    /**
     * @brief Removes the last occurrence of value from source's invocation list.
     * @param source The delegate to remove from; null returns null.
     * @param value  The delegate to remove; null returns source unchanged.
     * @return New delegate without the removed entry, or null if the list becomes empty.
     *         Returns source unchanged (same pointer) if value was not found.
     */
    static std::shared_ptr<Delegate> Remove(
        std::shared_ptr<Delegate> source, std::shared_ptr<Delegate> value);

    /**
     * @brief Removes all occurrences of value from source's invocation list.
     * @return New delegate with all matching entries removed, or null if none remain.
     */
    static std::shared_ptr<Delegate> RemoveAll(
        std::shared_ptr<Delegate> source, std::shared_ptr<Delegate> value);

    // -------------------------------------------------------------------------

    bool operator==(const Delegate& o) const { return Equals(o); }
    bool operator!=(const Delegate& o) const { return !Equals(o); }

    // -------------------------------------------------------------------------

    /**
     * @brief Provides a zero-allocation enumerator over a delegate's invocation list.
     *
     * C++ counterpart of .NET System.Delegate.InvocationListEnumerator<TDelegate>.
     * Supports both explicit MoveNext() / getCurrentProperty() and C++ range-based for loops.
     *
     * @tparam TDelegate Delegate or a subclass of Delegate.
     */
    template<typename TDelegate>
    struct InvocationListEnumerator {
    private:
        std::vector<std::shared_ptr<Delegate>> list_;
        int                                    index_   = -1;
        std::shared_ptr<TDelegate>             current_;

    public:
        InvocationListEnumerator() = default;

        /** @brief Constructs an enumerator over the invocation list of d. */
        explicit InvocationListEnumerator(std::shared_ptr<Delegate> d) {
            if (d) list_ = d->GetInvocationList();
        }

        /** @brief Gets the element at the current enumerator position. */
        [[nodiscard]] std::shared_ptr<TDelegate> getCurrentProperty() const { return current_; }

        /**
         * @brief Advances the enumerator to the next element.
         * @return true if an element is available; false at end of list.
         */
        bool MoveNext() {
            int next = index_ + 1;
            if (next < static_cast<int>(list_.size())) {
                current_ = std::dynamic_pointer_cast<TDelegate>(list_[next]);
                index_   = next;
                return true;
            }
            current_ = nullptr;
            return false;
        }

        /** @brief Returns this enumerator (foreach-compatibility, mirrors .NET GetEnumerator). */
        InvocationListEnumerator<TDelegate> GetEnumerator() const { return *this; }

        // ---- range-based for support ----

        struct Iterator {
            InvocationListEnumerator* e;
            bool                      valid;
            bool operator!=(const Iterator& o) const { return valid != o.valid; }
            Iterator& operator++() { valid = e->MoveNext(); return *this; }
            std::shared_ptr<TDelegate> operator*() const { return e->getCurrentProperty(); }
        };

        /** @brief Returns a begin iterator for range-based for. */
        Iterator begin() { bool ok = MoveNext(); return {this, ok}; }
        /** @brief Returns the sentinel end iterator. */
        Iterator end()   { return {this, false}; }
    };

    /**
     * @brief Returns an enumerator over the invocation list of d.
     *
     * C++ counterpart of .NET Delegate.EnumerateInvocationList<TDelegate>(TDelegate? d).
     * A null delegate yields an empty enumerator. The returned enumerator can be used
     * in a C++ range-based for loop or via explicit MoveNext() / getCurrentProperty() calls.
     *
     * @tparam TDelegate Must be Delegate or a subclass of Delegate.
     * @param d The delegate to enumerate.
     * @return InvocationListEnumerator<TDelegate> ready to iterate.
     */
    template<typename TDelegate>
    static InvocationListEnumerator<TDelegate> EnumerateInvocationList(std::shared_ptr<TDelegate> d) {
        return InvocationListEnumerator<TDelegate>(std::static_pointer_cast<Delegate>(d));
    }
};

} // namespace System
