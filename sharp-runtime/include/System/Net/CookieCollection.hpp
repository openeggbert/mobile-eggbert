// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include <vector>
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Net/Cookie.hpp"

namespace System::Net {

    using SharpRuntime::intcs;

    /**
     * @brief Provides a collection container for instances of the Cookie class.
     *
     * C++ counterpart of .NET System.Net.CookieCollection. A thin ordered container --
     * real .NET's version-negotiation and dedup-by-identity semantics are simplified to a
     * flat, insertion-ordered `std::vector<Cookie>`.
     */
    class CookieCollection {
    public:
        CookieCollection() = default;

        /** @brief Adds a cookie to the collection. */
        void Add(const Cookie& cookie) { cookies_.push_back(cookie); }

        /** @brief Adds every cookie from another collection. */
        void Add(const CookieCollection& other) {
            cookies_.insert(cookies_.end(), other.cookies_.begin(), other.cookies_.end());
        }

        /** @brief Gets the number of cookies contained in the collection. */
        [[nodiscard]] intcs getCountProperty() const { return static_cast<intcs>(cookies_.size()); }

        /** @brief Gets whether the collection is empty. */
        [[nodiscard]] bool getIsEmptyProperty() const { return cookies_.empty(); }

        [[nodiscard]] Cookie&       operator[](intcs index)       { return cookies_[static_cast<size_t>(index)]; }
        [[nodiscard]] const Cookie& operator[](intcs index) const { return cookies_[static_cast<size_t>(index)]; }

        [[nodiscard]] std::vector<Cookie>::iterator       begin()       { return cookies_.begin(); }
        [[nodiscard]] std::vector<Cookie>::iterator       end()         { return cookies_.end(); }
        [[nodiscard]] std::vector<Cookie>::const_iterator begin() const { return cookies_.begin(); }
        [[nodiscard]] std::vector<Cookie>::const_iterator end()   const { return cookies_.end(); }

    private:
        std::vector<Cookie> cookies_;
    };

} // namespace System::Net
