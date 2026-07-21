// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <list>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/IEnumerator.hpp"
#include "System/InvalidOperationException.hpp"

namespace System::Collections::Generic {

using SharpRuntime::intcs;

template<typename T> class LinkedList;

/**
 * @brief Represents a node in a LinkedList<T>.
 *
 * C++ counterpart of .NET System.Collections.Generic.LinkedListNode<T>.
 * Wraps a std::list<T> iterator and a pointer to the parent list, enabling
 * Next and Previous navigation. A default-constructed node is null (detached).
 *
 * @tparam T The type of the value held by the node.
 */
template<typename T>
class LinkedListNode {
    using list_t = std::list<T>;
    using iter_t = typename list_t::iterator;
    list_t* list_ptr_ = nullptr;
    iter_t  iter_;
    explicit LinkedListNode(list_t* lst, iter_t it) : list_ptr_(lst), iter_(it) {}
    friend class LinkedList<T>;
public:
    /** @brief Default-constructs a null node (not attached to any list). */
    LinkedListNode() = default;

    /**
     * @brief Returns true if the node is valid (attached to a list); false if null.
     *
     * C++ counterpart of checking whether a LinkedListNode<T> reference is non-null in .NET.
     */
    explicit operator bool() const { return list_ptr_ != nullptr; }

    /**
     * @brief Gets the value contained in the node.
     *
     * C++ counterpart of .NET LinkedListNode<T>.Value getter.
     * @return A reference to the value stored in the node.
     */
    T& getValueProperty() { return *iter_; }

    /**
     * @brief Gets the value contained in the node (const overload).
     *
     * C++ counterpart of .NET LinkedListNode<T>.Value getter.
     * @return A const reference to the value stored in the node.
     */
    const T& getValueProperty() const { return *iter_; }

    /**
     * @brief Gets the next node in the linked list, or a null node if this is the last node.
     *
     * C++ counterpart of .NET LinkedListNode<T>.Next.
     * @return The next node, or a null node if none exists.
     */
    [[nodiscard]] LinkedListNode<T> getNextProperty() const {
        if (!list_ptr_) return {};
        auto it = iter_;
        ++it;
        if (it == list_ptr_->end()) return {};
        return LinkedListNode<T>(list_ptr_, it);
    }

    /**
     * @brief Gets the previous node in the linked list, or a null node if this is the first node.
     *
     * C++ counterpart of .NET LinkedListNode<T>.Previous.
     * @return The previous node, or a null node if none exists.
     */
    [[nodiscard]] LinkedListNode<T> getPreviousProperty() const {
        if (!list_ptr_ || iter_ == list_ptr_->begin()) return {};
        auto it = iter_;
        --it;
        return LinkedListNode<T>(list_ptr_, it);
    }

    /** @brief Implicit conversion to T for backward-compatible use in value contexts. */
    operator T() const { return *iter_; }

