// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <utility>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Specialized/NotifyCollectionChangedAction.hpp"

namespace System::Collections::Specialized {

/**
 * @brief Provides data for the CollectionChanged event.
 *
 * C++ counterpart of .NET System.Collections.Specialized.NotifyCollectionChangedEventArgs.
 * .NET's version stores items as a non-generic IList; this port is templated on the element
 * type instead, since sharp-runtime has no boxed-object collection type to mirror IList with.
 *
 * @tparam T The type of elements in the collection.
 */
template<typename T>
class NotifyCollectionChangedEventArgs {
    /** @brief Tag used by the Move() factories to bypass constructor validation. */
    struct RawTag {};

    NotifyCollectionChangedEventArgs(RawTag, NotifyCollectionChangedAction action) : Action(action) {}

public:
    /** @brief The action that caused the event. */
    NotifyCollectionChangedAction Action;
    /** @brief New items involved in the change (Add, Replace, Move). */
    std::vector<T> NewItems;
    /** @brief Old items involved in the change (Remove, Replace, Move). */
    std::vector<T> OldItems;
    /** @brief Zero-based index at which the change occurred in the new list. */
    SharpRuntime::intcs NewStartingIndex = -1;
    /** @brief Zero-based index at which the change occurred in the old list. */
    SharpRuntime::intcs OldStartingIndex = -1;

    /**
     * @brief Constructs event args that describe a Reset change.
     *
     * C++ counterpart of .NET NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction).
     * @param action The action that caused the event; must be Reset.
     * @throws System::ArgumentException if @p action is not Reset.
     */
    explicit NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction action) : Action(action) {
        if (action != NotifyCollectionChangedAction::Reset)
            throw System::ArgumentException("This constructor can only be used with the Reset action.", "action");
    }

