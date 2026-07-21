// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <any>

namespace System {

    /**
     * @brief Encapsulates a memory slot to store local data.
     *
     * C++ counterpart of .NET System.LocalDataStoreSlot.
     * In .NET, LocalDataStoreSlot is used with Thread.AllocateDataSlot() and
     * Thread.GetData()/SetData() for thread-local storage.
     * In sharp-runtime the slot stores a single std::any value (not per-thread;
     * thread-local semantics require the caller to manage per-thread state).
     *
     * @note Status: Stub — thread-local per-slot semantics are not implemented.
     *       Use std::thread_local or Threading::ThreadLocal<T> for actual TLS.
     */
    class LocalDataStoreSlot final {
        std::any data_;

    public:
        /** @brief Constructs an empty LocalDataStoreSlot. */
        LocalDataStoreSlot() = default;

        /**
         * @brief Returns the data stored in this slot.
         *
         * C++ counterpart of reading back the value set via Thread.SetData.
         * @return The stored std::any value (empty if nothing has been set).
         */
        [[nodiscard]] const std::any& getData() const noexcept { return data_; }

        /**
         * @brief Stores a value in this slot.
         *
         * C++ counterpart of Thread.SetData(LocalDataStoreSlot, object).
         * @param value The value to store.
         */
        void setData(std::any value) noexcept { data_ = std::move(value); }

        /** @brief Returns true if a value has been stored in this slot. */
        [[nodiscard]] bool hasData() const noexcept { return data_.has_value(); }

        /** @brief Clears the stored value. */
        void clear() noexcept { data_.reset(); }
    };

} // namespace System
