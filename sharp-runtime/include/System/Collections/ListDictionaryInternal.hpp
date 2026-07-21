// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <list>
#include <stdexcept>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Collections/IDictionary.hpp"
#include "System/Collections/IDictionaryEnumerator.hpp"

namespace System::Collections {

using SharpRuntime::intcs;

/**
 * @brief Implements IDictionary using a singly linked list.
 *
 * C++ counterpart of .NET System.Collections.ListDictionaryInternal.
 * Recommended for collections that typically include fewer than 10 items.
 *
 * @warning Keys are compared by pointer identity (`==` on `const void*`), not by value.
 * Real .NET compares keys with `key.Equals(other)` (ListDictionaryInternal.cs), so e.g. two
 * distinct `string` objects with equal contents are the same key in .NET but distinct keys
 * here. This is a permanent architectural limitation, not a bug to be fixed locally: C++ has
 * no common object root, so this non-generic, `const void*`-keyed type cannot call a virtual
 * `Equals` on an arbitrary key without knowing its concrete type (same root cause as
 * System::Collections::Comparer and System::Collections::StructuralComparisons). Callers
 * needing value-based key comparison should use a generic, typed dictionary instead.
 */
class ListDictionaryInternal : public IDictionary {
    struct Node {
        const void* key;
        void*       value;
    };
    std::list<Node> list_;
    intcs version_ = 0;

    // Shared forward enumerator over list_'s Nodes; getEntryProperty()/getKeyProperty()/
    // getValueProperty() expose the current entry, matching IDictionaryEnumerator.
    class NodeEnumerator : public IDictionaryEnumerator {
        const ListDictionaryInternal* d_;
        intcs version_;
        std::list<Node>::const_iterator it_;
        bool started_ = false;
        bool valid_   = false;

    public:
        explicit NodeEnumerator(const ListDictionaryInternal* d)
            : d_(d), version_(d->version_), it_(d->list_.begin()) {}

        bool MoveNext() override {
            if (version_ != d_->version_) throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            if (!started_) { it_ = d_->list_.begin(); started_ = true; }
            else if (valid_) { ++it_; }
            valid_ = (it_ != d_->list_.end());
            return valid_;
        }

        void Reset() override {
            if (version_ != d_->version_) throw System::InvalidOperationException("Collection was modified; enumeration operation may not execute.");
            started_ = false;
            valid_ = false;
            it_ = d_->list_.begin();
        }

        [[nodiscard]] void* getCurrentProperty() const override {
            return const_cast<void*>(getKeyProperty());
        }

        [[nodiscard]] DictionaryEntry getEntryProperty() const override {
            requireValid();
            return DictionaryEntry(it_->key, it_->value);
        }

        [[nodiscard]] const void* getKeyProperty() const override {
            requireValid();
            return it_->key;
        }

        [[nodiscard]] const void* getValueProperty() const override {
            requireValid();
            return it_->value;
        }

    private:
        void requireValid() const {
            if (!started_ || !valid_)
                throw System::InvalidOperationException("Enumeration has either not started or has already finished.");
        }
    };

    // Read-only ICollection view over either the keys or the values of the dictionary.
    class MemberCollection : public ICollection {
        const ListDictionaryInternal* d_;
        bool keys_; // true: enumerate keys; false: enumerate values

        class Enumerator : public IEnumerator {
            IDictionaryEnumerator* inner_;
            bool keys_;
        public:
            Enumerator(IDictionaryEnumerator* inner, bool keys) : inner_(inner), keys_(keys) {}
            ~Enumerator() override { delete inner_; }
            bool MoveNext() override { return inner_->MoveNext(); }
            void Reset() override { inner_->Reset(); }
            [[nodiscard]] void* getCurrentProperty() const override {
                return const_cast<void*>(keys_ ? inner_->getKeyProperty() : inner_->getValueProperty());
            }
        };

    public:
        MemberCollection(const ListDictionaryInternal* d, bool keys) : d_(d), keys_(keys) {}

        [[nodiscard]] intcs getCountProperty() const override { return d_->getCountProperty(); }

        void CopyTo(void* array, intcs index) override {
            auto** dest = static_cast<void**>(array);
            intcs i = index;
            for (const auto& n : d_->list_) dest[i++] = keys_ ? const_cast<void*>(n.key) : n.value;
        }

        [[nodiscard]] bool getIsSynchronizedProperty() const override { return false; }
        [[nodiscard]] const void* getSyncRootProperty() const override { return d_; }

