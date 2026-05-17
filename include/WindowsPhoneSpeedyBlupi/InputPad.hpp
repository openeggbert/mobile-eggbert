#pragma once

#include "Microsoft/Devices/Sensors/Accelerometer.hpp"
#include "Microsoft/Devices/Sensors/AccelerometerReading.hpp"
#include "WindowsPhoneSpeedyBlupi/Decor.hpp"
#include "WindowsPhoneSpeedyBlupi/IPixmap.hpp"
#include "WindowsPhoneSpeedyBlupi/Slider.hpp"
#include "WindowsPhoneSpeedyBlupi/ISound.hpp"

#define VECTOR_CONTAINS(vector, element) count( vector .begin(), vector. end(), element );

namespace WindowsPhoneSpeedyBlupi
{
    class IGame1;

    /**
     * @brief Handles all player input for the game, including touch, keyboard, and accelerometer.
     *
     * InputPad translates raw input events from the platform (touch, mouse clicks,
     * keyboard presses, accelerometer readings) into logical ButtonGlyph presses that
     * are consumed by the game phase state machine in Game1 and the gameplay logic
     * in Decor via KeyChange().
     *
     * Responsibilities:
     * - Detecting which on-screen button (ButtonGlyph) was tapped/clicked.
     * - Drawing the on-screen touch button overlay (directional pad, action buttons).
     * - Managing accelerometer state for tilt-based left/right movement.
     * - Tracking the current game phase to show the correct button set.
     * - Forwarding movement input to Decor::SetSpeedX() / Decor::SetSpeedY().
     *
     * Does not own gameplay state. This is input code, not gameplay logic.
     *
     * @note On non-touchscreen devices, on-screen buttons may be hidden depending on
     *       Config::TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE.
     * @note The directional pad uses a circular detection zone of radius padRadius.
     */
    class InputPad
    {
        /** Radius of the directional pad touch zone in HUD-space pixels. */
        static constexpr int padRadius = 140;

        mutable IGame1* game1;

        mutable Decor* decor;

        mutable IPixmap* pixmap;

        mutable ISound* sound;

        mutable GameData* gameData;

        /** Set of button glyphs currently held down this frame. */
        mutable std::vector<Def::ButtonGlyph> pressedGlyphs;

        mutable Microsoft::Devices::Sensors::Accelerometer accelSensor;

        /** UI slider used to adjust accelerometer sensitivity in the setup screen. */
        mutable Slider accelSlider;

        /** True if the directional pad touch zone is currently being pressed. */
        bool padPressed = false;

        /** True if the cheat-code input overlay is currently visible. */
        bool showCheatMenu = false;

        /** Current touch/click position inside the pad zone, in HUD-space. */
        TinyPoint padTouchPos;

        /** The last button glyph for which a pointer-down event was recorded. */
        Def::ButtonGlyph lastButtonDown;

        /** The button glyph that is considered pressed for this frame. */
        mutable Def::ButtonGlyph buttonPressed;

        /** Running count of all touch or click events since the game started. */
        int touchOrClickCount = 0;

        /** True once the accelerometer sensor has been started. */
        bool accelStarted = false;;

        /** True if accelerometer-based movement is currently active. */
        bool accelActive = false;

        /** Accumulated horizontal speed from accelerometer tilt, in game units per frame. */
        double accelSpeedX = 0.0f;

        /** Stores whether the accelerometer was active in the previous frame. */
        bool accelLastState = false;

        /** True while waiting for the accelerometer to return to near-zero before activating. */
        bool accelWaitZero = false;

        /** Index of the current mission (level). Used to configure the button layout. */
        int mission = 0;

    public:
        DDATA(Def::Phase, Phase)
        DDATA(int, SelectedGamer)
        DDATA(TinyPoint, PixmapOrigin)

        [[nodiscard]] int getTotalTouchOrClickProperty() const;

        [[nodiscard]] Def::ButtonGlyph getButtonPressedProperty() const;
        DDATA(bool, ShowCheatMenu)

        [[nodiscard]] std::vector<Def::ButtonGlyph> getButtonGlyphsProperty() const;
        // Returns the point of the center of the pad on the screen.
        [[nodiscard]] TinyPoint getPadCenterProperty() const;

        /**
         * @brief Constructs the InputPad and binds it to the game, decor, pixmap, sound, and data objects.
         *
         * Does not start the accelerometer; call StartMission() before the first Update().
         */
        InputPad(IGame1* game1, Decor* decor, IPixmap* pixmap, ISound* sound, GameData* gameData);

        /**
         * @brief Prepares the input pad for the start of a new mission.
         *
         * Stores the mission index, resets accelerometer state, and configures the
         * button layout for gameplay phase.
         *
         * @param mission Mission/level index being started.
         */
        void StartMission(int mission);

        /**
         * @brief Processes all input events for the current frame.
         *
         * Polls touch/mouse/keyboard/accelerometer state, determines which buttons
         * are pressed, updates Decor movement speed via SetSpeedX()/SetSpeedY(),
         * and fires KeyChange() for action/jump/down buttons.
         *
         * Must be called once per game update frame before Draw().
         */
        void Update();

    private:
        /**
         * @brief Identifies which ButtonGlyph was hit by the given touch/click position.
         * @param touchOrClick Position in HUD-space.
         * @return The detected ButtonGlyph, or ButtonGlyph::None if no button was hit.
         */
        Def::ButtonGlyph ButtonDetect(TinyPoint touchOrClick);

    public:
        /**
         * @brief Draws the on-screen button overlay for the current game phase.
         *
         * Renders the directional pad and action buttons appropriate to the current
         * phase (play, pause, menu, setup, etc.). On non-touchscreen devices, buttons
         * may be hidden per Config::TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE.
         */
        void Draw();

    private:
        /**
         * @brief Returns the bounding rectangle of the circular pad zone.
         * @param center Center of the pad in HUD-space.
         * @param radius Radius of the pad zone in HUD-space pixels.
         * @return Bounding rectangle enclosing the circular zone.
         */
        TinyRect GetPadBounds(TinyPoint center, int radius);

    public:
        /**
         * @brief Returns the screen-space bounding rectangle of a specific button glyph.
         *
         * Used by external code (e.g., Game1 drawing methods) to position text labels
         * next to or under a button.
         *
         * @param glyph The button whose rectangle is requested.
         * @return Rectangle of the button in HUD-space, or an empty rect if not visible.
         */
        TinyRect GetButtonRect(Def::ButtonGlyph glyph);

    private:
        void StartAccel();

        void StopAccel();

        void HandleAccelSensorCurrentValueChanged(
            Microsoft::Devices::Sensors::SensorReadingEventArgs<Microsoft::Devices::Sensors::AccelerometerReading> e);
    };
}
