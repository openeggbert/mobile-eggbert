// SPDX-License-Identifier: MS-PL
#pragma once

#include "Microsoft/Xna/Framework/Input/Buttons.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "CNA/CNAHelper.hpp"

#include <initializer_list>

namespace Microsoft::Xna::Framework::Input
{
    struct GamePadState;

    /**
     * @brief Represents the state of the digital buttons on a gamepad.
     */
    struct GamePadButtons
    {
        /**
         * @brief Gets the state of the Back button.
         * @return The Back button state.
         */
        [[nodiscard]] ButtonState getBackProperty() const;

        /** @brief Constructs with no buttons pressed. */
        NOXNA GamePadButtons();

        /**
         * @brief Constructs from a combined Buttons flags value.
         * @param buttons The combined button flags.
         */
        explicit GamePadButtons(Buttons buttons);

        /**
         * @brief Derives a GamePadButtons from a list of Buttons flags values.
         * @param btns The list of button flags to combine.
         * @return The resulting GamePadButtons.
         */
        NOXNA static GamePadButtons FromButtonArray(std::initializer_list<Buttons> btns);

        /**
         * @brief Compares this instance with another for equality.
         * @param other The other GamePadButtons to compare.
         * @return True if equal; false otherwise.
         */
        [[nodiscard]] bool Equals(const GamePadButtons& other) const;

        /**
         * @brief Gets the hash code for this instance.
         * @return Hash code of the object.
         */
        [[nodiscard]] int GetHashCode() const;

        /**
         * @brief Determines whether two GamePadButtons instances are equal.
         * @param left The first object to compare.
         * @param right The second object to compare.
         * @return True if equal; false otherwise.
         */
        friend bool operator==(const GamePadButtons& left, const GamePadButtons& right);

        /**
         * @brief Determines whether two GamePadButtons instances are not equal.
         * @param left The first object to compare.
         * @param right The second object to compare.
         * @return True if not equal; false otherwise.
         */
        friend bool operator!=(const GamePadButtons& left, const GamePadButtons& right);

    private:
        [[nodiscard]] ButtonState ButtonStateFromFlag(Buttons flag) const;

        /** @brief Packed button flags. For internal library use only. */
        NOXNA Buttons buttons_;

        friend struct GamePadState;
    };
}
