// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Input/Buttons.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"

#include <initializer_list>

namespace Microsoft::Xna::Framework::Input
{
    struct GamePadState;

    /**
     * @brief Represents the state of the directional pad on a gamepad.
     */
    struct GamePadDPad
    {
        /** @brief Constructs a GamePadDPad with all directions released. */
        NOXNA GamePadDPad();

        /**
         * @brief Constructs a GamePadDPad with explicit direction states.
         * @param upValue The up direction state.
         * @param downValue The down direction state.
         * @param leftValue The left direction state.
         * @param rightValue The right direction state.
         */
        GamePadDPad(ButtonState upValue, ButtonState downValue,
                    ButtonState leftValue, ButtonState rightValue);

        /**
         * @brief Derives a GamePadDPad from a list of Buttons flags values.
         * @param buttons The list of button flags to combine.
         * @return The resulting GamePadDPad.
         */
        NOXNA static GamePadDPad FromButtonArray(std::initializer_list<Buttons> buttons);

        /**
         * @brief Compares this instance with another for equality.
         * @param other The other GamePadDPad to compare.
         * @return True if equal; false otherwise.
         */
        [[nodiscard]] bool Equals(const GamePadDPad& other) const;

        /**
         * @brief Gets the hash code for this instance.
         * @return Hash code of the object.
         */
        [[nodiscard]] int GetHashCode() const;

        /**
         * @brief Compares two GamePadDPad instances for equality.
         * @param left The left-hand operand.
         * @param right The right-hand operand.
         * @return True if equal; false otherwise.
         */
        friend bool operator==(const GamePadDPad& left, const GamePadDPad& right);

        /**
         * @brief Compares two GamePadDPad instances for inequality.
         * @param left The left-hand operand.
         * @param right The right-hand operand.
         * @return True if not equal; false otherwise.
         */
        friend bool operator!=(const GamePadDPad& left, const GamePadDPad& right);

    private:
        ButtonState down_;
        ButtonState left_;
        ButtonState right_;
        ButtonState up_;
    };
}
