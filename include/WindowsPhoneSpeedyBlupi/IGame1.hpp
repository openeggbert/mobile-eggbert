/**
 * @file IGame1.hpp
 * @brief Declares the IGame1 interface that exposes the top-level game object to subsystems.
 *
 * @details IGame1 is the minimal interface that Decor, InputPad, Pixmap, and Sound need to
 * call back into Game1 without creating circular header dependencies. It covers game-loop
 * lifecycle hooks, property accessors, phase/mission control, and drawing helpers.
 */

#pragma once

#include <any>

#include "Def.hpp"
#include "InputPad.hpp"
#include "Microsoft/Xna/Framework/Game.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "System/EventArgs.hpp"
#include "WindowsPhoneSpeedyBlupi/Config.hpp"

#define readonly mutable

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @class IGame1
     * @brief Interface for the top-level game object that drives the game loop.
     *
     * @details IGame1 exposes the portion of Game1's interface that subsystems (Decor,
     * InputPad, Pixmap, Sound) need to call back into the game layer. It provides
     * access to the graphics device, content manager, and game-mode flags, and
     * declares all game-loop lifecycle methods that the XNA/CNA Game base class
     * calls each frame.
     *
     * The concrete implementation is Game1, which owns:
     * - The graphics device manager (GraphicsDeviceManager).
     * - The Pixmap rendering subsystem.
     * - The Sound audio subsystem.
     * - The Decor gameplay simulation.
     * - The InputPad input handler.
     * - The GameData persistent state.
     *
     * @note The interface exists to allow subsystems to reference Game1 without
     *       creating a circular include dependency. Do not add gameplay logic here.
     */
    class IGame1
    {
    protected:
        ~IGame1() = default;

    public:
        /**
         * @brief Returns true when the game is currently displaying the ranking/high-score screen.
         * @return True if the ranking mode is active; false otherwise.
         */
        [[nodiscard]] virtual bool getIsRankingModeProperty() const = 0;

    public:
        /**
         * @brief Returns true when the game is running in trial (demo) mode.
         * @return True if trial mode is active; false for the full game.
         */
        [[nodiscard]] virtual bool getIsTrialModeProperty() const = 0;

    protected:
        /**
         * @brief Called once by the XNA/CNA framework after the graphics device is created.
         *
         * @details Initialises subsystems and loads configuration. Must be called before
         * LoadContent() and the first Update()/Draw() cycle.
         */
        virtual void Initialize() = 0;

        /**
         * @brief Loads all game content assets (textures, sounds, fonts).
         *
         * @details Called by the XNA/CNA framework after Initialize(). Delegates to
         * Pixmap::LoadContent() and Sound::LoadContent().
         */
        virtual void LoadContent() = 0;

        /**
         * @brief Releases all content assets loaded by LoadContent().
         *
         * @details Called by the XNA/CNA framework before the game exits or the graphics
         * device is reset. Must leave the game in a state from which LoadContent() can
         * be called again.
         */
        virtual void UnloadContent() = 0;

        /**
         * @brief Called by the XNA/CNA framework when the application loses focus.
         *
         * @details Used to pause audio, persist progress, and suppress input while the
         * application window is not in the foreground.
         *
         * @param[in] sender The object that raised the event (may be null).
         * @param[in] args   Event arguments (typically empty).
         */
        virtual void OnDeactivated(System::Object* sender, const System::EventArgs& args) = 0;

        /**
         * @brief Called by the XNA/CNA framework when the application regains focus.
         *
         * @details Used to resume audio and re-enable input after the application window
         * returns to the foreground.
         *
         * @param[in] sender The object that raised the event (may be null).
         * @param[in] args   Event arguments (typically empty).
         */
        virtual void OnActivated(System::Object* sender, const System::EventArgs& args) = 0;

        /**
         * @brief Called by the XNA/CNA framework just before the game process exits.
         *
         * @details Performs final cleanup (save state, release resources) that must
         * happen before the application terminates.
         *
         * @param[in] args Exit event arguments provided by the framework.
         */
        virtual void OnExiting(System::Object* sender, const System::EventArgs& args) = 0;

        /**
         * @brief Advances the game simulation by one frame.
         *
         * @details Called by the XNA/CNA framework at the configured update rate.
         * Drives the active Phase's update path (input, physics, AI, state machine).
         *
         * @param[in,out] gameTime Elapsed and total game time for this frame.
         */
        virtual void Update(Microsoft::Xna::Framework::GameTime& gameTime) = 0;

    private:
        /**
         * @brief Returns from the active mission to the main menu.
         *
         * @details Resets gameplay state and transitions the Phase back to Init.
         * Called when the player chooses "Menu" from the pause screen.
         */
        virtual void MissionBack() = 0;

        /**
         * @brief Starts a specific mission by index.
         *
         * @details Loads the level data for @p mission and transitions to the Play phase.
         *
         * @param[in] mission Zero-based mission index to start.
         */
        virtual void StartMission(int mission) = 0;

        /**
         * @brief Continues the most recently played mission from the saved checkpoint.
         *
         * @details Restores progress for the current gamer and transitions back to
         * the Play phase without restarting from the beginning of the level.
         */
        virtual void ContinueMission() = 0;

        /**
         * @brief Executes the cheat action associated with the given button glyph.
         *
         * @details Called when the player activates a cheat sequence. The exact effect
         * (unlock level, change speed, etc.) is determined by @p glyph.
         *
         * @param[in] glyph The cheat button that was activated.
         */
        virtual void CheatAction(Def::ButtonGlyph glyph) = 0;

    protected:
        /**
         * @brief Renders the current frame to the screen.
         *
         * @details Called by the XNA/CNA framework once per update cycle after Update().
         * Clears the back buffer, draws all layers, and presents.
         *
         * @param[in] gameTime Elapsed and total game time for this frame.
         */
        virtual void Draw(const Microsoft::Xna::Framework::GameTime& gameTime) = 0;

    private:
        /** @brief Draws a fade overlay on top of the background image for phase transitions. */
        virtual void DrawBackgroundFade() = 0;

        /** @brief Draws the background sprite for every visible UI button in the current phase. */
        virtual void DrawButtonsBackground() = 0;

        /** @brief Draws the text labels for every visible UI button in the current phase. */
        virtual void DrawButtonsText() = 0;

        /**
         * @brief Draws the gamer-name text on a specific button.
         *
         * @param[in] glyph The button whose text area receives the gamer name.
         * @param[in] gamer Zero-based gamer slot index (0, 1, or 2).
         */
        virtual void DrawButtonGamerText(Def::ButtonGlyph glyph, int gamer) = 0;

        /**
         * @brief Draws a localised string to the right of a button using a resource ID.
         *
         * @param[in] glyph The button to the left of the text.
         * @param[in] res   Resource string identifier.
         */
        virtual void DrawTextRightButton(Def::ButtonGlyph glyph, int res) = 0;

        /**
         * @brief Draws an explicit string to the right of a button.
         *
         * @param[in] glyph The button to the left of the text.
         * @param[in] text  String to display.
         */
        virtual void DrawTextRightButton(Def::ButtonGlyph glyph, std::string text) = 0;

        /**
         * @brief Draws a localised string below a button using a resource ID.
         *
         * @param[in] glyph The button above the text.
         * @param[in] res   Resource string identifier.
         */
        virtual void DrawTextUnderButton(Def::ButtonGlyph glyph, int res) = 0;

        /** @brief Draws an animated progress indicator during asynchronous Wait phases. */
        virtual void DrawWaitProgress() = 0;

        /** @brief Draws debug overlays (frame rate, state info) when debug mode is active. */
        virtual void DrawDebug() = 0;

        /**
         * @brief Selects the active gamer slot.
         *
         * @details Loads the saved progress for @p gamer and makes it the current profile.
         *
         * @param[in] gamer Zero-based gamer slot index (0, 1, or 2).
         */
        virtual void SetGamer(int gamer) = 0;

        /**
         * @brief Transitions the game to the specified phase.
         *
         * @param[in] phase The target Phase to transition to.
         */
        virtual void SetPhase(Def::Phase phase) = 0;

        /**
         * @brief Transitions the game to the specified phase with an associated mission index.
         *
         * @details Used when a phase transition also selects a specific level (e.g., starting Play).
         *
         * @param[in] phase   The target Phase to transition to.
         * @param[in] mission Zero-based mission index to associate with the new phase.
         */
        virtual void SetPhase(Def::Phase phase, int mission) = 0;

        /** @brief Persists the current gamer's progress to storage. */
        virtual void MemorizeGamerProgress() = 0;

    public:
        /** @brief Toggles between windowed and full-screen rendering modes. */
        virtual void ToggleFullScreen() = 0;

        /**
         * @brief Returns true if the game is currently running in full-screen mode.
         * @return True if full-screen is active; false for windowed mode.
         */
        virtual bool IsFullScreen() = 0;

#ifndef LEGACY
        /**
         * @brief Sets the simulation speed multiplier (MODERN builds only).
         *
         * @param[in] speed The desired GameSpeed preset.
         */
        virtual void SetGameSpeed(GameSpeed speed) = 0;

        /**
         * @brief Returns the current simulation speed multiplier (MODERN builds only).
         * @return The active GameSpeed value.
         */
        [[nodiscard]] virtual GameSpeed getGameSpeed() const = 0;
#endif

        /**
         * @brief Returns the graphics device manager owned by Game1.
         * @return A copy of the GraphicsDeviceManager instance.
         */
        virtual Microsoft::Xna::Framework::GraphicsDeviceManager getGraphics() = 0;

        /**
         * @brief Returns a reference to the XNA/CNA content manager used to load assets.
         * @return Reference to the ContentManager.
         */
        [[nodiscard]] virtual Microsoft::Xna::Framework::Content::ContentManager& getContentProperty() = 0;

        /**
         * @brief Returns a reference to the active graphics device.
         * @return Reference to the GraphicsDevice.
         */
        [[nodiscard]] virtual Microsoft::Xna::Framework::Graphics::GraphicsDevice& getGraphicsDeviceProperty() = 0;
    };
}
