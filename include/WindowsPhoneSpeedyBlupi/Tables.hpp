#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using SharpRuntime::shortcs;

    /**
     * @brief Static repository of all original game data tables.
     *
     * Tables contains the animation data, movement trajectories, sprite sequences,
     * environment/hazard data, and other lookup tables ported directly from the
     * original game. All values in these arrays come from the original C# source
     * and must not be changed without fully understanding the animation and gameplay
     * logic that indexes into them.
     *
     * Table format notes:
     * - Animation tables (e.g., table_blupi, table_bulldozer_*) typically contain
     *   sequences of (channel, icon, dx, dy, ...) tuples read by step counters.
     * - table_decor_quart drives the quarter-tile tile-adaptation system.
     * - table_decor_action drives animated tile sprites.
     * - table_explo_size provides per-channel explosion bounding sizes.
     *
     * @note Do not renumber, reorder, or resize any table without updating all code
     *       that indexes into it. Array sizes are part of the original game's data contract.
     * @note table_training1..4 are the only mutable tables (modified by Init() based on
     *       the player's current platform/mode). All others are const.
     * @note This is data code, not gameplay logic. Tables does not own any runtime state.
     */
    class Tables
    {
    public:
        Tables() = delete;
        ~Tables() = delete;

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
#endif
        };

        /**
         * @brief Blupi player animation table.
         *
         * Contains sequences of (channel, icon, offsetX, offsetY, ...) tuples indexed
         * by (BlupiAction, phase, direction). This is the primary animation data for
         * the player character. Do not modify.
         */
        static const shortcs table_blupi[2911];

        /** Mirror/flip variant of table_blupi for left/right direction switching. */
        static const shortcs table_mirror[335];

        /** Per-phase horizontal movement speed values for walking animation. */
        static const shortcs table_vitesse_march[4];

        static const shortcs table_vitesse_nage[7];

        static const shortcs table_vitesse_surf[6];

        static const shortcs table_decor_quart[7056];

        static const shortcs table_bulldozer_left[8];

        static const shortcs table_bulldozer_right[8];

        static const shortcs table_bulldozer_turn2l[22];

        static const shortcs table_bulldozer_turn2r[22];

        static const shortcs table_poisson_left[8];

        static const shortcs table_poisson_right[8];

        static const shortcs table_poisson_turn2l[48];

        static const shortcs table_poisson_turn2r[48];

        static const shortcs table_oiseau_left[8];

        static const shortcs table_oiseau_right[8];

        static const shortcs table_oiseau_turn2l[10];

        static const shortcs table_oiseau_turn2r[10];

        static const shortcs table_guepe_left[6];

        static const shortcs table_guepe_right[6];

        static const shortcs table_guepe_turn2l[5];

        static const shortcs table_guepe_turn2r[5];

        static const shortcs table_creature_left[8];

        static const shortcs table_creature_right[8];

        static const shortcs table_creature_turn2[152];

        static const shortcs table_blupih_left[8];

        static const shortcs table_blupih_right[8];

        static const shortcs table_blupih_turn2l[26];

        static const shortcs table_blupih_turn2r[26];

        static const shortcs table_blupit_left[8];

        static const shortcs table_blupit_right[8];

        static const shortcs table_blupit_turn2l[24];

        static const shortcs table_blupit_turn2r[24];

        static constexpr shortcs table_explo1Length = 39;

        static const shortcs table_explo1[table_explo1Length];

        static const shortcs table_explo2[20];

        static const shortcs table_explo3[20];

        static const shortcs table_explo4[9];

        static const shortcs table_explo5[12];

        static const shortcs table_explo6[6];

        static const shortcs table_explo7[128];

        static const shortcs table_explo8[5];

        static const shortcs table_sploutch1[10];

        static const shortcs table_sploutch2[13];

        static const shortcs table_sploutch3[18];

        static const shortcs table_tentacule[45];

        static const shortcs table_bridge[157];

        static const shortcs table_pollution[8];

        static const shortcs table_invertstart[8];

        static const shortcs table_invertstop[8];

        static const shortcs table_invertpanel[8];

        static const shortcs table_plouf[7];

        static const shortcs table_tiplouf[3];

        static const shortcs table_blup[20];

        static const shortcs table_follow1[26];

        static const shortcs table_follow2[5];

        static const shortcs table_cle[12];

        static const shortcs table_cle1[12];

        static const shortcs table_cle2[12];

        static const shortcs table_cle3[12];

        static const shortcs table_dynamitef[100];

        static const shortcs table_skate[34];

        static const shortcs table_glu[25];

        static const shortcs table_clear[70];

        static const shortcs table_electro[90];

        static const shortcs table_chenille[6];

        static const shortcs table_chenillei[6];

        static const shortcs table_adapt_decor[144];

        static const shortcs table_adapt_fromage[32];

        static const shortcs table_shield[16];

        static const shortcs table_shield_blupi[16];

        static const shortcs table_power[8];

        static const shortcs table_invert[20];

        static const shortcs table_charge[6];

        static const shortcs table_magicloop[5];

        static const shortcs table_magictrack[24];

        static const shortcs table_shieldloop[5];

        static const shortcs table_shieldtrack[20];

        static const shortcs table_drinkeffect[5];

        static constexpr unsigned char table_drinkoffsetLength = 3;

        static const shortcs table_drinkoffset[table_drinkoffsetLength];

        static const shortcs table_tresortrack[11];

        static const shortcs table_decor_lave[8];

        static const shortcs table_decor_piege1[16];

        static const shortcs table_decor_piege2[4];

        static const shortcs table_decor_goutte[48];

        static const shortcs table_decor_ecraseur[10];

        static const shortcs table_decor_scie[6];

        static const shortcs table_decor_temp[20];

        static const shortcs table_decor_eau1[6];

        static const shortcs table_decor_eau2[6];

        static const shortcs table_decor_ventillog[3];

        static const shortcs table_decor_ventillod[3];

        static const shortcs table_decor_ventilloh[3];

        static const shortcs table_decor_ventillob[3];

        static const shortcs table_decor_ventg[4];

        static const shortcs table_decor_ventd[4];

        static const shortcs table_decor_venth[4];

        static const shortcs table_decor_ventb[4];

        static const shortcs table_marine[11];

        static const shortcs table_ressort[8];

        /**
         * @brief Mutable training level data tables.
         *
         * These four tables hold the tile layout for the in-game training/tutorial
         * levels. Unlike the other const tables, these may be modified by Init()
         * to adapt tutorial content to the current platform mode.
         * Do not write to them outside of Init().
         */
        static shortcs table_training1[133];
        static shortcs table_training2[31];
        static shortcs table_training3[67];
        static shortcs table_training4[31];

        /** Animated tile action sequences used by Decor for scripted tile animations. */
        static const shortcs table_decor_action[519];

        /** Per-explosion-channel bounding-size table for explosion rendering. */
        static const shortcs table_explo_size[100];

        /** Terminal world/level descriptor used for end-of-game state. */
        static const shortcs world_terminal[30];

        /**
         * @brief Initialises mutable tables based on the current platform/mode configuration.
         *
         * Must be called once at startup before any table data is accessed.
         * Currently adapts the training level tables for the active game mode.
         */
        static void Init();
    };
}
