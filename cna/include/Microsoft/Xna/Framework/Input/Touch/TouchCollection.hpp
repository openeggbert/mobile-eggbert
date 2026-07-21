// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.hpp"

#include <cstddef>
#include <vector>

namespace Microsoft::Xna::Framework::Input::Touch
{
    /**
     * @brief Provides an immutable snapshot of current touch locations.
     */
    struct TouchCollection
    {
        /**
         * @brief Gets the number of touch locations in this collection.
         * @return The touch location count.
         */
        [[nodiscard]] int getCountProperty() const;

        /**
         * @brief Gets whether a touch device is connected.
         * @return True if connected; false otherwise.
         */
        [[nodiscard]] bool getIsConnectedProperty() const;

        /**
         * @brief Constructs an empty touch collection.
         * @note NOXNA — FNA's `TouchCollection` has no explicit parameterless constructor
         *       (only the one taking a touch array).
         */
        NOXNA TouchCollection();

        /**
         * @brief Constructs from a vector of touch locations.
         * @param touches The touch locations to include.
         */
        explicit TouchCollection(const std::vector<TouchLocation>& touches);

        /**
         * @brief Constructs by moving a vector of touch locations.
         * @param touches The touch locations to move in.
         */
        explicit TouchCollection(std::vector<TouchLocation>&& touches);

        /**
         * @brief Returns the touch location at the given index (mutable overload).
         *
         * Mirrors FNA's settable `this[int]` indexer; since the collection is never
         * actually read-only in this implementation (unlike FNA's default-constructed,
         * null-backed struct), assignment is never blocked purely because IsReadOnly is
         * true. Assignment still requires an in-range index — out-of-range access throws
         * std::out_of_range, matching FNA's indexer throwing for a bad/null-backed index.
         *
         * @param index The zero-based index to retrieve.
         * @return A reference to the touch location.
         */
        [[nodiscard]] TouchLocation& operator[](std::size_t index);

        /**
         * @brief Returns the touch location at the given index.
         * @param index The zero-based index to retrieve.
         * @return A const reference to the touch location.
         */
        [[nodiscard]] const TouchLocation& operator[](std::size_t index) const;

        /**
         * @brief Returns a mutable iterator to the beginning of the collection.
         * @note NOXNA — replaces FNA's `IEnumerable<TouchLocation>::GetEnumerator()`.
         */
        NOXNA std::vector<TouchLocation>::iterator begin();
        /** @brief Returns a mutable iterator past the end of the collection. */
        NOXNA std::vector<TouchLocation>::iterator end();
        /** @brief Returns a const iterator to the beginning of the collection. */
        NOXNA [[nodiscard]] std::vector<TouchLocation>::const_iterator begin() const;
        /** @brief Returns a const iterator past the end of the collection. */
        NOXNA [[nodiscard]] std::vector<TouchLocation>::const_iterator end() const;

    private:
        std::vector<TouchLocation> touches_;
    };
}
