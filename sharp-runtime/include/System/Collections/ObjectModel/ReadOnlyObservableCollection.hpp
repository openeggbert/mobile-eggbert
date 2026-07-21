// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/Collections/ObjectModel/ObservableCollection.hpp"

namespace System::Collections::ObjectModel {

using SharpRuntime::intcs;

/**
 * @brief Provides a read-only wrapper around an ObservableCollection that still
 *        propagates CollectionChanged notifications to subscribers.
 *
 * C++ counterpart of .NET System.Collections.ObjectModel.ReadOnlyObservableCollection<T>.
 * Shares ownership of the wrapped ObservableCollection via shared_ptr, matching .NET's
 * live-wrapper semantics: this is a read-only *view* onto the same underlying collection,
 * not a snapshot copy. The wrapped collection is still expected to be mutated by whoever
 * holds the shared_ptr (or another copy of it); those changes, and their CollectionChanged/
 * PropertyChanged notifications, are visible/forwarded through this wrapper.
 *
 * Deviation from the previous version of this header: it took the source ObservableCollection
 * by const-ref (copying it) or by rvalue-ref (moving it), either of which produced a private,
 * disconnected copy — mutating the original source afterwards had no effect on the wrapper and
 * fired no events through it, defeating the entire purpose of an *observable* read-only view.
 *
 * @tparam T The type of elements in the collection.
 */
template<typename T>
class ReadOnlyObservableCollection {
    std::shared_ptr<ObservableCollection<T>> source_;

public:
    /** @brief Handler type for CollectionChanged subscribers. */
    using ChangedHandler = NotifyCollectionChangedEventHandler<T>;

    /** @brief List of CollectionChanged event subscribers. */
    std::vector<ChangedHandler> CollectionChanged;

    /**
     * @brief Constructs a ReadOnlyObservableCollection wrapping the given ObservableCollection.
     *
     * C++ counterpart of .NET ReadOnlyObservableCollection<T>(ObservableCollection<T>).
     * @param source A shared pointer to the ObservableCollection to wrap.
     * @throws System::ArgumentNullException if @p source is null.
     */
    explicit ReadOnlyObservableCollection(std::shared_ptr<ObservableCollection<T>> source)
        : source_(std::move(source)) {
        if (!source_)
            throw System::ArgumentNullException("list");
        source_->CollectionChanged.push_back(
            [this](void* s, const NotifyCollectionChangedEventArgs<T>& args) {
                (void)s;
                OnCollectionChanged(args);
            });
    }

    // The constructor above registers a forwarding callback with the shared source that
    // captures `this`. A copy or move would leave that callback pointing at the wrong (or, once
    // the original is destroyed, dangling) object while the shared source keeps calling it --
    // confirmed via ASan (stack-use-after-scope) with a repro that destroys the original after
    // construction and then mutates the still-alive shared source. Real .NET's
    // ReadOnlyObservableCollection<T> is a reference type, so this class's copy/move surface was
    // never actually meaningful; deleting it here just makes that reference-type intent explicit
    // and safe rather than silently unsafe.
    ReadOnlyObservableCollection(const ReadOnlyObservableCollection&) = delete;
    ReadOnlyObservableCollection& operator=(const ReadOnlyObservableCollection&) = delete;
    ReadOnlyObservableCollection(ReadOnlyObservableCollection&&) = delete;
    ReadOnlyObservableCollection& operator=(ReadOnlyObservableCollection&&) = delete;

    /**
     * @brief Gets an empty ReadOnlyObservableCollection instance.
     *
     * C++ counterpart of .NET ReadOnlyObservableCollection<T>.Empty.
     * @return A reference to a shared, permanently-empty instance.
     */
    static ReadOnlyObservableCollection<T>& Empty() {
        static ReadOnlyObservableCollection<T> empty(std::make_shared<ObservableCollection<T>>());
        return empty;
    }

    /**
     * @brief Gets the number of elements in the collection.
     *
     * C++ counterpart of .NET ReadOnlyObservableCollection<T>.Count.
     * @return The number of elements.
     */
    [[nodiscard]] intcs getCountProperty() const { return source_->getCountProperty(); }

    /**
     * @brief Gets a value indicating whether the collection contains no elements.
     *
     * C++ counterpart of checking Count == 0 on .NET ReadOnlyObservableCollection<T>.
     * @return true if the collection is empty; otherwise false.
     */
    [[nodiscard]] bool getIsEmptyProperty() const { return source_->getCountProperty() == 0; }

    /**
     * @brief Returns a const reference to the element at the specified index.
     *
     * C++ counterpart of .NET ReadOnlyObservableCollection<T>.Item[int] getter.
     * @param index The zero-based index.
     * @return A const reference to the element.
     */
    [[nodiscard]] const T& operator[](intcs index) const { return (*source_)[index]; }

    /**
     * @brief Determines whether the collection contains the specified element.
     *
     * C++ counterpart of .NET ReadOnlyObservableCollection<T>.Contains(T).
     * @param item The element to locate.
     * @return true if found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& item) const { return source_->Contains(item); }

    /**
     * @brief Returns the zero-based index of the first occurrence of @p item, or -1 if not found.
     *
     * C++ counterpart of .NET ReadOnlyCollection<T>.IndexOf(T), inherited by ReadOnlyObservableCollection.
     * @param item The element to locate.
     * @return The zero-based index, or -1 if not found.
     */
    [[nodiscard]] intcs IndexOf(const T& item) const { return source_->IndexOf(item); }

    /** @brief Returns a const iterator to the beginning of the collection (STL interop). */
    auto begin() const { return source_->begin(); }
    /** @brief Returns a const iterator past the end of the collection (STL interop). */
    auto end()   const { return source_->end(); }

protected:
    /**
     * @brief Raises the CollectionChanged event, forwarding notifications from the wrapped source.
     *
     * C++ counterpart of .NET ReadOnlyObservableCollection<T>.OnCollectionChanged(NotifyCollectionChangedEventArgs).
     * @param args The event data describing the change.
     */
    virtual void OnCollectionChanged(const NotifyCollectionChangedEventArgs<T>& args) {
        for (auto& h : CollectionChanged) h(this, args);
    }
};

} // namespace System::Collections::ObjectModel