    /** @brief Returns true if the node is null (default-constructed or detached). */
    bool operator==(std::nullptr_t) const { return list_ptr_ == nullptr; }
    /** @brief Returns true if the node is non-null. */
    bool operator!=(std::nullptr_t) const { return list_ptr_ != nullptr; }
    /** @brief Returns true if the node's value equals @p other. */
    bool operator==(const T& other) const { return list_ptr_ != nullptr && *iter_ == other; }
    /** @brief Returns true if the node's value differs from @p other. */
    bool operator!=(const T& other) const { return !(*this == other); }
};

/**
 * @brief Represents a doubly linked list.
 *
 * C++ counterpart of .NET System.Collections.Generic.LinkedList<T>.
 * Backed by std::list<T>; provides O(1) insertion and removal at known nodes.
 *
 * @note GetEnumerator()'s Enumerator detects structural modification (AddFirst/AddLast/
 * AddBefore/AddAfter/Remove/RemoveFirst/RemoveLast/Clear) during iteration via a version
 * counter, matching .NET's InvalidOperationException fail-fast contract (see List<T>/ArrayList's
 * Enumerator in this codebase for the same established pattern). Directly using begin()/end()
 * STL iterators still follows plain std::list invalidation rules, not .NET's version-checked
 * contract (erasing a node only invalidates that node's own iterator under std::list semantics;
 * other iterators remain valid) -- only the GetEnumerator()-returned Enumerator is fail-fast.
 *
 * @note Missing surface, and why: real .NET's LinkedListNode<T> is an independently
 * allocatable object that can exist *detached* from any list (`new LinkedListNode<T>(value)`),
 * which is why LinkedList<T> exposes AddFirst/AddLast/AddBefore/AddAfter overloads that take an
 * already-constructed LinkedListNode<T> and attach it (throwing InvalidOperationException if
 * that node already belongs to a list). This port's LinkedListNode<T> is instead just a
 * (list pointer, std::list iterator) pair with no independent existence outside a list --
 * architecturally, there is no way to "pre-construct" a detached node the way .NET's design
 * allows. Porting the existing-node overloads would need a real design change (a heap-allocated
 * node type), not a mechanical add, so they're deliberately omitted rather than faked with a
 * signature that can't preserve the real semantics.
 *
 * @tparam T The type of elements in the linked list.
 */
template<typename T>
class LinkedList {
    std::list<T> list_;
    intcs version_ = 0;

    class Enumerator : public IEnumerator<T> {
        const LinkedList<T>* list_;
        intcs version_;
        typename std::list<T>::iterator cur_, end_;
        bool started_ = false;
    public:
        Enumerator(const LinkedList<T>* list, typename std::list<T>::iterator b, typename std::list<T>::iterator e)
            : list_(list), version_(list->version_), cur_(b), end_(e) {}
        bool MoveNext() override {
            if (version_ != list_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            if (!started_) { started_ = true; } else { ++cur_; }
            return cur_ != end_;
        }
        void Reset() override {
            if (version_ != list_->version_)
                throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            started_ = false;
        }
        const T& Current() const override { return *cur_; }
    };

    /** Validates that @p node is non-null and belongs to this list. C++ counterpart of .NET LinkedList<T>.ValidateNode. */
    void validateNode(const LinkedListNode<T>& node) const {
        if (!node) throw System::ArgumentNullException("node");
        if (node.list_ptr_ != &list_) throw System::InvalidOperationException("The LinkedList node does not belong to current LinkedList.");
    }

public:
    /** @brief Initializes a new empty LinkedList<T>. */
    LinkedList() = default;

    /**
     * @brief Gets the number of nodes in the linked list.
     *
     * C++ counterpart of .NET LinkedList<T>.Count.
     * @return The number of nodes.
     */
    [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(list_.size()); }

    /**
     * @brief Gets the first node of the linked list, or a null node if the list is empty.
     *
     * C++ counterpart of .NET LinkedList<T>.First.
     * @return The first node, or a null node.
     */
    [[nodiscard]] LinkedListNode<T> getFirstProperty() {
        if (list_.empty()) return {};
        return LinkedListNode<T>(&list_, list_.begin());
    }

    /**
     * @brief Gets the first node of the linked list (const overload), or null if empty.
     *
     * @return The first node, or a null node.
     */
    [[nodiscard]] LinkedListNode<T> getFirstProperty() const {
        if (list_.empty()) return {};
        return LinkedListNode<T>(const_cast<std::list<T>*>(&list_),
                                 const_cast<std::list<T>&>(list_).begin());
    }

    /**
     * @brief Gets the last node of the linked list, or a null node if the list is empty.
     *
     * C++ counterpart of .NET LinkedList<T>.Last.
     * @return The last node, or a null node.
     */
    [[nodiscard]] LinkedListNode<T> getLastProperty() {
        if (list_.empty()) return {};
        auto it = list_.end(); --it;
        return LinkedListNode<T>(&list_, it);
    }

