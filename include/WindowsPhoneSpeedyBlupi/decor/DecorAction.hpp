/**
 * @file DecorAction.hpp
 * @brief Defines the DecorAction enumeration and its comparison operators.
 *
 * @details
 * DecorAction represents a camera-shake animation that the game engine plays
 * on the background (decor) layer.  When a game event sets @c m_decorAction to
 * a non-None value, @c Decor::DecorNextAction() consults @c table_decor_action
 * to compute per-frame pixel offsets that jolt the viewport.  Once all frames
 * have been consumed the field is reset to @c DecorAction::None.
 *
 * The underlying type is @c SharpRuntime::ubytecs (unsigned byte), matching
 * the original C# enum layout so that values serialised in save files remain
 * compatible with the original Windows Phone game.
 *
 * @see Decor::DecorNextAction()
 * @see Tables::table_decor_action
 */

#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /// Underlying integer type used for DecorAction storage (unsigned byte, C#-compatible).
    using DecorActionUnderlying = SharpRuntime::ubytecs;

    /**
     * @enum DecorAction
     * @brief Identifies the camera-shake animation to play on the decor layer.
     *
     * @details
     * The engine keeps a single active action in @c Decor::m_decorAction.
     * Each non-None value selects a row in @c Tables::table_decor_action, which
     * encodes the number of frames and per-frame (dx, dy) offsets that are
     * multiplied by 3 to produce the final pixel displacement.
     *
     * @note Values 3 and 4 are unassigned in the original game; the sequence
     *       jumps directly to 5 for ElectricShake.
     * @note Stored as an unsigned byte (ubytecs) to preserve C# enum layout.
     * @see Decor::DecorNextAction()
     */
    enum class DecorAction : DecorActionUnderlying
    {
        None          = 0, ///< @brief No camera shake is active; the viewport is stationary.
        SmallShake    = 1, ///< @brief Triggered by minor impacts: a crate landing, a small explosion, or Blupi collecting a bonus item.
        BigShake      = 2, ///< @brief Triggered by major impacts: Blupi walking into a fan blade or a large explosion.
        ElectricShake = 5, ///< @brief Triggered when Blupi contacts an electric field (ObjectType90 spark object); produces a rapid jittery motion.
    };

    /**
     * @brief Converts a DecorAction enumerator to its raw underlying integer value.
     * @param[in] type The DecorAction value to convert.
     * @return The underlying unsigned-byte representation of @p type.
     */
    static constexpr auto ToRaw(DecorAction type) -> DecorActionUnderlying
    {
        return static_cast<DecorActionUnderlying>(type);
    }

    /**
     * @brief Constructs a DecorAction from an arbitrary integer value.
     *
     * @details
     * No range check is performed.  Values that do not correspond to a named
     * enumerator are still representable (the enum is not a closed set at the
     * language level), but @c DecorNextAction() will silently skip them because
     * @c table_decor_action has no matching entry.
     *
     * @param[in] value Integer to convert (typically read from a save file).
     * @return The DecorAction whose underlying value equals
     *         @c static_cast<DecorActionUnderlying>(value).
     */
    static constexpr auto ToDecorAction(const int value) -> DecorAction
    {
        return static_cast<DecorAction>(
            static_cast<DecorActionUnderlying>(value)
        );
    }

    /**
     * @brief Less-than comparison for two DecorAction values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is strictly less than that of @p rhs.
     */
    constexpr bool operator<(const DecorAction lhs, const DecorAction rhs) noexcept
    {
        return ToRaw(lhs) < ToRaw(rhs);
    }

    /**
     * @brief Greater-than comparison for two DecorAction values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is strictly greater than that of @p rhs.
     */
    constexpr bool operator>(const DecorAction lhs, const DecorAction rhs) noexcept
    {
        return ToRaw(lhs) > ToRaw(rhs);
    }

    /**
     * @brief Less-than-or-equal comparison for two DecorAction values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is less than or equal to that of @p rhs.
     */
    constexpr bool operator<=(const DecorAction lhs, const DecorAction rhs) noexcept
    {
        return ToRaw(lhs) <= ToRaw(rhs);
    }

    /**
     * @brief Greater-than-or-equal comparison for two DecorAction values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is greater than or equal to that of @p rhs.
     */
    constexpr bool operator>=(const DecorAction lhs, const DecorAction rhs) noexcept
    {
        return ToRaw(lhs) >= ToRaw(rhs);
    }
}
