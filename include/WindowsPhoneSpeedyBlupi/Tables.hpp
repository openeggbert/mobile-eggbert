/**
 * @file Tables.hpp
 * @brief Declaration of the Tables class containing all static game data tables.
 *
 * @details This header declares the Tables utility class for the C++ port of
 * Speedy Blupi (originally XNA / Windows Phone C#).  Every array declared here
 * is a direct, value-for-value port of the original C# source data and must not
 * be altered without a full audit of every index site in the codebase.
 *
 * The class is non-instantiable (deleted constructor and destructor) and acts
 * purely as a namespace-scoped data container.  All members are @c static.
 *
 * Broad table categories:
 *  - **Player / NPC animation**: @c table_blupi, @c table_mirror, vehicle and
 *    creature tables.  Each entry is typically a flat sprite-index sequence
 *    consumed one entry per game tick by a phase counter.
 *  - **Speed curves**: @c table_vitesse_march, @c table_vitesse_nage,
 *    @c table_vitesse_surf — per-phase pixel-per-tick movement values.
 *  - **Tile adaptation**: @c table_decor_quart (7056 entries) drives the
 *    quarter-tile corner-blending system; @c table_adapt_decor and
 *    @c table_adapt_fromage map neighbour bitmasks to replacement tile IDs.
 *  - **Effects / hazards**: explosion, splash, electricity, lava, trap, fan,
 *    and environmental animation tables.
 *  - **Power-ups / items**: shield, power-charge, skateboard, key, dynamite,
 *    magic-loop, drink-effect, and treasure-track tables.
 *  - **Training levels**: @c table_training1..4 are the only *mutable* tables;
 *    Init() patches localised text-resource IDs into index slot 5 of every
 *    6-element record.
 *  - **Misc**: @c world_terminal (end-of-game descriptor),
 *    @c table_decor_action (scripted tile-motion offsets),
 *    @c table_explo_size (per-channel bounding box widths).
 */
