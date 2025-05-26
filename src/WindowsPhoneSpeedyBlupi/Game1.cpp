// // WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// // WindowsPhoneSpeedyBlupi.Game1
// using System;
// using Microsoft.Xna.Framework;
// using Microsoft.Xna.Framework.GamerServices;
// using Microsoft.Xna.Framework.Input;
// using Microsoft.Xna.Framework.Input.Touch;
// using Microsoft.Xna.Framework.Media;
// using WindowsPhoneSpeedyBlupi;
// using static System.Net.Mime.MediaTypeNames;

#include "WindowsPhoneSpeedyBlupi/Game1.h"
#include <iostream>


namespace WindowsPhoneSpeedyBlupi {
    Game1::Game1():
        IMPL_PROP_CUSTOM_READONLY(bool, IsTrialMode, {return false;}),
        IMPL_PROP_CUSTOM_READONLY(bool, IsRankingMode,
{
    if (!simulateTrialMode)
    {
        return isTrialMode;
    }
    return true;
}


        )
    {

        Exiting += OnExiting;

#if KNI
        Deactivated += OnDeactivated;
        Activated += OnActivated;
#endif

        if (Env.IMPL.isNotKNI() && !TouchPanel.GetCapabilities().IsConnected)
        {
            this.IsMouseVisible = true;
#if !FNA
            Mouse.SetCursor(MouseCursor.Arrow);
#endif
        }

        graphics = new GraphicsDeviceManager(this);
        graphics.IsFullScreen = false;
        base.Content.RootDirectory = "Content";
        base.TargetElapsedTime = TimeSpan.FromTicks(500000L);
        base.InactiveSleepTime = TimeSpan.FromSeconds(1.0);
        missionToStart1 = -1;
        missionToStart2 = -1;
        gameData = new GameData();
        pixmap = new Pixmap(this, graphics);
        sound = new Sound(this, gameData);
        decor = new Decor();
        decor.Create(sound, pixmap, gameData);
        TinyPoint pos = new TinyPoint
        {
            X = 196,
            Y = 426
        };
        waitJauge = new Jauge();
        waitJauge.Create(pixmap, sound, pos, 3, false);
        waitJauge.SetHide(false);
        waitJauge.Zoom = 2.0;
        phase = Def::Phase.None;
        fadeOutPhase = Def::Phase.None;
        inputPad = new InputPad(this, decor, pixmap, sound, gameData);
        SetPhase(Def::Phase.First);

    }

    Game1::~Game1() {

    }
    void Game1::LoadContent() {

    }


    void Game1::Update(float deltaTime) {

    }

    void Game1::Draw() {
        std::cout<<"hello"<<std::endl;
    }



}







