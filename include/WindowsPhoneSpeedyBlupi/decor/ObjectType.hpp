/**
 * @file ObjectType.hpp
 * @brief Defines the ObjectType enumeration and its comparison operators.
 *
 * @details
 * ObjectType identifies every distinct species of moving object (MoveObject)
 * that the Decor engine can spawn, animate, and remove.  The values are the
 * numeric IDs inherited verbatim from the original Speedy Blupi / Windows Phone
 * game.  They must not be changed because they are stored in level data files
 * and must match the binary format expected by the original game engine.
 *
 * Each type controls three things inside @c Decor.cpp:
 *  1. Which animation table is chosen by @c MoveObjectStepIcon().
 *  2. How the object moves (patrol, seek, fixed) in @c MoveObjectStepLine().
 *  3. How the engine reacts when Blupi touches the object (collectible pickup,
 *     damage, vehicle boarding, etc.).
 *
 * IDs that are not mentioned anywhere in Decor.cpp (42–203 with many gaps) are
 * either unused in the current levels or are placeholders reserved for future
 * use.  Their semantics are unknown and they are documented below only as
 * "purpose unknown".
 *
 * @note This is gameplay/simulation data.  Do not confuse ObjectType values
 *       with sprite/icon indices or PixmapChannel values.
 * @note Stored as an unsigned byte (ubytecs) to match the original C# enum
 *       layout.
 * @see Decor::MoveObjectStepIcon()
 * @see Decor::MoveObjectStepLine()
 */