#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SharpRuntime::shortcs;

    /**
     * @class Tables
     * @brief Static repository of all original game data tables.
     *
     * @details Tables contains the animation data, movement trajectories, sprite
     * sequences, environment/hazard data, and other lookup tables ported directly
     * from the original C# Speedy Blupi game.  All values in these arrays come
     * from the original C# source and must not be changed without fully
     * understanding the animation and gameplay logic that indexes into them.
     *
     * Table format notes:
     * - Animation tables (e.g., @c table_blupi, @c table_bulldozer_*) typically
     *   contain flat sequences of sprite icon indices consumed one entry per game
     *   tick by a phase counter.
     * - @c table_decor_quart drives the quarter-tile tile-adaptation system; each
     *   group of 12 values corresponds to one neighbour-bitmask combination.
     * - @c table_decor_action drives animated tile sprites with per-frame (dx, dy)
     *   displacement pairs.
     * - @c table_explo_size provides per-channel explosion bounding-box widths.
     *
     * @note Do not renumber, reorder, or resize any table without updating all code
     *       that indexes into it.  Array sizes are part of the original game's data
     *       contract.
     * @note @c table_training1..4 are the only mutable tables (modified by Init()
     *       based on the player's current platform/mode).  All others are @c const.
     * @note This is data, not gameplay logic.  Tables does not own any runtime state.
     */
    class Tables
    {
    public:
        Tables() = delete;  ///< Non-instantiable utility class.
        ~Tables() = delete; ///< Non-instantiable utility class.

        /**
         * @brief Cheat code identifiers recognised by Decor::CheatAction().
         *
         * Each value activates a specific debug/cheat effect in the current level.
         * Cheats are entered via the cheat button gesture sequence defined in Game1.
         */
        enum class CheatCodes
        {
            BuildOfficialMissions, ///< Toggle official mission builder mode.
            OpenDoors,             ///< Open all doors in the current level.
            CleanAll,              ///< Remove all objects from the level.
            SuperBlupi,            ///< Enable Super Blupi enhanced mode.
            LayEgg,                ///< Place an egg in front of Blupi.
            KillEgg,               ///< Remove the nearest egg.
            Skate,                 ///< Give Blupi the skateboard.
            Copter,                ///< Give Blupi the helicopter.
            Jeep,                  ///< Give Blupi the jeep.
            AllTreasure,           ///< Collect all treasures instantly.
            EndGoal,               ///< Trigger the level win condition.
            ShowSecret,            ///< Reveal secret paths.
            RoundShield,           ///< Activate the Shield power-up.
            Lollipop,              ///< Activate the lollipop power-up.
            Bombs,                 ///< Give Blupi a supply of dynamite.
            BirdLime,              ///< Activate bird-lime/glue effect.
            Tank,                  ///< Give Blupi the tank vehicle.
            PowerCharge,           ///< Activate the Power charge.
            Drink,                 ///< Trigger the drink animation.
            Overcraft,             ///< Give Blupi the overcraft.
            Dynamite,              ///< Give Blupi extra dynamite.
            WeelKeys               ///< Give Blupi a set of keys.
#ifndef LEGACY
            ,Quick                 ///< The game speed can be switched to 2x, 4x or 8x
#endif
#ifdef MODERN
            ,Ghost                 ///< Ghost mode: semi-transparent, free flight, no interactions.
            ,Debug                 ///< Debug overlay: shows runtime state in top-right corner.
            ,Zoom                  ///< Zoom cheat: cycles through zoom-out levels (100%, 25%, 50%).
            ,Cheats                ///< Cheats overlay: shows list of all cheats for 5 seconds.
#endif
        };

        // -----------------------------------------------------------------------
        // Player animation
        // -----------------------------------------------------------------------

        /**
         * @brief Blupi player animation table.
         *
         * @details Contains a flat sequence of sprite icon indices for every
         * player action / phase combination.  The layout within the array is
         * defined by the original C# data: each animation block begins with a
         * header record whose first value is the action ID, followed by the icon
         * index sequence for that action.  The entire array is walked linearly
         * by the animation engine using offsets pre-computed at startup.
         *
         * @note This is the largest and most critical table in the game; any
         *       corruption here affects every Blupi movement and action.
         */
        static const shortcs table_blupi[2911];

        /**
         * @brief Mirror / horizontal-flip lookup for player sprite icon indices.
         *
         * @details Maps a right-facing icon index to its corresponding left-facing
         * (horizontally mirrored) icon index.  The array is indexed directly by
         * the current sprite icon number; the result replaces that icon when the
         * character is facing left.
         *
         * @note The array covers icon indices 0–334.  Icons not present in
         *       @c table_blupi will never be looked up here.
         */
        static const shortcs table_mirror[335];

        // -----------------------------------------------------------------------
        // Movement speed tables
        // -----------------------------------------------------------------------

        /**
         * @brief Per-phase pixel-per-tick horizontal speed for the walking animation.
         *
         * @details Each element corresponds to one phase step of the walk cycle and
         * gives the number of pixels Blupi moves per game tick during that step.
         * Indexed by @c (phase % 4).
         */
        static const shortcs table_vitesse_march[4];

        /**
         * @brief Per-phase pixel-per-tick speed for the swimming animation.
         *
         * @details Seven speed values cycled through the swimming phase counter.
         * Values reflect the varying paddle cadence of the stroke.
         */
        static const shortcs table_vitesse_nage[7];

        /**
         * @brief Per-phase pixel-per-tick speed for the surfboard animation.
         *
         * @details Six speed values cycled through the surfing phase counter.
         * The first and last entries are zero, producing a brief pause at the
         * extremes of the surf motion.
         */
        static const shortcs table_vitesse_surf[6];

        // -----------------------------------------------------------------------
        // Tile-adaptation tables
        // -----------------------------------------------------------------------

        /**
         * @brief Quarter-tile neighbour-mask to tile-ID mapping table.
         *
         * @details Drives the corner-blending / quarter-tile adaptation system.
         * The 7056-entry flat array is logically a two-dimensional lookup:
         * the outer dimension selects a tile-type group and the inner dimension
         * encodes the 8-neighbour bitmask of the current tile.  The returned
         * value is the tile icon ID that should be rendered for that particular
         * neighbourhood configuration.
         *
         * @warning Do not alter the size or order of this table; the indexing
         *          arithmetic in Decor is tightly coupled to the original layout.
         */
        static const shortcs table_decor_quart[7056];

        // -----------------------------------------------------------------------
        // Bulldozer NPC animation
        // -----------------------------------------------------------------------

        /** @brief Sprite icon sequence for the bulldozer moving left (8 frames, looping). */
        static const shortcs table_bulldozer_left[8];

        /** @brief Sprite icon sequence for the bulldozer moving right (8 frames, looping). */
        static const shortcs table_bulldozer_right[8];

        /** @brief Sprite icon sequence for the bulldozer turning left (22-frame transition). */
        static const shortcs table_bulldozer_turn2l[22];

        /** @brief Sprite icon sequence for the bulldozer turning right (22-frame transition). */
        static const shortcs table_bulldozer_turn2r[22];

        // -----------------------------------------------------------------------
        // Fish (poisson) NPC animation
        // -----------------------------------------------------------------------

        /** @brief Sprite icon sequence for the fish swimming left (8 frames, looping). */
        static const shortcs table_poisson_left[8];

        /** @brief Sprite icon sequence for the fish swimming right (8 frames, looping). */
        static const shortcs table_poisson_right[8];

        /** @brief Sprite icon sequence for the fish turning left (48-frame transition). */
        static const shortcs table_poisson_turn2l[48];

        /** @brief Sprite icon sequence for the fish turning right (48-frame transition). */
        static const shortcs table_poisson_turn2r[48];

        // -----------------------------------------------------------------------
        // Bird (oiseau) NPC animation
        // -----------------------------------------------------------------------

        /** @brief Sprite icon sequence for the bird flying left (8 frames, looping). */
        static const shortcs table_oiseau_left[8];

        /** @brief Sprite icon sequence for the bird flying right (8 frames, looping). */
        static const shortcs table_oiseau_right[8];

        /** @brief Sprite icon sequence for the bird banking left (10-frame transition). */
        static const shortcs table_oiseau_turn2l[10];

        /** @brief Sprite icon sequence for the bird banking right (10-frame transition). */
        static const shortcs table_oiseau_turn2r[10];

        // -----------------------------------------------------------------------
        // Wasp (guepe) NPC animation
        // -----------------------------------------------------------------------

        /** @brief Sprite icon sequence for the wasp flying left (6 frames, looping). */
        static const shortcs table_guepe_left[6];

        /** @brief Sprite icon sequence for the wasp flying right (6 frames, looping). */
        static const shortcs table_guepe_right[6];

        /** @brief Sprite icon sequence for the wasp turning left (5-frame transition). */
        static const shortcs table_guepe_turn2l[5];

        /** @brief Sprite icon sequence for the wasp turning right (5-frame transition). */
        static const shortcs table_guepe_turn2r[5];

        // -----------------------------------------------------------------------
        // Creature NPC animation
        // -----------------------------------------------------------------------

        /** @brief Sprite icon sequence for the creature moving left (8 frames, looping). */
        static const shortcs table_creature_left[8];

        /** @brief Sprite icon sequence for the creature moving right (8 frames, looping). */
        static const shortcs table_creature_right[8];

        /**
         * @brief Sprite icon sequence for the creature turn animation (152 frames).
         *
         * @details Encodes a long pulsing turn cycle used when the creature changes
         * direction; the sequence loops several times before settling on the new
         * direction icons.
         */
        static const shortcs table_creature_turn2[152];

        // -----------------------------------------------------------------------
        // Mini-Blupi helicopter (blupih) NPC animation
        // -----------------------------------------------------------------------

        /** @brief Sprite icon sequence for the helicopter Blupi moving left (8 frames, looping). */
        static const shortcs table_blupih_left[8];

        /** @brief Sprite icon sequence for the helicopter Blupi moving right (8 frames, looping). */
        static const shortcs table_blupih_right[8];

        /** @brief Sprite icon sequence for the helicopter Blupi turning left (26-frame transition). */
        static const shortcs table_blupih_turn2l[26];

        /** @brief Sprite icon sequence for the helicopter Blupi turning right (26-frame transition). */
        static const shortcs table_blupih_turn2r[26];

        // -----------------------------------------------------------------------
        // Mini-Blupi tank (blupit) NPC animation
        // -----------------------------------------------------------------------

        /** @brief Sprite icon sequence for the tank Blupi moving left (8 frames, looping). */
        static const shortcs table_blupit_left[8];

        /** @brief Sprite icon sequence for the tank Blupi moving right (8 frames, looping). */
        static const shortcs table_blupit_right[8];

        /** @brief Sprite icon sequence for the tank Blupi turning left (24-frame transition). */
        static const shortcs table_blupit_turn2l[24];

        /** @brief Sprite icon sequence for the tank Blupi turning right (24-frame transition). */
        static const shortcs table_blupit_turn2r[24];

        // -----------------------------------------------------------------------
        // Explosion animation tables
        // -----------------------------------------------------------------------

        /** @brief Number of entries in @c table_explo1 (compile-time constant). */
        static constexpr shortcs table_explo1Length = 39;

        /**
         * @brief Primary explosion sprite icon sequence (39 frames, channel 0).
         *
         * @details Standard dynamite/bomb explosion; runs from small flash to full
         * blast and dissipates.  Values are icon indices; -1 is never present here.
         */
        static const shortcs table_explo1[table_explo1Length];

        /**
         * @brief Secondary explosion icon sequence (20 frames, channel 1).
         *
         * @details Scattered debris fragment animation.  Entries of -1 indicate
         * a blank / invisible frame (no sprite drawn on that tick).
         */
        static const shortcs table_explo2[20];

        /**
         * @brief Tertiary explosion icon sequence (20 frames, channel 2).
         *
         * @details Smoke-puff variation using a smaller set of icons than explo1.
         */
        static const shortcs table_explo3[20];

        /**
         * @brief Small flash explosion icon sequence (9 frames, channel 3).
         *
         * @details Short-lived spark flash used for minor impacts.
         */
        static const shortcs table_explo4[9];

        /**
         * @brief Interleaved explosion icon sequence with blanks (12 frames, channel 4).
         *
         * @details Every other entry is -1 (blank), producing a strobing effect.
         */
        static const shortcs table_explo5[12];

        /**
         * @brief Dense explosion icon sequence (6 frames, channel 5).
         *
         * @details Rapid six-frame burst, used for compressed explosions.
         */
        static const shortcs table_explo6[6];

        /**
         * @brief Large scatter explosion icon sequence (128 frames, channel 6).
         *
         * @details Long multi-particle scatter animation; -1 entries suppress
         * individual scatter particles for a staggered appearance.
         */
        static const shortcs table_explo7[128];

        /**
         * @brief Tail-end explosion icon sequence (5 frames, channel 7).
         *
         * @details Final dying-ember frames appended after the main blast.
         */
        static const shortcs table_explo8[5];

        // -----------------------------------------------------------------------
        // Splash (sploutch) animation tables
        // -----------------------------------------------------------------------

        /**
         * @brief Water splash icon sequence — immediate (10 frames).
         *
         * @details Used when an object hits water from a short height.
         * All 10 entries are valid sprite icons (no blank frames).
         */
        static const shortcs table_sploutch1[10];

        /**
         * @brief Water splash icon sequence — short delay (13 frames).
         *
         * @details The first three entries are -1 (blank), introducing a brief
         * pause before the splash animation begins.
         */
        static const shortcs table_sploutch2[13];

        /**
         * @brief Water splash icon sequence — long delay (18 frames).
         *
         * @details The first eight entries are -1 (blank) for a longer pre-splash
         * pause, used for objects falling from greater heights.
         */
        static const shortcs table_sploutch3[18];

        // -----------------------------------------------------------------------
        // Tentacle animation
        // -----------------------------------------------------------------------

        /**
         * @brief Tentacle sprite icon sequence (45 frames).
         *
         * @details Encodes the tentacle-rise, hold, and retract animation.
         * A -1 at position 7 and at the final position hides the sprite at those
         * ticks to create a snap effect.
         */
        static const shortcs table_tentacule[45];

        // -----------------------------------------------------------------------
        // Bridge construction animation
        // -----------------------------------------------------------------------

        /**
         * @brief Bridge panel sprite icon sequence (157 frames).
         *
         * @details Encodes bridge-extend (first 28 frames), an invisible hold
         * section filled with -1, and bridge-retract (last 17 frames) ending on
         * the folded-bridge icon (364).  Consumed one entry per tick.
         */
        static const shortcs table_bridge[157];

        // -----------------------------------------------------------------------
        // Environmental effect icon sequences
        // -----------------------------------------------------------------------

        /**
         * @brief Pollution / smog cloud sprite icon sequence (8 frames, looping).
         *
         * @details Cycled continuously over an active pollution tile.
         */
        static const shortcs table_pollution[8];

        /**
         * @brief Inversion field startup icon sequence (8 frames, channel start).
         *
         * @details Played once when an inversion panel is first activated.
         */
        static const shortcs table_invertstart[8];

        /**
         * @brief Inversion field shutdown icon sequence (8 frames, channel stop).
         *
         * @details Played once when an inversion panel is deactivated; reverses
         * the order of @c table_invertstart.
         */
        static const shortcs table_invertstop[8];

        /**
         * @brief Inversion panel idle icon sequence (8 frames, looping).
         *
         * @details Shown on the inversion panel tile while the field is active.
         */
        static const shortcs table_invertpanel[8];

        /**
         * @brief Water entry splash icon sequence (7 frames).
         *
         * @details Short ripple sequence played when Blupi enters water.
         */
        static const shortcs table_plouf[7];

        /**
         * @brief Tip-splash (tiny droplet) icon sequence (3 frames).
         *
         * @details Very brief water-drop effect; index 1 is a blank frame (244).
         */
        static const shortcs table_tiplouf[3];

        /**
         * @brief Water bubble icon sequence (20 frames, looping).
         *
         * @details Looping bubble animation displayed while Blupi is submerged.
         */
        static const shortcs table_blup[20];

        // -----------------------------------------------------------------------
        // Follow / enemy tracking animation
        // -----------------------------------------------------------------------

        /**
         * @brief Enemy follow / tracking full animation sequence (26 frames).
         *
         * @details Long cycle used for the chase indicator shown above a
         * following enemy; includes approach, hold, and release phases.
         */
        static const shortcs table_follow1[26];

        /**
         * @brief Enemy follow / tracking fast sequence (5 frames).
         *
         * @details Abbreviated version of @c table_follow1 for rapid-follow mode.
         */
        static const shortcs table_follow2[5];

        // -----------------------------------------------------------------------
        // Key pick-up animation sequences
        // -----------------------------------------------------------------------

        /**
         * @brief Generic key spin icon sequence (12 frames, looping).
         *
         * @details Used for the standard (uncoloured) key collectible.
         */
        static const shortcs table_cle[12];

        /** @brief Red key spin icon sequence (12 frames, looping). */
        static const shortcs table_cle1[12];

        /** @brief Green key spin icon sequence (12 frames, looping). */
        static const shortcs table_cle2[12];

        /** @brief Blue key spin icon sequence (12 frames, looping). */
        static const shortcs table_cle3[12];

        // -----------------------------------------------------------------------
        // Dynamite fuse flicker animation
        // -----------------------------------------------------------------------

        /**
         * @brief Dynamite fuse-flicker icon sequence (100 frames).
         *
         * @details Pseudo-random looking sequence of fuse-spark icons (252–255)
         * played while a placed dynamite charge is counting down.  The apparent
         * randomness is baked-in data, not runtime RNG.
         */
        static const shortcs table_dynamitef[100];

        // -----------------------------------------------------------------------
        // Skateboard animation
        // -----------------------------------------------------------------------

        /**
         * @brief Skateboard wheel-spin icon sequence (34 frames, looping).
         *
         * @details Eases in at the start, accelerates, then eases out at the end
         * of the speed ramp.
         */
        static const shortcs table_skate[34];

        // -----------------------------------------------------------------------
        // Bird-lime / glue effect animation
        // -----------------------------------------------------------------------

        /**
         * @brief Bird-lime (glue) splash icon sequence (25 frames).
         *
         * @details Played on the tile where glue was deployed and while Blupi is
         * stuck.
         */
        static const shortcs table_glu[25];

        // -----------------------------------------------------------------------
        // Clear / transparency effect animation
        // -----------------------------------------------------------------------

        /**
         * @brief Clear (transparency) effect icon sequence (70 frames).
         *
         * @details Cycled while the transparency / ghost power-up is active;
         * uses the same icon set as the electric animation.
         */
        static const shortcs table_clear[70];

        // -----------------------------------------------------------------------
        // Electric shock animation
        // -----------------------------------------------------------------------

        /**
         * @brief Electric-shock icon sequence (90 frames).
         *
         * @details First 30 frames show a fast spark strobe (icons 266/267
         * alternating), followed by 60 frames of the aftermath body-vibration
         * animation.
         */
        static const shortcs table_electro[90];

        // -----------------------------------------------------------------------
        // Caterpillar (chenille) animation
        // -----------------------------------------------------------------------

        /**
         * @brief Caterpillar forward-crawl icon sequence (6 frames, looping).
         *
         * @details Indices 311–316 in ascending order.
         */
        static const shortcs table_chenille[6];

        /**
         * @brief Caterpillar reverse-crawl icon sequence (6 frames, looping).
         *
         * @details Indices 316–311 in descending order (reverse of
         * @c table_chenille).
         */
        static const shortcs table_chenillei[6];

        // -----------------------------------------------------------------------
        // Tile-adaptation lookup tables
        // -----------------------------------------------------------------------

        /**
         * @brief Decor tile-type neighbour-to-icon mapping (144 entries).
         *
         * @details Maps each of the 144 neighbour-bitmask combinations for the
         * standard decor tile types to the correct replacement tile icon ID.
         * Used by the tile-adaptation pass in Decor to select corner variants.
         */
        static const shortcs table_adapt_decor[144];

        /**
         * @brief Cheese / fromage tile-type neighbour-to-icon mapping (32 entries).
         *
         * @details Same purpose as @c table_adapt_decor but specific to the
         * cheese/fromage tile type.  Entry 0 and entry 16 are -1 (no replacement).
         */
        static const shortcs table_adapt_fromage[32];

        // -----------------------------------------------------------------------
        // Shield power-up animation
        // -----------------------------------------------------------------------

        /**
         * @brief Shield orbit icon sequence (16 frames, looping).
         *
         * @details Icon indices for the rotating shield orb surrounding Blupi
         * while the round-shield cheat / power-up is active.
         */
        static const shortcs table_shield[16];

        /**
         * @brief Shield-on-Blupi overlay icon sequence (16 frames, looping).
         *
         * @details Identical data to @c table_shield; drawn on top of Blupi rather
         * than as a separate sprite in some rendering paths.
         */
        static const shortcs table_shield_blupi[16];

        // -----------------------------------------------------------------------
        // Power charge animation
        // -----------------------------------------------------------------------

        /**
         * @brief Power-charge icon sequence (8 frames, looping).
         *
         * @details Spinning energy-ball icons displayed while the power-charge
         * cheat is active.
         */
        static const shortcs table_power[8];

        // -----------------------------------------------------------------------
        // Inversion overlay animation
        // -----------------------------------------------------------------------

        /**
         * @brief Inversion overlay icon sequence (20 frames).
         *
         * @details First 10 frames ramp up the inversion colour wash; last 10
         * frames ramp it back down.
         */
        static const shortcs table_invert[20];

        // -----------------------------------------------------------------------
        // Charge / energy bar animation
        // -----------------------------------------------------------------------

        /**
         * @brief Energy-charge bar icon sequence (6 frames).
         *
         * @details Six consecutive icons representing increasing charge level.
         */
        static const shortcs table_charge[6];

        // -----------------------------------------------------------------------
        // Magic-loop power-up animation
        // -----------------------------------------------------------------------

        /**
         * @brief Magic-loop idle icon sequence (5 frames, looping).
         *
         * @details Short looping shimmer shown while the magic loop orbits Blupi.
         */
        static const shortcs table_magicloop[5];

        /**
         * @brief Magic-loop full track icon sequence (24 frames).
         *
         * @details Complete magic-loop travel animation including approach,
         * orbit, and departure phases.
         */
        static const shortcs table_magictrack[24];

        // -----------------------------------------------------------------------
        // Shield-loop animation
        // -----------------------------------------------------------------------

        /**
         * @brief Shield-loop idle icon sequence (5 frames, looping).
         *
         * @details Short looping shimmer shown while the lollipop shield orbits.
         */
        static const shortcs table_shieldloop[5];

        /**
         * @brief Shield-loop full track icon sequence (20 frames).
         *
         * @details Complete lollipop-shield travel animation.
         */
        static const shortcs table_shieldtrack[20];

        // -----------------------------------------------------------------------
        // Drink power-up animation
        // -----------------------------------------------------------------------

        /**
         * @brief Drink-effect icon sequence (5 frames).
         *
         * @details Visual effect overlay played when Blupi consumes the drink
         * power-up.  Same icon set as @c table_shieldloop.
         */
        static const shortcs table_drinkeffect[5];

        /** @brief Number of entries in @c table_drinkoffset (compile-time constant). */
        static constexpr unsigned char table_drinkoffsetLength = 3;

        /**
         * @brief Byte offsets into @c table_blupi for the three drink-animation phases.
         *
         * @details Each of the three values is the index into @c table_blupi at
         * which the drink action's icon sub-sequence begins for the corresponding
         * animation phase (0 = normal, 1 = mid, 2 = advanced).
         */
        static const shortcs table_drinkoffset[table_drinkoffsetLength];

        // -----------------------------------------------------------------------
        // Treasure track animation
        // -----------------------------------------------------------------------

        /**
         * @brief Treasure-track icon sequence (11 frames).
         *
         * @details Oscillating shimmer played on a collected treasure tile;
         * bounces between icons 161 and 166.
         */
        static const shortcs table_tresortrack[11];

        // -----------------------------------------------------------------------
        // Decor hazard animation tables
        // -----------------------------------------------------------------------

        /**
         * @brief Lava tile animation icon sequence (8 frames, looping).
         *
         * @details Cycled continuously on lava tiles to produce a bubbling effect.
         */
        static const shortcs table_decor_lave[8];

        /**
         * @brief Spike-trap tile animation icon sequence — armed cycle (16 frames).
         *
         * @details Complex irregular cycle that produces the threatening spike
         * movement when the trap is fully deployed.
         */
        static const shortcs table_decor_piege1[16];

        /**
         * @brief Spike-trap tile animation icon sequence — trigger cycle (4 frames).
         *
         * @details Short cycle used when the trap resets after firing.
         */
        static const shortcs table_decor_piege2[4];

        /**
         * @brief Dripping water tile animation icon sequence (48 frames).
         *
         * @details Encodes a multi-drop sequence with -1 blank frames between
         * drops to represent the pause between individual drips.
         */
        static const shortcs table_decor_goutte[48];

        /**
         * @brief Crusher tile animation icon sequence (10 frames).
         *
         * @details Descend frames followed by a hold at the lowest position;
         * index 0 and 1 are the raised state, index 7–9 hold the lowered crush.
         */
        static const shortcs table_decor_ecraseur[10];

        /**
         * @brief Saw blade tile animation icon sequence (6 frames, looping).
         *
         * @details Six consecutive icons cycling through the saw rotation.
         */
        static const shortcs table_decor_scie[6];

        /**
         * @brief Temperature-control tile animation icon sequence (20 frames).
         *
         * @details Ramps temperature icons down then back up; ends with two -1
         * blank frames for a brief pause at the cycle boundary.
         */
        static const shortcs table_decor_temp[20];

        /**
         * @brief Water surface ripple icon sequence — style 1 (6 frames, looping).
         *
         * @details Used for the primary water-surface tile animation.
         */
        static const shortcs table_decor_eau1[6];

        /**
         * @brief Water surface ripple icon sequence — style 2 (6 frames, looping).
         *
         * @details Used for the secondary / deeper water-surface tile animation.
         */
        static const shortcs table_decor_eau2[6];

        // -----------------------------------------------------------------------
        // Fan (ventillo) and wind-vent animation tables
        // -----------------------------------------------------------------------

        /**
         * @brief Fan blade icon sequence — left-blowing (3 frames, looping).
         *
         * @details Cycled on a left-facing fan tile.
         */
        static const shortcs table_decor_ventillog[3];

        /**
         * @brief Fan blade icon sequence — right-blowing (3 frames, looping).
         *
         * @details Cycled on a right-facing fan tile.
         */
        static const shortcs table_decor_ventillod[3];

        /**
         * @brief Fan blade icon sequence — upward-blowing (3 frames, looping).
         *
         * @details Cycled on an upward-facing fan tile.
         */
        static const shortcs table_decor_ventilloh[3];

        /**
         * @brief Fan blade icon sequence — downward-blowing (3 frames, looping).
         *
         * @details Cycled on a downward-facing fan tile.
         */
        static const shortcs table_decor_ventillob[3];

        /**
         * @brief Wind-vent icon sequence — left (4 frames, looping).
         *
         * @details Particle stream emitted from a left-facing wind vent.
         */
        static const shortcs table_decor_ventg[4];

        /**
         * @brief Wind-vent icon sequence — right (4 frames, looping).
         *
         * @details Particle stream emitted from a right-facing wind vent.
         */
        static const shortcs table_decor_ventd[4];

        /**
         * @brief Wind-vent icon sequence — up (4 frames, looping).
         *
         * @details Particle stream emitted from an upward-facing wind vent.
         */
        static const shortcs table_decor_venth[4];

        /**
         * @brief Wind-vent icon sequence — down (4 frames, looping).
         *
         * @details Particle stream emitted from a downward-facing wind vent.
         */
        static const shortcs table_decor_ventb[4];

        // -----------------------------------------------------------------------
        // Marine / spring hazard animation
        // -----------------------------------------------------------------------

        /**
         * @brief Nautical mine icon sequence (11 frames, looping).
         *
         * @details Slow spin cycle used on the sea-mine (marine) tile.
         */
        static const shortcs table_marine[11];

        /**
         * @brief Spring / coil icon sequence (8 frames).
         *
         * @details Compress-and-release animation; used on spring tiles that
         * launch Blupi upward.
         */
        static const shortcs table_ressort[8];

        // -----------------------------------------------------------------------
        // Training / tutorial level tables (mutable)
        // -----------------------------------------------------------------------

        /**
         * @brief Tutorial level 1 tile layout and hint data (133 entries, mutable).
         *
         * @details Records are 6 elements wide:
         *   [0] start tile index,
         *   [1] end tile index,
         *   [2] minimum tile row (0 = any),
         *   [3] maximum tile row (50 = full height),
         *   [4] action flag (-1 = no restriction),
         *   [5] localised text-resource ID — **patched by Init()**.
         * The table terminates with a -1 sentinel in element [0] of the last
         * record.
         *
         * @note Do not write to this table outside of Init().
         */
        static shortcs table_training1[133];

        /**
         * @brief Tutorial level 2 tile layout and hint data (31 entries, mutable).
         *
         * @details Same 6-element record format as @c table_training1.
         * Text-resource IDs are patched by Init() at slots [5], [11], [17],
         * [23], [29].
         *
         * @note Do not write to this table outside of Init().
         */
        static shortcs table_training2[31];

        /**
         * @brief Tutorial level 3 tile layout and hint data (67 entries, mutable).
         *
         * @details Same 6-element record format; 11 text-resource slots are
         * patched by Init().
         *
         * @note Do not write to this table outside of Init().
         */
        static shortcs table_training3[67];

        /**
         * @brief Tutorial level 4 tile layout and hint data (31 entries, mutable).
         *
         * @details Same 6-element record format; 5 text-resource slots are
         * patched by Init().
         *
         * @note Do not write to this table outside of Init().
         */
        static shortcs table_training4[31];

        // -----------------------------------------------------------------------
        // Decor action and explosion meta-tables
        // -----------------------------------------------------------------------

        /**
         * @brief Scripted tile-motion offset table (519 entries).
         *
         * @details Encodes per-tick (dx, dy) displacement pairs used by the Decor
         * scripted-animation system.  The array contains multiple concatenated
         * motion scripts separated by terminator records; each script entry is a
         * pair (dx, dy) consumed one pair per game tick.  A value of -1 in the dx
         * position terminates the current script.
         *
         * The first two bytes of each script are a header: [0] = script ID,
         * [1] = total number of (dx, dy) pairs that follow.
         *
         * @note All values are in sub-pixel units scaled to the tile coordinate
         *       system used by Decor.
         */
        static const shortcs table_decor_action[519];

        /**
         * @brief Per-explosion-channel bounding-box width table (100 entries).
         *
         * @details Indexed by explosion channel index (0–99).  Each value is the
         * half-width in pixels of the bounding square used for collision and
         * clipping during that channel's explosion rendering.
         * Common values: 128 (standard blast radius), 64 (small fragment),
         * 144 (oversized mega-blast for channels 66–68).
         */
        static const shortcs table_explo_size[100];

        /**
         * @brief End-of-game world descriptor (30 entries).
         *
         * @details Flat array of (tile-x, tile-y) coordinate pairs that define
         * the positions of the terminal world tiles rendered on the victory screen.
         * The first pair (0, 0) is the anchor; subsequent pairs place decorative
         * trophy / portal tiles.
         */
        static const shortcs world_terminal[30];

        // -----------------------------------------------------------------------
        // Initialisation
        // -----------------------------------------------------------------------

        /**
         * @brief Initialises mutable tables based on the current platform/mode configuration.
         *
         * @details Must be called exactly once at startup, before any table data is
         * accessed by game logic.  The function is idempotent: a static flag prevents
         * repeated initialisation.
         *
         * Currently the function patches localised text-resource IDs (drawn from
         * @c MyResource) into element [5] of each 6-element hint record in
         * @c table_training1..4.  This allows training-level hint text to be
         * localised at runtime without changing the static array data.
         *
         * @pre @c MyResource must be fully initialised before this is called.
         * @post @c table_training1..4 have valid text-resource IDs at every slot-5
         *       position; all other table entries remain unchanged.
         */
        static void Init();
    };
}