        [[nodiscard]] IEnumerator* GetEnumerator() override {
            return new Enumerator(new NodeEnumerator(d_), keys_);
        }
    };

public:
    /** @brief Initializes a new empty ListDictionaryInternal. */
    ListDictionaryInternal() = default;

    /**
     * @brief Gets the number of key/value pairs contained in the dictionary.
     *
     * C++ counterpart of .NET ListDictionaryInternal.Count.
     */
    [[nodiscard]] intcs getCountProperty() const override {
        return static_cast<intcs>(list_.size());
    }

    /**
     * @brief Copies dictionary entries (as DictionaryEntry) to a buffer starting at the given index.
     *
     * C++ counterpart of .NET ICollection.CopyTo(Array, int).
     * @param array Pointer to a DictionaryEntry[] buffer.
     * @param index Zero-based index at which copying begins.
     */
    void CopyTo(void* array, intcs index) override {
        auto* dest = static_cast<DictionaryEntry*>(array);
        intcs i = index;
        for (const auto& n : list_) dest[i++] = DictionaryEntry(n.key, n.value);
    }

    /**
     * @brief Gets the value associated with the specified key, or nullptr if not found.
     *
     * C++ counterpart of .NET ListDictionaryInternal indexer getter.
     * @param key Pointer used as the key (compared by address).
     */
    [[nodiscard]] void* getItem(const void* key) const override {
        for (const auto& n : list_) {
            if (n.key == key) return n.value;
        }
        return nullptr;
    }

    /**
     * @brief Sets the value for the key, adding a new entry if the key is not present.
     *
     * C++ counterpart of .NET ListDictionaryInternal indexer setter.
     * @param key   Pointer used as the key (compared by address).
     * @param value Value to associate with the key.
     */
    void setItem(const void* key, void* value) override {
        for (auto& n : list_) {
            if (n.key == key) { n.value = value; return; }
        }
        list_.push_back({key, value});
        ++version_;
    }

    /**
     * @brief Gets an ICollection view over the dictionary's keys.
     *
     * C++ counterpart of .NET IDictionary.Keys.
     * @return Heap-allocated ICollection; caller takes ownership.
     */
    [[nodiscard]] ICollection* getKeysProperty() const override { return new MemberCollection(this, true); }

    /**
     * @brief Gets an ICollection view over the dictionary's values.
     *
     * C++ counterpart of .NET IDictionary.Values.
     * @return Heap-allocated ICollection; caller takes ownership.
     */
    [[nodiscard]] ICollection* getValuesProperty() const override { return new MemberCollection(this, false); }

    /**
     * @brief Determines whether the dictionary contains an element with the specified key.
     *
     * C++ counterpart of .NET IDictionary.Contains(object).
     * @param key Pointer used as the key (compared by address).
     */
    [[nodiscard]] bool Contains(const void* key) const override {
        for (const auto& n : list_) {
            if (n.key == key) return true;
        }
        return false;
    }

    /**
     * @brief Adds an element with the specified key and value to the dictionary.
     *
     * C++ counterpart of .NET IDictionary.Add(object, object?).
     * @throws System::ArgumentException if the key already exists, matching .NET.
     */
    void Add(const void* key, void* value) override {
        for (const auto& n : list_) {
            if (n.key == key) throw System::ArgumentException("Item has already been added.");
        }
        list_.push_back({key, value});
        ++version_;
    }

    /**
     * @brief Removes all elements from the dictionary.
     *
     * C++ counterpart of .NET IDictionary.Clear().
     */
    void Clear() override { list_.clear(); ++version_; }

    /**
     * @brief Removes the element with the specified key from the dictionary.
     *
     * C++ counterpart of .NET IDictionary.Remove(object).
     * @param key Pointer used as the key (compared by address).
     */
    void Remove(const void* key) override {
        size_t before = list_.size();
        list_.remove_if([key](const Node& n){ return n.key == key; });
        if (list_.size() != before) ++version_;
    }

    /**
     * @brief Returns an IDictionaryEnumerator for the dictionary.
     *
     * C++ counterpart of .NET IDictionary.GetEnumerator().
     * @return Heap-allocated IDictionaryEnumerator; caller takes ownership.
     */
    [[nodiscard]] IDictionaryEnumerator* GetEnumerator() override { return new NodeEnumerator(this); }
};

} // namespace System::Collections