    /**
     * @brief Gets the last node of the linked list (const overload), or null if empty.
     *
     * @return The last node, or a null node.
     */
    [[nodiscard]] LinkedListNode<T> getLastProperty() const {
        if (list_.empty()) return {};
        auto& ml = const_cast<std::list<T>&>(list_);
        auto it = ml.end(); --it;
        return LinkedListNode<T>(const_cast<std::list<T>*>(&list_), it);
    }

    /**
     * @brief Adds a new node containing @p value at the start of the list.
     *
     * C++ counterpart of .NET LinkedList<T>.AddFirst(T).
     * @param value The value to add.
     * @return The new node.
     */
    LinkedListNode<T> AddFirst(const T& value) {
        list_.push_front(value);
        ++version_;
        return LinkedListNode<T>(&list_, list_.begin());
    }

    /**
     * @brief Adds a new node containing @p value at the end of the list.
     *
     * C++ counterpart of .NET LinkedList<T>.AddLast(T).
     * @param value The value to add.
     * @return The new node.
     */
    LinkedListNode<T> AddLast(const T& value) {
        list_.push_back(value);
        ++version_;
        auto it = list_.end(); --it;
        return LinkedListNode<T>(&list_, it);
    }

    /**
     * @brief Adds a new node containing @p value immediately before @p node.
     *
     * C++ counterpart of .NET LinkedList<T>.AddBefore(LinkedListNode<T>, T).
     * @param node  The node before which to insert.
     * @param value The value to add.
     * @return The new node.
     * @throws System::ArgumentNullException if @p node is null.
     * @throws System::InvalidOperationException if @p node does not belong to this list.
     */
    LinkedListNode<T> AddBefore(LinkedListNode<T> node, const T& value) {
        validateNode(node);
        auto it = list_.insert(node.iter_, value);
        ++version_;
        return LinkedListNode<T>(&list_, it);
    }

    /**
     * @brief Adds a new node containing @p value immediately after @p node.
     *
     * C++ counterpart of .NET LinkedList<T>.AddAfter(LinkedListNode<T>, T).
     * @param node  The node after which to insert.
     * @param value The value to add.
     * @return The new node.
     * @throws System::ArgumentNullException if @p node is null.
     * @throws System::InvalidOperationException if @p node does not belong to this list.
     */
    LinkedListNode<T> AddAfter(LinkedListNode<T> node, const T& value) {
        validateNode(node);
        auto it = node.iter_;
        ++it;
        it = list_.insert(it, value);
        ++version_;
        return LinkedListNode<T>(&list_, it);
    }

    /**
     * @brief Removes the node at the start of the list.
     *
     * C++ counterpart of .NET LinkedList<T>.RemoveFirst().
     * @throws System::InvalidOperationException if the list is empty.
     */
    void RemoveFirst() {
        if (list_.empty()) throw System::InvalidOperationException("The LinkedList is empty.");
        list_.pop_front();
        ++version_;
    }

    /**
     * @brief Removes the node at the end of the list.
     *
     * C++ counterpart of .NET LinkedList<T>.RemoveLast().
     * @throws System::InvalidOperationException if the list is empty.
     */
    void RemoveLast() {
        if (list_.empty()) throw System::InvalidOperationException("The LinkedList is empty.");
        list_.pop_back();
        ++version_;
    }

    /**
     * @brief Removes the first occurrence of @p value from the list.
     *
     * C++ counterpart of .NET LinkedList<T>.Remove(T).
     * @param value The value to remove.
     * @return true if an element was found and removed; otherwise false.
     */
    bool Remove(const T& value) {
        auto it = std::find(list_.begin(), list_.end(), value);
        if (it == list_.end()) return false;
        list_.erase(it);
        ++version_;
        return true;
    }

    /**
     * @brief Removes the specified node from the list.
     *
     * C++ counterpart of .NET LinkedList<T>.Remove(LinkedListNode<T>).
     * @param node The node to remove.
     * @throws System::ArgumentNullException if @p node is null.
     * @throws System::InvalidOperationException if @p node does not belong to this list.
     */
    void Remove(LinkedListNode<T> node) {
        validateNode(node);
        list_.erase(node.iter_);
        ++version_;
    }

