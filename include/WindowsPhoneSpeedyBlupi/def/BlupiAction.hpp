/**
 * @file BlupiAction.hpp
 * @brief Defines the BlupiAction enumeration describing every animation/movement state of the player character.
 *
 * @details Each BlupiAction value identifies one distinct state in the gameplay state
 * machine (idle, walking, jumping, vehicle modes, hazard contacts, etc.). The mapping
 * from action values to sprite frames is resolved by the animation tables in Tables.hpp.
 */

#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using BlupiActionUnderlying = SharpRuntime::ubytecs;

    /**
     * @brief Player (Blupi) action/state used by the gameplay state machine.
     *
     * Each value corresponds to a distinct animation and movement state that Blupi
     * can be in during gameplay. The state machine in Decor transitions between these
     * values based on player input, collision results, and game events.
     *
     * Groups of related actions follow naming conventions from the original game:
     * - Stop/March/Turn variants: idle, walking, and turning for a given vehicle/mode.
     * - Jump/Air variants: airborne states.
     * - Ouf variants: recovery/celebration animations after a dangerous situation.
     * - Glu/Electro/Charge: hazard-contact states.
     *
     * @note This is gameplay state, not a sprite/icon index. The mapping from
     *       BlupiAction to sprite frame is determined by the animation tables in Tables.hpp.
     * @note Stored as an unsigned byte (ubytecs) to match the original C# enum layout.
     * @note Values come from the original game's ACTION_ constants. Do not renumber them.
     */
    enum class BlupiAction : BlupiActionUnderlying
    {
        None = 0,              ///< @brief No action / uninitialised state.
        Stop = 1,              ///< @brief Standing still (ACTION_STOP).
        March = 2,             ///< Walking (ACTION_MARCH).
        Turn = 3,              ///< Turning around (ACTION_TURN).
        Jump = 4,              ///< Jumping (ACTION_JUMP).
        Air = 5,               ///< Airborne / falling (ACTION_AIR).
        Down = 6,              ///< Moving down (ACTION_DOWN).
        Up = 7,                ///< Moving up (ACTION_UP).
        Vertigo = 8,           ///< Hanging on a ledge in fear (ACTION_VERTIGO).
        Recede = 9,            ///< Moving backward (ACTION_RECEDE).
        Advance = 10,          ///< Moving forward (ACTION_ADVANCE).
        Clear1 = 11,           ///< Clearing/erasing animation variant 1 (ACTION_CLEAR1).
        Set = 12,              ///< Placing/setting an object (ACTION_SET).
        Win = 13,              ///< Level-win celebration (ACTION_WIN).
        Push = 14,             ///< Pushing a crate (ACTION_PUSH).
        StopHelico = 15,       ///< Hovering in helicopter mode (ACTION_STOPHELICO).
        MarchHelico = 16,      ///< Flying forward in helicopter mode (ACTION_MARCHHELICO).
        TurnHelico = 17,       ///< Turning in helicopter mode (ACTION_TURNHELICO).
        StopNage = 18,         ///< Treading water (ACTION_STOPNAGE).
        MarchNage = 19,        ///< Swimming forward (ACTION_MARCHNAGE).
        TurnNage = 20,         ///< Turning while swimming (ACTION_TURNNAGE).
        StopSurf = 21,         ///< Surfboard idle (ACTION_STOPSURF).
        MarchSurf = 22,        ///< Surfing forward (ACTION_MARCHSURF).
        TurnSurf = 23,         ///< Turning on surfboard (ACTION_TURNSURF).
        Drown = 24,            ///< Drowning in deep water (ACTION_DROWN).
        StopJeep = 25,         ///< Jeep idle (ACTION_STOPJEEP).
        MarchJeep = 26,        ///< Driving jeep (ACTION_MARCHJEEP).
        TurnJeep = 27,         ///< Turning jeep (ACTION_TURNJEEP).
        StopPop = 28,          ///< Pop-star idle (ACTION_STOPPOP).
        Pop = 29,              ///< Pop-star dancing/moving (ACTION_POP).
        Bye = 30,              ///< Farewell/exit animation (ACTION_BYE).
        StopSuspend = 31,      ///< Hanging idle (ACTION_STOPSUSPEND).
        MarchSuspend = 32,     ///< Moving while hanging (ACTION_MARCHSUSPEND).
        TurnSuspend = 33,      ///< Turning while hanging (ACTION_TURNSUSPEND).
        JumpSuspend = 34,      ///< Jumping from a hanging position (ACTION_JUMPSUSPEND).
        Hide = 35,             ///< Hiding (ACTION_HIDE).
        JumpAie = 36,          ///< Hurt-jump (ACTION_JUMPAIE).
        StopSkate = 37,        ///< Skateboard idle (ACTION_STOPSKATE).
        MarchSkate = 38,       ///< Skating forward (ACTION_MARCHSKATE).
        TurnSkate = 39,        ///< Turning on skateboard (ACTION_TURNSKATE).
        JumpSkate = 40,        ///< Jumping on skateboard (ACTION_JUMPSKATE).
        AirSkate = 41,         ///< Airborne on skateboard (ACTION_AIRSKATE).
        TakeSkate = 42,        ///< Picking up skateboard (ACTION_TAKESKATE).
        DeposeSkate = 43,      ///< Putting down skateboard (ACTION_DEPOSESKATE).
        Ouf1a = 44,            ///< Relief animation variant 1a (ACTION_OUF1a).
        Ouf1b = 45,            ///< Relief animation variant 1b (ACTION_OUF1b).
        Ouf2 = 46,             ///< Relief animation variant 2 (ACTION_OUF2).
        Ouf3 = 47,             ///< Relief animation variant 3 (ACTION_OUF3).
        Ouf4 = 48,             ///< Relief animation variant 4 (ACTION_OUF4).
        Sucette = 49,          ///< Collecting a lollipop power-up (ACTION_SUCETTE).
        StopTank = 50,         ///< Tank idle (ACTION_STOPTANK).
        MarchTank = 51,        ///< Driving tank (ACTION_MARCHTANK).
        TurnTank = 52,         ///< Turning tank (ACTION_TURNTANK).
        FireTank = 53,         ///< Tank firing (ACTION_FIRETANK).
        Glu = 54,              ///< Stuck in glue/trap (ACTION_GLU).
        Drink = 55,            ///< Drinking a power-up (ACTION_DRINK).
        Charge = 56,           ///< Being charged at by an enemy (ACTION_CHARGE).
        Electro = 57,          ///< Electrocuted (ACTION_ELECTRO).
        HelicoGlu = 58,        ///< Helicopter stuck in glue (ACTION_HELICOGLU).
        TurnAir = 59,          ///< Turning while airborne (ACTION_TURNAIR).
        StopMarch = 60,        ///< Decelerating from walk to stop (ACTION_STOPMARCH).
        StopJump = 61,         ///< Jump landing (ACTION_STOPJUMP).
        StopJumph = 62,        ///< High-jump landing (ACTION_STOPJUMPh).
        Mockery = 63,          ///< Enemy mocking Blupi (ACTION_MOCKERY).
        Mockeryi = 64,         ///< Enemy mocking Blupi, inverted (ACTION_MOCKERYi).
        Ouf5 = 65,             ///< Relief animation variant 5 (ACTION_OUF5).
        Balloon = 66,          ///< Balloon mode (ACTION_BALLOON).
        StopOver = 67,         ///< Flat/squashed idle (ACTION_STOPOVER).
        MarchOver = 68,        ///< Moving while flat (ACTION_MARCHOVER).
        TurnOver = 69,         ///< Turning while flat (ACTION_TURNOVER).
        Recedeq = 70,          ///< Quick backward movement (ACTION_RECEDEq).
        Advanceq = 71,         ///< Quick forward movement (ACTION_ADVANCEq).
        StopEcrase = 72,       ///< Crushed idle (ACTION_STOPECRASE).
        MarchEcrase = 73,      ///< Moving while crushed (ACTION_MARCHECRASE).
        Teleporte = 74,        ///< Teleporting (ACTION_TELEPORTE).
        Clear2 = 75,           ///< Clearing animation variant 2 (ACTION_CLEAR2).
        Clear3 = 76,           ///< Clearing animation variant 3 (ACTION_CLEAR3).
        Clear4 = 77,           ///< Clearing animation variant 4 (ACTION_CLEAR4).
        Clear5 = 78,           ///< Clearing animation variant 5 (ACTION_CLEAR5).
        Clear6 = 79,           ///< Clearing animation variant 6 (ACTION_CLEAR6).
        Clear7 = 80,           ///< Clearing animation variant 7 (ACTION_CLEAR7).
        Clear8 = 81,           ///< Clearing animation variant 8 (ACTION_CLEAR8).
        Switch = 82,           ///< Activating a switch (ACTION_SWITCH).
        Mockeryp = 83,         ///< Enemy mocking, alternate pose (ACTION_MOCKERYp).
        Non = 84,              ///< Blupi refusing / shaking head (ACTION_NON).
        SlowdownSkate = 85,    ///< Skateboard braking (ACTION_SLOWDOWNSKATE).
        TakeDynamite = 86,     ///< Picking up dynamite (ACTION_TAKEDYNAMITE).
        PutDynamite = 87       ///< Placing dynamite (ACTION_PUTDYNAMITE).
    };

    /**
     * @brief Returns the raw underlying byte value of a BlupiAction.
     * @param[in] action The action to convert.
     * @return Underlying unsigned byte value.
     */
    static constexpr auto ToRaw(BlupiAction action) -> BlupiActionUnderlying
    {
        return static_cast<BlupiActionUnderlying>(action);
    }

    /**
     * @brief Converts an integer to a BlupiAction enum value.
     *
     * Used when loading action indices from data tables or save files.
     * The caller is responsible for ensuring @p value is a valid BlupiAction.
     *
     * @param[in] value Raw integer from original game data.
     * @return Corresponding BlupiAction enum value.
     */
    static constexpr auto ToBlupiAction(const int value) -> BlupiAction
    {
        return static_cast<BlupiAction>(
            static_cast<BlupiActionUnderlying>(value)
        );
    }
}
