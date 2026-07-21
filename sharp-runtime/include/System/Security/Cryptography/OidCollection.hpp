// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Security/Cryptography/Oid.hpp"

namespace System::Security::Cryptography {

    /**
     * @brief A collection of Oid objects.
     *
     * C++ counterpart of .NET System.Security.Cryptography.OidCollection.
     *
     * @note The string indexer matches only against `Oid::getValueProperty()` exactly — .NET's
     * version first tries to resolve @p oid as a friendly name via `OidLookup`, which is out of
     * scope here (see `Oid`'s doc comment).
     */
    class OidCollection {
        std::vector<Oid> oids_;

    public:
        OidCollection() = default;

        /** @brief Adds @p oid to the collection. @return The index at which it was added. */
        SharpRuntime::intcs Add(const Oid& oid) {
            oids_.push_back(oid);
            return static_cast<SharpRuntime::intcs>(oids_.size() - 1);
        }

        /** @return The Oid at @p index. @throws System::ArgumentOutOfRangeException if out of range. */
        [[nodiscard]] const Oid& operator[](SharpRuntime::intcs index) const {
            if (index < 0 || static_cast<size_t>(index) >= oids_.size()) {
                throw System::ArgumentOutOfRangeException("index");
            }
            return oids_[static_cast<size_t>(index)];
        }

        /** @return The first Oid whose Value equals @p oid, or nullptr if none matches. */
        [[nodiscard]] const Oid* operator[](const std::string& oid) const {
            auto it = std::find_if(oids_.begin(), oids_.end(),
                                    [&](const Oid& entry) { return entry.getValueProperty() == oid; });
            return it == oids_.end() ? nullptr : &*it;
        }

        /** @return The number of Oid objects in the collection. */
        [[nodiscard]] SharpRuntime::intcs getCountProperty() const { return static_cast<SharpRuntime::intcs>(oids_.size()); }

        /**
         * @brief Copies the elements of the collection into @p array, starting at @p index.
         *
         * C++ counterpart of .NET OidCollection.CopyTo(Oid[], int).
         * @param array Destination vector; must already have room for index + Count elements.
         * @param index Zero-based starting index in @p array.
         * @throws System::ArgumentOutOfRangeException if @p index is negative or >= array.size()
         *         (matching .NET's own -- deliberately stricter than the usual "<=" bound --
         *         OidCollection.cs check; this means even an empty collection can't CopyTo an
         *         empty destination at index 0).
         * @throws System::ArgumentException if @p array does not have room for all elements
         *         starting at @p index.
         */
        void CopyTo(std::vector<Oid>& array, SharpRuntime::intcs index) const {
            if (index < 0 || static_cast<size_t>(index) >= array.size()) {
                throw System::ArgumentOutOfRangeException("index");
            }
            if (static_cast<size_t>(index) + oids_.size() > array.size()) {
                throw System::ArgumentException("Destination array was not long enough.");
            }
            std::copy(oids_.begin(), oids_.end(), array.begin() + index);
        }

        [[nodiscard]] auto begin() const { return oids_.begin(); }
        [[nodiscard]] auto end() const { return oids_.end(); }
    };

} // namespace System::Security::Cryptography
