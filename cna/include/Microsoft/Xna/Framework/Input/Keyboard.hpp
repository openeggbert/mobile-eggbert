// SPDX-License-Identifier: MS-PL
#pragma once

#include "KeyboardState.hpp"
#include "Keys.hpp"
#include "CNA/CNAHelper.hpp"

namespace Microsoft::Xna::Framework::Input
{
    /**
     * @brief Allows getting keystrokes from keyboard.
     */
    class Keyboard
    {
    public:
        /** @brief Keyboard is a static class (XNA `public static class Keyboard`) and cannot be instantiated. */
        Keyboard() = delete;

        /**
         * @brief Returns the current keyboard state.
         * @return The current keyboard state.
         */
        static KeyboardState GetState();
    };
}
