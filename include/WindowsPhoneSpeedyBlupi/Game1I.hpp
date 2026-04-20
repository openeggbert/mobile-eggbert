//
// Created by robertvokac on 3/27/25.
//

#ifndef GAME1I_H
#define GAME1I_H

#include <any>

#include "Def.hpp"
#include "InputPad.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDeviceManager.hpp"
#include "System/EventArgs.hpp"

#define readonly mutable

namespace WindowsPhoneSpeedyBlupi {
    class Game1I {
    protected:
        ~Game1I() = default;
    public:
        [[nodiscard]]virtual bool getIsRankingModeProperty() const = 0;

    public:
        [[nodiscard]]virtual bool getIsTrialModeProperty() const = 0;

    protected:
        virtual void Initialize() = 0;

    protected:
        virtual void LoadContent() = 0;

    protected:
        virtual void UnloadContent() = 0;

    protected:
        virtual void OnDeactivated(std::any sender, System::EventArgs args) = 0;

    protected:
        virtual void OnActivated(std::any sender, System::EventArgs args) = 0;

    protected:
        virtual void OnExiting(Microsoft::Xna::Framework::ExitingEventArgs args) = 0;

    protected:
        virtual void Update(Microsoft::Xna::Framework::GameTime &gameTime) = 0;

    private:
        virtual void MissionBack() = 0;

    private:
        virtual void StartMission(int mission) = 0;

    private:
        virtual void ContinueMission() = 0;

    private:
        virtual void CheatAction(Def::ButtonGlyph glyph) = 0;

    protected:
        virtual void Draw(const Microsoft::Xna::Framework::GameTime &gameTime) = 0;

    private:
        virtual void DrawBackgroundFade() = 0;

    private:
        virtual void DrawButtonsBackground() = 0;

    private:
        virtual void DrawButtonsText() = 0;

    private:
        virtual void DrawButtonGamerText(Def::ButtonGlyph glyph, int gamer) = 0;

    private:
        virtual void DrawTextRightButton(Def::ButtonGlyph glyph, int res) = 0;

    private:
        virtual void DrawTextRightButton(Def::ButtonGlyph glyph, std::string text) = 0;

    private:
        virtual void DrawTextUnderButton(Def::ButtonGlyph glyph, int res) = 0;

    private:
        virtual void DrawWaitProgress() = 0;

    private:
        virtual void DrawDebug() = 0;

    private:
        virtual void SetGamer(int gamer) = 0;

    private:
        virtual void SetPhase(Def::Phase phase) = 0;

    private:
        virtual void SetPhase(Def::Phase phase, int mission) = 0;

    private:
        virtual void MemorizeGamerProgress() = 0;

    public:
        virtual void ToggleFullScreen() = 0;

    public:
        virtual bool IsFullScreen() = 0;

    public:
        virtual Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager getGraphics() = 0;
        ////

        [[nodiscard]] virtual Microsoft::Xna::Framework::Content::ContentManager& getContentProperty() = 0;

        [[nodiscard]] virtual Microsoft::Xna::Framework::Graphics::GraphicsDevice& getGraphicsDeviceProperty() = 0;
    };

}

#endif //GAME1I_H
