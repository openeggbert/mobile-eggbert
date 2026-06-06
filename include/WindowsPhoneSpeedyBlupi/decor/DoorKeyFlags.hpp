/**
 * @file DoorKeyFlags.hpp
 * @brief Defines the DoorKeyFlags bitmask enumeration and its bitwise operators.
 *
 * @details
 * DoorKeyFlags encodes which of the three collectible keys Blupi currently
 * carries.  The game stores the active key set in @c Decor::m_blupiCle and
 * gates locked doors by testing individual bits against the set.
 *
 * Door-unlock logic in @c Decor.cpp:
 * - When Blupi picks up a key object of type ObjectType49/50/51, the
 *   corresponding flag (Key1/Key2/Key3) is OR-ed into @c m_blupiCle.
 * - When Blupi uses a key to open a door, the matching flag is AND-masked out.
 * - A cheat / power-up path OR-es @c DoorKeyFlags::All into @c m_blupiCle to
 *   grant all three keys at once.
 * - The field is serialised to / deserialised from save files via
 *   @c Worlds::WriteIntField / @c Worlds::GetIntField.
 *
 * The underlying type is @c SharpRuntime::ubytecs (unsigned byte), matching
 * the original C# flag-enum layout.
 *
 * @see Decor::m_blupiCle
 * @see ObjectType (ObjectType49, ObjectType50, ObjectType51 are key pickup objects)
 */

#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /// Underlying integer type used for DoorKeyFlags storage (unsigned byte, C#-compatible).
    using DoorKeyFlagsUnderlying = SharpRuntime::ubytecs;

    /**
     * @enum DoorKeyFlags
     * @brief Bitmask of keys currently held by Blupi.
     *
     * @details
     * Each named flag occupies a distinct bit so that multiple keys can be
     * carried simultaneously.  The @c All flag is a convenience aggregate of
     * all three individual key bits.
     *
     * Typical usage:
     * @code
     * // Test whether Key2 is held:
     * if ((m_blupiCle & DoorKeyFlags::Key2) != DoorKeyFlags::None) { ... }
     *
     * // Grant all keys:
     * m_blupiCle = m_blupiCle | DoorKeyFlags::All;
     *
     * // Consume Key1 after unlocking a door:
     * m_blupiCle = ToDoorKeyFlags(ToRaw(m_blupiCle) & ~ToRaw(DoorKeyFlags::Key1));
     * @endcode
     *
     * @note Stored as an unsigned byte (ubytecs) to preserve C# flag-enum layout.
     * @see Decor::m_blupiCle
     */
    enum class DoorKeyFlags : DoorKeyFlagsUnderlying
    {
        None = 0,                  ///< @brief No keys are held; Blupi cannot open any locked door.
        Key1 = 1 << 0,             ///< @brief The first key (bit 0); collected via ObjectType49 and consumed to open the matching locked door.
        Key2 = 1 << 1,             ///< @brief The second key (bit 1); collected via ObjectType50 and consumed to open the matching locked door.
        Key3 = 1 << 2,             ///< @brief The third key (bit 2); collected via ObjectType51 and consumed to open the matching locked door.
        All  = Key1 | Key2 | Key3, ///< @brief All three keys simultaneously; used as a convenience mask or to grant every key at once.
    };

    /**
     * @brief Converts a DoorKeyFlags value to its raw underlying integer value.
     * @param[in] type The DoorKeyFlags value to convert.
     * @return The underlying unsigned-byte representation of @p type.
     */
    static constexpr auto ToRaw(DoorKeyFlags type) -> DoorKeyFlagsUnderlying
    {
        return static_cast<DoorKeyFlagsUnderlying>(type);
    }

    /**
     * @brief Constructs a DoorKeyFlags value from an arbitrary integer.
     *
     * @details
     * Typically used when deserialising the key-state field from a save file.
     * No range or validity check is performed.
     *
     * @param[in] value Integer to convert (e.g. a value read from a save file).
     * @return The DoorKeyFlags whose underlying value equals
     *         @c static_cast<DoorKeyFlagsUnderlying>(value).
     */
    static constexpr auto ToDoorKeyFlags(const int value) -> DoorKeyFlags
    {
        return static_cast<DoorKeyFlags>(
            static_cast<DoorKeyFlagsUnderlying>(value)
        );
    }

    /**
     * @brief Less-than comparison for two DoorKeyFlags values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is strictly less than that of @p rhs.
     */
    constexpr bool operator<(const DoorKeyFlags lhs, const DoorKeyFlags rhs) noexcept
    {
        return ToRaw(lhs) < ToRaw(rhs);
    }

    /**
     * @brief Greater-than comparison for two DoorKeyFlags values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is strictly greater than that of @p rhs.
     */
    constexpr bool operator>(const DoorKeyFlags lhs, const DoorKeyFlags rhs) noexcept
    {
        return ToRaw(lhs) > ToRaw(rhs);
    }

    /**
     * @brief Less-than-or-equal comparison for two DoorKeyFlags values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is less than or equal to that of @p rhs.
     */
    constexpr bool operator<=(const DoorKeyFlags lhs, const DoorKeyFlags rhs) noexcept
    {
        return ToRaw(lhs) <= ToRaw(rhs);
    }

    /**
     * @brief Greater-than-or-equal comparison for two DoorKeyFlags values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is greater than or equal to that of @p rhs.
     */
    constexpr bool operator>=(const DoorKeyFlags lhs, const DoorKeyFlags rhs) noexcept
    {
        return ToRaw(lhs) >= ToRaw(rhs);
    }

    /**
     * @brief Bitwise OR of two DoorKeyFlags values; combines key sets.
     *
     * @details
     * Used to add one or more keys to Blupi's inventory, e.g.:
     * @code
     * m_blupiCle = m_blupiCle | DoorKeyFlags::Key1;
     * @endcode
     *
     * @param[in] a First key set.
     * @param[in] b Second key set.
     * @return A DoorKeyFlags value with all bits set in either @p a or @p b.
     */
    constexpr DoorKeyFlags operator|(DoorKeyFlags a, DoorKeyFlags b)
    {
        return static_cast<DoorKeyFlags>(ToRaw(a) | ToRaw(b));
    }

    /**
     * @brief Bitwise AND of two DoorKeyFlags values; tests or masks key bits.
     *
     * @details
     * Used to test whether a specific key flag is present, e.g.:
     * @code
     * if ((m_blupiCle & DoorKeyFlags::Key2) != DoorKeyFlags::None) { ... }
     * @endcode
     *
     * @param[in] a First key set.
     * @param[in] b Second key set (typically a single-flag mask).
     * @return A DoorKeyFlags value with only the bits set in both @p a and @p b.
     */
    constexpr DoorKeyFlags operator&(DoorKeyFlags a, DoorKeyFlags b)
    {
        return static_cast<DoorKeyFlags>(ToRaw(a) & ToRaw(b));
    }
}
