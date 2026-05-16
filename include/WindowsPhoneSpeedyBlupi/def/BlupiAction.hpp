#pragma once
#include "SharpRuntime/SharpRuntimeHelper.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using BlupiActionUnderlying = SharpRuntime::ubytecs;
    enum class BlupiAction : BlupiActionUnderlying
    {
        None = 0,
        Stop = 1, // ACTION_STOP
        March = 2, // ACTION_MARCH
        Turn = 3, // ACTION_TURN
        Jump = 4, // ACTION_JUMP
        Air = 5, // ACTION_AIR
        Down = 6, // ACTION_DOWN
        Up = 7, // ACTION_UP
        Vertigo = 8, // ACTION_VERTIGO
        Recede = 9, // ACTION_RECEDE
        Advance = 10, // ACTION_ADVANCE
        Clear1 = 11, // ACTION_CLEAR1
        Set = 12, // ACTION_SET
        Win = 13, // ACTION_WIN
        Push = 14, // ACTION_PUSH
        StopHelico = 15, // ACTION_STOPHELICO
        MarchHelico = 16, // ACTION_MARCHHELICO
        TurnHelico = 17, // ACTION_TURNHELICO
        StopNage = 18, // ACTION_STOPNAGE
        MarchNage = 19, // ACTION_MARCHNAGE
        TurnNage = 20, // ACTION_TURNNAGE
        StopSurf = 21, // ACTION_STOPSURF
        MarchSurf = 22, // ACTION_MARCHSURF
        TurnSurf = 23, // ACTION_TURNSURF
        Drown = 24, // ACTION_DROWN
        StopJeep = 25, // ACTION_STOPJEEP
        MarchJeep = 26, // ACTION_MARCHJEEP
        TurnJeep = 27, // ACTION_TURNJEEP
        StopPop = 28, // ACTION_STOPPOP
        Pop = 29, // ACTION_POP
        Bye = 30, // ACTION_BYE
        StopSuspend = 31, // ACTION_STOPSUSPEND
        MarchSuspend = 32, // ACTION_MARCHSUSPEND
        TurnSuspend = 33, // ACTION_TURNSUSPEND
        JumpSuspend = 34, // ACTION_JUMPSUSPEND
        Hide = 35, // ACTION_HIDE
        JumpAie = 36, // ACTION_JUMPAIE
        StopSkate = 37, // ACTION_STOPSKATE
        MarchSkate = 38, // ACTION_MARCHSKATE
        TurnSkate = 39, // ACTION_TURNSKATE
        JumpSkate = 40, // ACTION_JUMPSKATE
        AirSkate = 41, // ACTION_AIRSKATE
        TakeSkate = 42, // ACTION_TAKESKATE
        DeposeSkate = 43, // ACTION_DEPOSESKATE
        Ouf1a = 44, // ACTION_OUF1a
        Ouf1b = 45, // ACTION_OUF1b
        Ouf2 = 46, // ACTION_OUF2
        Ouf3 = 47, // ACTION_OUF3
        Ouf4 = 48, // ACTION_OUF4
        Sucette = 49, // ACTION_SUCETTE
        StopTank = 50, // ACTION_STOPTANK
        MarchTank = 51, // ACTION_MARCHTANK
        TurnTank = 52, // ACTION_TURNTANK
        FireTank = 53, // ACTION_FIRETANK
        Glu = 54, // ACTION_GLU
        Drink = 55, // ACTION_DRINK
        Charge = 56, // ACTION_CHARGE
        Electro = 57, // ACTION_ELECTRO
        HelicoGlu = 58, // ACTION_HELICOGLU
        TurnAir = 59, // ACTION_TURNAIR
        StopMarch = 60, // ACTION_STOPMARCH
        StopJump = 61, // ACTION_STOPJUMP
        StopJumph = 62, // ACTION_STOPJUMPh
        Mockery = 63, // ACTION_MOCKERY
        Mockeryi = 64, // ACTION_MOCKERYi
        Ouf5 = 65, // ACTION_OUF5
        Balloon = 66, // ACTION_BALLOON
        StopOver = 67, // ACTION_STOPOVER
        MarchOver = 68, // ACTION_MARCHOVER
        TurnOver = 69, // ACTION_TURNOVER
        Recedeq = 70, // ACTION_RECEDEq
        Advanceq = 71, // ACTION_ADVANCEq
        StopEcrase = 72, // ACTION_STOPECRASE
        MarchEcrase = 73, // ACTION_MARCHECRASE
        Teleporte = 74, // ACTION_TELEPORTE
        Clear2 = 75, // ACTION_CLEAR2
        Clear3 = 76, // ACTION_CLEAR3
        Clear4 = 77, // ACTION_CLEAR4
        Clear5 = 78, // ACTION_CLEAR5
        Clear6 = 79, // ACTION_CLEAR6
        Clear7 = 80, // ACTION_CLEAR7
        Clear8 = 81, // ACTION_CLEAR8
        Switch = 82, // ACTION_SWITCH
        Mockeryp = 83, // ACTION_MOCKERYp
        Non = 84, // ACTION_NON
        SlowdownSkate = 85, // ACTION_SLOWDOWNSKATE
        TakeDynamite = 86, // ACTION_TAKEDYNAMITE
        PutDynamite = 87 // ACTION_PUTDYNAMITE
    };

    static constexpr auto ToRaw(BlupiAction action) -> BlupiActionUnderlying
    {
        return static_cast<BlupiActionUnderlying>(action);
    }

    static constexpr auto ToBlupiAction(const int value) -> BlupiAction
    {
        return static_cast<BlupiAction>(
            static_cast<BlupiActionUnderlying>(value)
        );
    }
}
