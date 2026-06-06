/**
 * @file ContinueMission.hpp
 * @brief Defines the ContinueMissionType enumeration tracking the lifecycle of a continue-mission request.
 *
 * @details Used by Game1 to determine whether a "continue from checkpoint" transition is
 * absent, queued, or actively executing, without replaying the full level-start sequence.
 */

#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using ContinueMissionTypeUnderlying = SharpRuntime::ubytecs;

    /**
     * @brief State of a "continue mission" request in the game session.
     *
     * When the player returns to the main menu or loses a life, a continue-mission
     * request may be queued. This enum tracks whether such a request is absent,
     * waiting to be processed, or currently in progress.
     *
     * Used by Game1 to coordinate the transition from the main menu back into
     * an active level without replaying the full level-start sequence.
     *
     * @note This is game-session state, not level or save state.
     * @note Stored as an unsigned byte (ubytecs) to match the original C# enum layout.
     */
    enum class ContinueMissionType : SharpRuntime::ushortcs
    {
        None    = 0,   ///< @brief No continue-mission request is pending.
        Pending = 1,   ///< @brief A continue request has been issued but not yet acted on.
        Active  = 2    ///< @brief The continue sequence is currently executing.
    };

    /**
     * @brief Returns the raw underlying byte value of a ContinueMissionType.
     * @param[in] value The ContinueMissionType value to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(ContinueMissionType value) -> ContinueMissionTypeUnderlying
    {
        return static_cast<ContinueMissionTypeUnderlying>(value);
    }

    /**
     * @brief Converts an integer to a ContinueMissionType enum value.
     * @param[in] value Raw integer (must be 0, 1, or 2).
     * @return Corresponding ContinueMissionType enum value.
     */
    static constexpr auto ToContinueMissionType(const int value) -> ContinueMissionType
    {
        return static_cast<ContinueMissionType>(
            static_cast<ContinueMissionTypeUnderlying>(value)
        );
    }
}
