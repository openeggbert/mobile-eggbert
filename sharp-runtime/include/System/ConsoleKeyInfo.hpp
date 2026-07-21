// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on .NET runtime API (MIT License, Copyright .NET Foundation and Contributors)
#pragma once
#include "System/ConsoleKey.hpp"
#include "System/ConsoleModifiers.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace System {

    /**
     * @brief Describes the console key that was pressed, including the character
     * represented by the key and the state of the modifier keys.
     *
     * C++ counterpart of .NET System.ConsoleKeyInfo.
     */
    struct ConsoleKeyInfo {
    private:
        char            keyChar_;
        ConsoleKey      key_;
        ConsoleModifiers mods_;

    public:
        /**
         * @brief Constructs a ConsoleKeyInfo with the specified key character, key, and modifiers.
         * @param keyChar  The Unicode character represented by the pressed key.
         * @param key      The ConsoleKey value corresponding to the pressed key.
         * @param shift    true if the SHIFT modifier was held.
         * @param alt      true if the ALT modifier was held.
         * @param control  true if the CTRL modifier was held.
         */
        ConsoleKeyInfo(char keyChar, ConsoleKey key, bool shift, bool alt, bool control)
            : keyChar_(keyChar), key_(key), mods_(ConsoleModifiers::None)
        {
            if (static_cast<int>(key) < 0 || static_cast<int>(key) > 255)
                throw ArgumentOutOfRangeException("key", "Console key values must be between 0 and 255 inclusive.");
            if (shift)   mods_ = mods_ | ConsoleModifiers::Shift;
            if (alt)     mods_ = mods_ | ConsoleModifiers::Alt;
            if (control) mods_ = mods_ | ConsoleModifiers::Control;
        }

        /** @brief Default constructor — represents a null key press (keyChar=0, key=None, no mods). */
        ConsoleKeyInfo()
            : keyChar_(0), key_(ConsoleKey::None), mods_(ConsoleModifiers::None) {}

        /**
         * @brief Gets the Unicode character represented by the pressed key.
         * @return The character.
         */
        [[nodiscard]] char getKeyCharProperty() const noexcept { return keyChar_; }

        /**
         * @brief Gets the ConsoleKey value corresponding to the pressed key.
         * @return The key.
         */
        [[nodiscard]] ConsoleKey getKeyProperty() const noexcept { return key_; }

        /**
         * @brief Gets the state of the modifier keys (Shift, Alt, Control) at the time of the key press.
         * @return Bitwise OR of active ConsoleModifiers values.
         */
        [[nodiscard]] ConsoleModifiers getModifiersProperty() const noexcept { return mods_; }

        /** @brief Returns true if both ConsoleKeyInfo values represent the same key press. */
        [[nodiscard]] bool operator==(const ConsoleKeyInfo& o) const noexcept {
            return keyChar_ == o.keyChar_ && key_ == o.key_ && mods_ == o.mods_;
        }
        /** @brief Returns true if the ConsoleKeyInfo values differ. */
        [[nodiscard]] bool operator!=(const ConsoleKeyInfo& o) const noexcept {
            return !(*this == o);
        }
    };

} // namespace System
