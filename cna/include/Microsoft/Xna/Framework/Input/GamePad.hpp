// SPDX-License-Identifier: MS-PL
#pragma once

#include "CNA/CNAHelper.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadDeadZone.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"

#include <cstdint>
#include <string>

namespace Microsoft::Xna::Framework::Input
{
    /**
     * @brief Provides access to gamepad state snapshots.
     */
    class GamePad
    {
    public:
        GamePad() = delete;

        /**
         * @brief Returns a snapshot of the current gamepad state using IndependentAxes dead zone.
         * @param playerIndex The player index to query.
         * @return The current gamepad state.
         */
        static GamePadState GetState(PlayerIndex playerIndex);

        /**
         * @brief Returns a snapshot of the current gamepad state using the specified dead zone mode.
         * @param playerIndex The player index to query.
         * @param deadZoneMode The dead zone processing mode to apply.
         * @return The current gamepad state.
         */
        static GamePadState GetState(PlayerIndex playerIndex, GamePadDeadZone deadZoneMode);

        /** @brief Left stick dead zone threshold (XInput-based). */
        NOXNA static constexpr float LeftDeadZone     = 7849.0f / 32768.0f;
        /** @brief Right stick dead zone threshold (XInput-based). */
        NOXNA static constexpr float RightDeadZone    = 8689.0f / 32768.0f;
        /** @brief Trigger pressed threshold (XInput-based). */
        NOXNA static constexpr float TriggerThreshold = 30.0f / 255.0f;

        /**
         * @brief Applies dead zone exclusion to a single axis value.
         * @param value The raw axis value.
         * @param deadZone The dead zone threshold.
         * @return The processed axis value.
         */
        NOXNA static float ExcludeAxisDeadZone(float value, float deadZone);
    };
}