    /**
     * @brief Constructs event args that describe a single-item Add or Remove change.
     *
     * C++ counterpart of .NET NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction, object, int).
     * @param action      The action that caused the event; must be Add or Remove.
     * @param changedItem The item affected by the change.
     * @param index       The zero-based index where the change occurred.
     * @throws System::ArgumentException if @p action is not Add or Remove.
     */
    NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction action, T changedItem,
                                      SharpRuntime::intcs index = -1)
        : Action(action) {
        switch (action) {
            case NotifyCollectionChangedAction::Add:
                NewItems.push_back(std::move(changedItem));
                NewStartingIndex = index;
                break;
            case NotifyCollectionChangedAction::Remove:
                OldItems.push_back(std::move(changedItem));
                OldStartingIndex = index;
                break;
            default:
                throw System::ArgumentException("This constructor can only be used with the Add or Remove action.", "action");
        }
    }

    /**
     * @brief Constructs event args that describe a multi-item Add or Remove change.
     *
     * C++ counterpart of .NET NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction, IList, int).
     * @param action        The action that caused the event; must be Add or Remove.
     * @param changedItems  The items affected by the change.
     * @param startingIndex The zero-based index where the change occurred.
     * @throws System::ArgumentException if @p action is not Add or Remove.
     * @throws System::ArgumentOutOfRangeException if @p startingIndex is less than -1.
     */
    NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction action, std::vector<T> changedItems,
                                      SharpRuntime::intcs startingIndex = -1)
        : Action(action) {
        switch (action) {
            case NotifyCollectionChangedAction::Add:
            case NotifyCollectionChangedAction::Remove:
                if (startingIndex < -1)
                    throw System::ArgumentOutOfRangeException("startingIndex");
                if (action == NotifyCollectionChangedAction::Add) {
                    NewItems = std::move(changedItems);
                    NewStartingIndex = startingIndex;
                } else {
                    OldItems = std::move(changedItems);
                    OldStartingIndex = startingIndex;
                }
                break;
            default:
                throw System::ArgumentException("This constructor can only be used with the Add or Remove action.", "action");
        }
    }

    /**
     * @brief Builds event args that describe a single-item Replace change.
     *
     * C++ counterpart of .NET NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction, object, object, int).
     * Exposed as a named factory rather than an overloaded constructor: a constructor shaped
     * (action, T, T, intcs=-1) would collide with the single-item Add/Remove constructor above
     * — shaped (action, T, intcs=-1) — whenever T is itself SharpRuntime::intcs (e.g.
     * NotifyCollectionChangedEventArgs<int>), since both would resolve to the same 3-argument
     * (action, intcs, intcs) call.
     * @param newItem The new item replacing the original item.
     * @param oldItem The original item that was replaced.
     * @param index   The zero-based index of the replaced item.
     */
    static NotifyCollectionChangedEventArgs Replace(T newItem, T oldItem, SharpRuntime::intcs index = -1) {
        NotifyCollectionChangedEventArgs args(RawTag{}, NotifyCollectionChangedAction::Replace);
        args.NewItems.push_back(std::move(newItem));
        args.OldItems.push_back(std::move(oldItem));
        args.NewStartingIndex = args.OldStartingIndex = index;
        return args;
    }

    /**
     * @brief Constructs event args that describe a multi-item Replace change.
     *
     * C++ counterpart of .NET NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction, IList, IList, int).
     * @param action        The action that caused the event; must be Replace.
     * @param newItems      The new items replacing the original items.
     * @param oldItems      The original items that were replaced.
     * @param startingIndex The zero-based starting index of the replaced items.
     * @throws System::ArgumentException if @p action is not Replace.
     */
    NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction action, std::vector<T> newItems,
                                      std::vector<T> oldItems, SharpRuntime::intcs startingIndex = -1)
        : Action(action) {
        if (action != NotifyCollectionChangedAction::Replace)
            throw System::ArgumentException("This constructor can only be used with the Replace action.", "action");
        NewItems = std::move(newItems);
        OldItems = std::move(oldItems);
        NewStartingIndex = OldStartingIndex = startingIndex;
    }

    /**
     * @brief Builds event args that describe a single-item Move change.
     *
     * C++ counterpart of .NET NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction, object, int, int).
     * Exposed as a named factory rather than an overloaded constructor: a constructor shaped
     * (action, T, intcs, intcs) would collide with the Replace constructor above whenever T is
     * itself SharpRuntime::intcs (e.g. NotifyCollectionChangedEventArgs<int>), producing two
     * constructors with an identical parameter list.
     * @param changedItem The item that was moved.
     * @param index       The new index of the item.
     * @param oldIndex    The previous index of the item.
     * @throws System::ArgumentOutOfRangeException if @p index is negative.
     */
    static NotifyCollectionChangedEventArgs Move(T changedItem, SharpRuntime::intcs index,
                                                  SharpRuntime::intcs oldIndex) {
        if (index < 0)
            throw System::ArgumentOutOfRangeException("index");
        NotifyCollectionChangedEventArgs args(RawTag{}, NotifyCollectionChangedAction::Move);
        args.NewItems = {changedItem};
        args.OldItems = {std::move(changedItem)};
        args.NewStartingIndex = index;
        args.OldStartingIndex = oldIndex;
        return args;
    }

    /**
     * @brief Builds event args that describe a multi-item Move change.
     *
     * C++ counterpart of .NET NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction, IList, int, int).
     * @param changedItems The items that were moved.
     * @param index        The new starting index of the items.
     * @param oldIndex     The previous starting index of the items.
     * @throws System::ArgumentOutOfRangeException if @p index is negative.
     */
    static NotifyCollectionChangedEventArgs Move(std::vector<T> changedItems, SharpRuntime::intcs index,
                                                  SharpRuntime::intcs oldIndex) {
        if (index < 0)
            throw System::ArgumentOutOfRangeException("index");
        NotifyCollectionChangedEventArgs args(RawTag{}, NotifyCollectionChangedAction::Move);
        args.NewItems = changedItems;
        args.OldItems = std::move(changedItems);
        args.NewStartingIndex = index;
        args.OldStartingIndex = oldIndex;
        return args;
    }
};

} // namespace System::Collections::Specialized