    /**
     * @brief Removes all nodes from the list.
     *
     * C++ counterpart of .NET LinkedList<T>.Clear().
     */
    void Clear() { list_.clear(); ++version_; }

    /**
     * @brief Determines whether @p value is in the list.
     *
     * C++ counterpart of .NET LinkedList<T>.Contains(T).
     * @param value The value to locate.
     * @return true if the value is found; otherwise false.
     */
    [[nodiscard]] bool Contains(const T& value) const {
        return std::find(list_.begin(), list_.end(), value) != list_.end();
    }

    /**
     * @brief Finds the first node containing @p value.
     *
     * C++ counterpart of .NET LinkedList<T>.Find(T).
     * @param value The value to search for.
     * @return The first matching node, or a null node if not found.
     */
    [[nodiscard]] LinkedListNode<T> Find(const T& value) {
        auto it = std::find(list_.begin(), list_.end(), value);
        if (it == list_.end()) return {};
        return LinkedListNode<T>(&list_, it);
    }

    /**
     * @brief Finds the last node containing @p value.
     *
     * C++ counterpart of .NET LinkedList<T>.FindLast(T).
     * @param value The value to search for.
     * @return The last matching node, or a null node if not found.
     */
    [[nodiscard]] LinkedListNode<T> FindLast(const T& value) {
        for (auto it = list_.end(); it != list_.begin(); ) {
            --it;
            if (*it == value) return LinkedListNode<T>(&list_, it);
        }
        return {};
    }

    /**
     * @brief Copies the list elements to a vector starting at @p index.
     *
     * C++ counterpart of .NET LinkedList<T>.CopyTo(T[], int).
     * @param dest  The destination vector (must have sufficient capacity).
     * @param index The zero-based index in @p dest at which copying begins.
     * @throws System::ArgumentOutOfRangeException if @p index is negative, or if @p index is
     *         greater than the length of @p dest (real .NET's LinkedList<T>.CopyTo makes this a
     *         *separate* check from the insufficient-space case below, throwing
     *         ArgumentOutOfRangeException rather than ArgumentException for it -- e.g. an
     *         out-of-bounds index against an EMPTY list still throws the range exception, not
     *         the space one, since there is no "insufficient space" to speak of when there is
     *         nothing to copy).
     * @throws System::ArgumentException if @p dest does not have enough remaining room (from
     *         @p index onward) for every element of this list.
     */
    void CopyTo(std::vector<T>& dest, intcs index) const {
        if (index < 0)
            throw System::ArgumentOutOfRangeException("index", "Non-negative number required.");
        if (static_cast<std::size_t>(index) > dest.size())
            throw System::ArgumentOutOfRangeException("index", "Larger than collection size.");
        if (dest.size() - static_cast<std::size_t>(index) < list_.size())
            throw System::ArgumentException("Destination array is not long enough to copy all the items in the collection.");
        for (const auto& item : list_) {
            dest[static_cast<std::size_t>(index++)] = item;
        }
    }

    /**
     * @brief Returns an enumerator that iterates through the list.
     *
     * C++ counterpart of .NET LinkedList<T>.GetEnumerator().
     * @return A heap-allocated IEnumerator<T>; caller takes ownership.
     */
    [[nodiscard]] IEnumerator<T>* GetEnumerator() {
        return new Enumerator(this, list_.begin(), list_.end());
    }

    /** @brief Returns an iterator to the first node (STL interop). */
    auto begin()       { return list_.begin(); }
    /** @brief Returns an iterator past the last node (STL interop). */
    auto end()         { return list_.end(); }
    /** @brief Returns a const iterator to the first node (STL interop). */
    auto begin() const { return list_.cbegin(); }
    /** @brief Returns a const iterator past the last node (STL interop). */
    auto end()   const { return list_.cend(); }
};

} // namespace System::Collections::Generic