#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /// Underlying integer type used for ObjectType storage (unsigned byte, C#-compatible).
    using ObjectTypeUnderlying = SharpRuntime::ubytecs;

    /**
     * @enum ObjectType
     * @brief Identifies the type of a moving object in the level.
     *
     * @details
     * Each value corresponds to a specific object class: enemies, crates,
     * projectiles, collectibles, platform lifts, hazards, and special effects.
     * The type determines the animation table used, the collision behaviour, and
     * whether the object interacts with Blupi.
     *
     * Values are numeric IDs inherited from the original game.  The mapping
     * from numeric ID to game concept is partially documented via code
     * archaeology of Decor.cpp.  Do not assume a value's meaning without
     * checking the animation and collision code.
     *
     * Numeric IDs must not be changed because they are stored in level files
     * and must match the original game data format exactly.
     *
     * @note Stored as an unsigned byte (ubytecs) to preserve C# enum layout.
     * @see Decor::MoveObjectStepIcon()
     * @see Decor::MoveObjectStepLine()
     */
    enum class ObjectType : ObjectTypeUnderlying
    {
        // -----------------------------------------------------------------------
        // Sentinel / empty slot
        // -----------------------------------------------------------------------

        ObjectType0   =   0, ///< @brief Null / inactive slot; a MoveObject whose type is 0 is considered absent and is not drawn or updated.

        // -----------------------------------------------------------------------
        // Platform lifts (types 1, 47, 48)
        // Used by MoveObjectStepLine to carry Blupi along their trajectory.
        // -----------------------------------------------------------------------

        ObjectType1   =   1, ///< @brief Standard platform lift; moves Blupi along a waypoint path without a horizontal drift bonus.
        ObjectType47  =  47, ///< @brief Platform lift with a rightward carry bonus (+2 px/frame horizontal velocity added to Blupi when riding).
        ObjectType48  =  48, ///< @brief Platform lift with a leftward carry bonus (-2 px/frame horizontal velocity added to Blupi when riding).

        // -----------------------------------------------------------------------
        // Hazard enemies — horizontal patrol (types 2, 3, 96, 97)
        // -----------------------------------------------------------------------

        ObjectType2   =   2, ///< @brief Standard patrolling enemy (icon set 12–20 in Element channel); damages Blupi on contact.
        ObjectType3   =   3, ///< @brief Patrolling enemy variant (icon set 48–56 in Element channel); similar behaviour to ObjectType2.
        ObjectType96  =  96, ///< @brief Follow enemy variant 1 (table_follow1 animation); chases Blupi and damages on contact.
        ObjectType97  =  97, ///< @brief Follow enemy variant 2 (table_follow2 animation); tracks Blupi's exact position each frame.

        // -----------------------------------------------------------------------
        // Bulldozer enemy (type 4)
        // -----------------------------------------------------------------------

        ObjectType4   =   4, ///< @brief Bulldozer enemy; patrols horizontally using table_bulldozer animation tables, kills Blupi on contact.

        // -----------------------------------------------------------------------
        // Collectibles — treasure and life items (types 5, 6, 7, 21, 39)
        // -----------------------------------------------------------------------

        ObjectType5   =   5, ///< @brief Treasure collectible (icons 0–11 in Element channel); incrementing m_nbTresor when collected, also init object for Blupi avatar.
        ObjectType6   =   6, ///< @brief Extra-life egg collectible (icons 21–28 in Element channel); increments m_nbVies when collected.
        ObjectType7   =   7, ///< @brief Level-exit goal marker (icons 29–36 in Element channel); triggers the win sequence when Blupi touches it.
        ObjectType21  =  21, ///< @brief Secret-level exit goal marker (table_cle animation); sets m_bFoundCle and triggers the win sequence.
        ObjectType39  =  39, ///< @brief Collectible sparkle effect (table_tresortrack, 11 frames); spawned when a treasure or key is picked up, auto-expires.

        // -----------------------------------------------------------------------
        // Key collectibles (types 49, 50, 51)
        // Each key corresponds to a DoorKeyFlags bit.
        // -----------------------------------------------------------------------

        ObjectType49  =  49, ///< @brief Key 1 collectible (table_cle1 animation); sets DoorKeyFlags::Key1 in m_blupiCle when collected.
        ObjectType50  =  50, ///< @brief Key 2 collectible (table_cle2 animation); sets DoorKeyFlags::Key2 in m_blupiCle when collected.
        ObjectType51  =  51, ///< @brief Key 3 collectible (table_cle3 animation); sets DoorKeyFlags::Key3 in m_blupiCle when collected.

        // -----------------------------------------------------------------------
        // Power-up / vehicle collectibles (types 13, 19, 24, 25, 26, 28, 29, 30, 31, 40, 46, 55)
        // -----------------------------------------------------------------------

        ObjectType13  =  13, ///< @brief Helicopter pick-up object (icon 68, Element channel); boarding sets m_blupiHelico and removes the object.
        ObjectType19  =  19, ///< @brief Jeep vehicle pick-up (icon 89, Element channel); boarding sets m_blupiJeep.
        ObjectType24  =  24, ///< @brief Skate collectible (table_skate, 34-frame animation); initiates BlupiAction::TakeSkate when collected.
        ObjectType25  =  25, ///< @brief Shield power-up (table_shield, 16-frame animation); activates m_blupiShield for 100 ticks on pickup.
        ObjectType26  =  26, ///< @brief Suction-cup power-up (table_power, 8-frame animation); initiates BlupiAction::Sucette on pickup.
        ObjectType28  =  28, ///< @brief Tank vehicle pick-up (icon 167, Element channel); boarding sets m_blupiTank.
        ObjectType29  =  29, ///< @brief Bullet ammo pack (icon 177, Element channel); adds 10 bullets to m_blupiBullet on pickup.
        ObjectType30  =  30, ///< @brief Drink power-up (icon 178, Element channel); initiates BlupiAction::Drink on pickup.
        ObjectType31  =  31, ///< @brief Charge / cloud power-up (table_charge, 6-frame Object channel animation); activates m_blupiCloud for 100 ticks.
        ObjectType40  =  40, ///< @brief Mirror/invert power-up (table_invert, 20-frame animation); activates m_blupiInvert for 100 ticks.
        ObjectType46  =  46, ///< @brief Balloon vehicle pick-up (icon 208, Element channel); boarding sets m_blupiOver.
        ObjectType55  =  55, ///< @brief Dynamite stick pick-up (icon 252, Element channel); initiates BlupiAction::TakeDynamite on pickup.

        // -----------------------------------------------------------------------
        // Explosion / visual effects (types 8–11, 36, 37, 38, 41, 42, 53, 90–93, 98–100)
        // These are transient; they auto-expire when their animation finishes.
        // -----------------------------------------------------------------------

        ObjectType8   =   8, ///< @brief Primary explosion effect (table_explo1 in Explosion channel); spawned by dynamite and enemy death.
        ObjectType9   =   9, ///< @brief Secondary small explosion effect (table_explo2, 20 frames in Explosion channel); spawned by minor impacts.
        ObjectType10  =  10, ///< @brief Tertiary explosion effect (table_explo3, 20 frames in Explosion channel); spawned by enemy/projectile impact with Blupi.
        ObjectType11  =  11, ///< @brief Fan-hit shockwave effect (table_explo4, 9 frames in Explosion channel); spawned when Blupi is hit by a fan blade, triggers BigShake.
        ObjectType12  =  12, ///< @brief Purpose unknown; declared for completeness.
        ObjectType36  =  36, ///< @brief Pollution / cloud puff effect (table_pollution, 8 frames in Element channel); auto-expires after 16 ticks.
        ObjectType37  =  37, ///< @brief Clear / dissipate visual effect (table_clear, 70 frames in Element channel); auto-expires after 70 ticks.
        ObjectType38  =  38, ///< @brief Electric arc effect (table_electro, 90 frames); starts in Blupi1_12 channel then switches to Element channel.
        ObjectType41  =  41, ///< @brief Invert-start particle (table_invertstart, 8 frames in Element channel); spawned in a 4-direction burst when ObjectType40 is collected.
        ObjectType42  =  42, ///< @brief Invert-stop particle (table_invertstop, 8 frames in Element channel); spawned in a 4-direction burst when the invert power-up expires.
        ObjectType53  =  53, ///< @brief Tentacle hazard (table_tentacule, 45-frame Explosion channel animation, 90 ticks); also excluded from normal draw-collision pass.
        ObjectType90  =  90, ///< @brief Electric spark effect (table_explo5, 12 frames in Explosion channel); spawned on electric contact, triggers ElectricShake.
        ObjectType91  =  91, ///< @brief Small flash effect (table_explo6, 6 frames in Explosion channel); spawned by special projectile impact.
        ObjectType92  =  92, ///< @brief Long energy-arc effect (table_explo7, 128 frames in Explosion channel); spawned when Blupi uses a charged attack.
        ObjectType93  =  93, ///< @brief Tiny flash effect (table_explo8, 5 frames in Explosion channel).
        ObjectType98  =  98, ///< @brief Water splash variant 1 (table_sploutch1, 10 frames in Explosion channel); spawned when entering water.
        ObjectType99  =  99, ///< @brief Water splash variant 2 (table_sploutch2, 13 frames in Explosion channel); spawned by a larger water entry.
        ObjectType100 = 100, ///< @brief Water splash variant 3 (table_sploutch3, 18 frames in Explosion channel); spawned by the largest water entry.

        // -----------------------------------------------------------------------
        // Water / goo effects (types 14, 15, 34, 35)
        // -----------------------------------------------------------------------

        ObjectType14  =  14, ///< @brief Water plouf splash (table_plouf, 7 frames in Object channel); spawned when Blupi falls into water.
        ObjectType15  =  15, ///< @brief Water bubble / blup (table_blup, 20 frames in Object channel); rising bubble animation, removes itself when it reaches its waypoint.
        ObjectType34  =  34, ///< @brief Goo / glue particle (table_glu, 25-frame looping Element animation); sticks to the level geometry.
        ObjectType35  =  35, ///< @brief Small plouf splash (table_tiplouf, 7 frames in Object channel); spawned by smaller water impacts.

        // -----------------------------------------------------------------------
        // Projectiles (type 23)
        // -----------------------------------------------------------------------

        ObjectType23  =  23, ///< @brief Fired projectile (icon 176, Element channel); spawned by blupih / blupit enemies and travels toward Blupi; removes itself on timeout (phase >= 55).

        // -----------------------------------------------------------------------
        // Enemies — patrol walkers (types 16, 17, 18, 20, 32, 33, 44, 54)
        // -----------------------------------------------------------------------

        ObjectType16  =  16, ///< @brief Spider/arthropod enemy (icons 69–77 in Element channel); standard patrol walker, damages Blupi on contact.
        ObjectType17  =  17, ///< @brief Fish enemy (table_poisson animation); horizontal patrol walker in water sections.
        ObjectType18  =  18, ///< @brief Additional patrol enemy variant; exact sprite unknown; destroyed by dynamite like other patrol enemies.
        ObjectType20  =  20, ///< @brief Bird enemy (table_oiseau animation); horizontal patrol walker, damages Blupi on contact.
        ObjectType32  =  32, ///< @brief Blupi-hostile clone "blupih" (table_blupih animation); patrol enemy that fires ObjectType23 projectiles during its turn animation.
        ObjectType33  =  33, ///< @brief Blupi-hostile clone "blupit" (table_blupit animation); fires two ObjectType23 projectiles per turn, one at phase 3 and one at phase 21.
        ObjectType44  =  44, ///< @brief Wasp / bee enemy (table_guepe animation); fast horizontal patrol, damages Blupi on contact.
        ObjectType54  =  54, ///< @brief Large creature enemy (table_creature animation, 152-frame turn); slow patrol, destroys Blupi's helicopter on contact.

        // -----------------------------------------------------------------------
        // Moving decoration / level objects (types 22, 27, 52, 56, 57, 58)
        // -----------------------------------------------------------------------

        ObjectType22  =  22, ///< @brief Animated door opening sequence; spawned by Decor::OpenDoor(), removes itself when animation phase reaches step 3.
        ObjectType27  =  27, ///< @brief Magic track sparkle trail (table_magictrack, 24 frames in Element channel); auto-expires after 24 ticks.
        ObjectType52  =  52, ///< @brief Bridge construction animation (table_bridge, 157 frames in Object channel); simultaneously updates the static decor tile at its start position.
        ObjectType56  =  56, ///< @brief Dynamite fuse animation (table_dynamitef, 100 frames in Element channel); triggers multiple DynamiteStart() blasts between phases 50–69.
        ObjectType57  =  57, ///< @brief Shield trail sparkle (table_shieldtrack, 20 frames in Element channel); auto-expires after 20 ticks.
        ObjectType58  =  58, ///< @brief Shield disappear effect; transitions from shield state; auto-expires after 20 ticks.

        // -----------------------------------------------------------------------
        // Blupi avatar colour variants (types 200–203)
        // Used to display alternate skins for multi-player or costume selection.
        // -----------------------------------------------------------------------

        ObjectType200 = 200, ///< @brief Blupi avatar — default skin (icons 257–262 on PixmapChannel::Blupi); also acts as a costume-select pickup that triggers the player-select voyage when touched.
        ObjectType201 = 201, ///< @brief Blupi avatar — skin variant 1 (icons 257–262 on PixmapChannel::Blupi1_11); damages Blupi on contact if shield/hide/SuperBlupi are inactive.
        ObjectType202 = 202, ///< @brief Blupi avatar — skin variant 2 (icons 257–262 on PixmapChannel::Blupi1_12); damages Blupi on contact if shield/hide/SuperBlupi are inactive.
        ObjectType203 = 203, ///< @brief Blupi avatar — skin variant 3 (icons 257–262 on PixmapChannel::Blupi1_13); damages Blupi on contact if shield/hide/SuperBlupi are inactive.

        // -----------------------------------------------------------------------
        // Unidentified / reserved entries (43, 45, 59–89, 94–95, 101–199)
        // The values below are declared to make the enum contiguous so that
        // level-file round-trips are lossless, but their game semantics are
        // currently unknown.  Do not use them without first investigating
        // all usages of MoveObject::type in Decor.cpp.
        // -----------------------------------------------------------------------

        ObjectType43  =  43, ///< @brief Purpose unknown; declared for completeness.
        ObjectType45  =  45, ///< @brief Purpose unknown; declared for completeness.
        ObjectType59  =  59, ///< @brief Purpose unknown; declared for completeness.
        ObjectType60  =  60, ///< @brief Purpose unknown; declared for completeness.
        ObjectType61  =  61, ///< @brief Purpose unknown; declared for completeness.
        ObjectType62  =  62, ///< @brief Purpose unknown; declared for completeness.
        ObjectType63  =  63, ///< @brief Purpose unknown; declared for completeness.
        ObjectType64  =  64, ///< @brief Purpose unknown; declared for completeness.
        ObjectType65  =  65, ///< @brief Purpose unknown; declared for completeness.
        ObjectType66  =  66, ///< @brief Purpose unknown; declared for completeness.
        ObjectType67  =  67, ///< @brief Purpose unknown; declared for completeness.
        ObjectType68  =  68, ///< @brief Purpose unknown; declared for completeness.
        ObjectType69  =  69, ///< @brief Purpose unknown; declared for completeness.
        ObjectType70  =  70, ///< @brief Purpose unknown; declared for completeness.
        ObjectType71  =  71, ///< @brief Purpose unknown; declared for completeness.
        ObjectType72  =  72, ///< @brief Purpose unknown; declared for completeness.
        ObjectType73  =  73, ///< @brief Purpose unknown; declared for completeness.
        ObjectType74  =  74, ///< @brief Purpose unknown; declared for completeness.
        ObjectType75  =  75, ///< @brief Purpose unknown; declared for completeness.
        ObjectType76  =  76, ///< @brief Purpose unknown; declared for completeness.
        ObjectType77  =  77, ///< @brief Purpose unknown; declared for completeness.
        ObjectType78  =  78, ///< @brief Purpose unknown; declared for completeness.
        ObjectType79  =  79, ///< @brief Purpose unknown; declared for completeness.
        ObjectType80  =  80, ///< @brief Purpose unknown; declared for completeness.
        ObjectType81  =  81, ///< @brief Purpose unknown; declared for completeness.
        ObjectType82  =  82, ///< @brief Purpose unknown; declared for completeness.
        ObjectType83  =  83, ///< @brief Purpose unknown; declared for completeness.
        ObjectType84  =  84, ///< @brief Purpose unknown; declared for completeness.
        ObjectType85  =  85, ///< @brief Purpose unknown; declared for completeness.
        ObjectType86  =  86, ///< @brief Purpose unknown; declared for completeness.
        ObjectType87  =  87, ///< @brief Purpose unknown; declared for completeness.
        ObjectType88  =  88, ///< @brief Purpose unknown; declared for completeness.
        ObjectType89  =  89, ///< @brief Purpose unknown; declared for completeness.
        ObjectType94  =  94, ///< @brief Purpose unknown; declared for completeness.
        ObjectType95  =  95, ///< @brief Purpose unknown; declared for completeness.
        ObjectType101 = 101, ///< @brief Purpose unknown; declared for completeness.
        ObjectType102 = 102, ///< @brief Purpose unknown; declared for completeness.
        ObjectType103 = 103, ///< @brief Purpose unknown; declared for completeness.
        ObjectType104 = 104, ///< @brief Purpose unknown; declared for completeness.
        ObjectType105 = 105, ///< @brief Purpose unknown; declared for completeness.
        ObjectType106 = 106, ///< @brief Purpose unknown; declared for completeness.
        ObjectType107 = 107, ///< @brief Purpose unknown; declared for completeness.
        ObjectType108 = 108, ///< @brief Purpose unknown; declared for completeness.
        ObjectType109 = 109, ///< @brief Purpose unknown; declared for completeness.
        ObjectType110 = 110, ///< @brief Purpose unknown; declared for completeness.
        ObjectType111 = 111, ///< @brief Purpose unknown; declared for completeness.
        ObjectType112 = 112, ///< @brief Purpose unknown; declared for completeness.
        ObjectType113 = 113, ///< @brief Purpose unknown; declared for completeness.
        ObjectType114 = 114, ///< @brief Purpose unknown; declared for completeness.
        ObjectType115 = 115, ///< @brief Purpose unknown; declared for completeness.
        ObjectType116 = 116, ///< @brief Purpose unknown; declared for completeness.
        ObjectType117 = 117, ///< @brief Purpose unknown; declared for completeness.
        ObjectType118 = 118, ///< @brief Purpose unknown; declared for completeness.
        ObjectType119 = 119, ///< @brief Purpose unknown; declared for completeness.
        ObjectType120 = 120, ///< @brief Purpose unknown; declared for completeness.
        ObjectType121 = 121, ///< @brief Purpose unknown; declared for completeness.
        ObjectType122 = 122, ///< @brief Purpose unknown; declared for completeness.
        ObjectType123 = 123, ///< @brief Purpose unknown; declared for completeness.
        ObjectType124 = 124, ///< @brief Purpose unknown; declared for completeness.
        ObjectType125 = 125, ///< @brief Purpose unknown; declared for completeness.
        ObjectType126 = 126, ///< @brief Purpose unknown; declared for completeness.
        ObjectType127 = 127, ///< @brief Purpose unknown; declared for completeness.
        ObjectType128 = 128, ///< @brief Purpose unknown; declared for completeness.
        ObjectType129 = 129, ///< @brief Purpose unknown; declared for completeness.
        ObjectType130 = 130, ///< @brief Purpose unknown; declared for completeness.
        ObjectType131 = 131, ///< @brief Purpose unknown; declared for completeness.
        ObjectType132 = 132, ///< @brief Purpose unknown; declared for completeness.
        ObjectType133 = 133, ///< @brief Purpose unknown; declared for completeness.
        ObjectType134 = 134, ///< @brief Purpose unknown; declared for completeness.
        ObjectType135 = 135, ///< @brief Purpose unknown; declared for completeness.
        ObjectType136 = 136, ///< @brief Purpose unknown; declared for completeness.
        ObjectType137 = 137, ///< @brief Purpose unknown; declared for completeness.
        ObjectType138 = 138, ///< @brief Purpose unknown; declared for completeness.
        ObjectType139 = 139, ///< @brief Purpose unknown; declared for completeness.
        ObjectType140 = 140, ///< @brief Purpose unknown; declared for completeness.
        ObjectType141 = 141, ///< @brief Purpose unknown; declared for completeness.
        ObjectType142 = 142, ///< @brief Purpose unknown; declared for completeness.
        ObjectType143 = 143, ///< @brief Purpose unknown; declared for completeness.
        ObjectType144 = 144, ///< @brief Purpose unknown; declared for completeness.
        ObjectType145 = 145, ///< @brief Purpose unknown; declared for completeness.
        ObjectType146 = 146, ///< @brief Purpose unknown; declared for completeness.
        ObjectType147 = 147, ///< @brief Purpose unknown; declared for completeness.
        ObjectType148 = 148, ///< @brief Purpose unknown; declared for completeness.
        ObjectType149 = 149, ///< @brief Purpose unknown; declared for completeness.
        ObjectType150 = 150, ///< @brief Purpose unknown; declared for completeness.
        ObjectType151 = 151, ///< @brief Purpose unknown; declared for completeness.
        ObjectType152 = 152, ///< @brief Purpose unknown; declared for completeness.
        ObjectType153 = 153, ///< @brief Purpose unknown; declared for completeness.
        ObjectType154 = 154, ///< @brief Purpose unknown; declared for completeness.
        ObjectType155 = 155, ///< @brief Purpose unknown; declared for completeness.
        ObjectType156 = 156, ///< @brief Purpose unknown; declared for completeness.
        ObjectType157 = 157, ///< @brief Purpose unknown; declared for completeness.
        ObjectType158 = 158, ///< @brief Purpose unknown; declared for completeness.
        ObjectType159 = 159, ///< @brief Purpose unknown; declared for completeness.
        ObjectType160 = 160, ///< @brief Purpose unknown; declared for completeness.
        ObjectType161 = 161, ///< @brief Purpose unknown; declared for completeness.
        ObjectType162 = 162, ///< @brief Purpose unknown; declared for completeness.
        ObjectType163 = 163, ///< @brief Purpose unknown; declared for completeness.
        ObjectType164 = 164, ///< @brief Purpose unknown; declared for completeness.
        ObjectType165 = 165, ///< @brief Purpose unknown; declared for completeness.
        ObjectType166 = 166, ///< @brief Purpose unknown; declared for completeness.
        ObjectType167 = 167, ///< @brief Purpose unknown; declared for completeness.
        ObjectType168 = 168, ///< @brief Purpose unknown; declared for completeness.
        ObjectType169 = 169, ///< @brief Purpose unknown; declared for completeness.
        ObjectType170 = 170, ///< @brief Purpose unknown; declared for completeness.
        ObjectType171 = 171, ///< @brief Purpose unknown; declared for completeness.
        ObjectType172 = 172, ///< @brief Purpose unknown; declared for completeness.
        ObjectType173 = 173, ///< @brief Purpose unknown; declared for completeness.
        ObjectType174 = 174, ///< @brief Purpose unknown; declared for completeness.
        ObjectType175 = 175, ///< @brief Purpose unknown; declared for completeness.
        ObjectType176 = 176, ///< @brief Purpose unknown; declared for completeness.
        ObjectType177 = 177, ///< @brief Purpose unknown; declared for completeness.
        ObjectType178 = 178, ///< @brief Purpose unknown; declared for completeness.
        ObjectType179 = 179, ///< @brief Purpose unknown; declared for completeness.
        ObjectType180 = 180, ///< @brief Purpose unknown; declared for completeness.
        ObjectType181 = 181, ///< @brief Purpose unknown; declared for completeness.
        ObjectType182 = 182, ///< @brief Purpose unknown; declared for completeness.
        ObjectType183 = 183, ///< @brief Purpose unknown; declared for completeness.
        ObjectType184 = 184, ///< @brief Purpose unknown; declared for completeness.
        ObjectType185 = 185, ///< @brief Purpose unknown; declared for completeness.
        ObjectType186 = 186, ///< @brief Purpose unknown; declared for completeness.
        ObjectType187 = 187, ///< @brief Purpose unknown; declared for completeness.
        ObjectType188 = 188, ///< @brief Purpose unknown; declared for completeness.
        ObjectType189 = 189, ///< @brief Purpose unknown; declared for completeness.
        ObjectType190 = 190, ///< @brief Purpose unknown; declared for completeness.
        ObjectType191 = 191, ///< @brief Purpose unknown; declared for completeness.
        ObjectType192 = 192, ///< @brief Purpose unknown; declared for completeness.
        ObjectType193 = 193, ///< @brief Purpose unknown; declared for completeness.
        ObjectType194 = 194, ///< @brief Purpose unknown; declared for completeness.
        ObjectType195 = 195, ///< @brief Purpose unknown; declared for completeness.
        ObjectType196 = 196, ///< @brief Purpose unknown; declared for completeness.
        ObjectType197 = 197, ///< @brief Purpose unknown; declared for completeness.
        ObjectType198 = 198, ///< @brief Purpose unknown; declared for completeness.
        ObjectType199 = 199  ///< @brief Purpose unknown; declared for completeness.

        // TODO: Replace magic numeric object type IDs with named enum values.
        // Investigate all assignments and comparisons of MoveObject::type in Decor.cpp.
        // Known so far:
        //   6 = life egg / extra life collectible.
        // This value uses channel 10 and icons 21..28 in MoveObjectStepIcon().
        // When collected, it triggers VoyageInit(..., 21, 10) and later increments m_nbVies.
        // Keep the original numeric IDs to preserve compatibility with the original game data.
    };

    /**
     * @brief Converts an ObjectType enumerator to its raw underlying integer value.
     * @param[in] type The ObjectType value to convert.
     * @return The underlying unsigned-byte representation of @p type.
     */
    static constexpr auto ToRaw(ObjectType type) -> ObjectTypeUnderlying
    {
        return static_cast<ObjectTypeUnderlying>(type);
    }

    /**
     * @brief Constructs an ObjectType from an arbitrary integer value.
     *
     * @details
     * Typically used when deserialising object data from a level file.  No
     * range or validity check is performed; an out-of-range value is silently
     * stored and may cause undefined animation behaviour at runtime.
     *
     * @param[in] value Integer to convert (e.g. a value read from a level file).
     * @return The ObjectType whose underlying value equals
     *         @c static_cast<ObjectTypeUnderlying>(value).
     */
    static constexpr auto ToObjectType(const int value) -> ObjectType
    {
        return static_cast<ObjectType>(
            static_cast<ObjectTypeUnderlying>(value)
        );
    }

    /**
     * @brief Less-than comparison for two ObjectType values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is strictly less than that of @p rhs.
     */
    constexpr bool operator<(const ObjectType lhs, const ObjectType rhs) noexcept
    {
        return ToRaw(lhs) < ToRaw(rhs);
    }

    /**
     * @brief Greater-than comparison for two ObjectType values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is strictly greater than that of @p rhs.
     */
    constexpr bool operator>(const ObjectType lhs, const ObjectType rhs) noexcept
    {
        return ToRaw(lhs) > ToRaw(rhs);
    }

    /**
     * @brief Less-than-or-equal comparison for two ObjectType values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is less than or equal to that of @p rhs.
     */
    constexpr bool operator<=(const ObjectType lhs, const ObjectType rhs) noexcept
    {
        return ToRaw(lhs) <= ToRaw(rhs);
    }

    /**
     * @brief Greater-than-or-equal comparison for two ObjectType values.
     * @param[in] lhs Left-hand operand.
     * @param[in] rhs Right-hand operand.
     * @return @c true if the raw value of @p lhs is greater than or equal to that of @p rhs.
     */
    constexpr bool operator>=(const ObjectType lhs, const ObjectType rhs) noexcept
    {
        return ToRaw(lhs) >= ToRaw(rhs);
    }
}
