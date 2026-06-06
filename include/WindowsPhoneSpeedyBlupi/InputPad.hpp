/**
 * @file InputPad.hpp
 * @brief Declaration of the InputPad player-input controller.
 *
 * @details
 * InputPad is the single point of contact between the platform input layer
 * (touch, mouse, keyboard, accelerometer) and the game state machine. It
 * translates raw hardware events into @c ButtonGlyph presses and directional
 * speed values each frame.
 *
 * Build-configuration variants:
 * - **LEGACY** – minimal build without typed-cheat or persistent-cheat
 *   support (no @c typedCheatBuffer, no @c activePersistentCheats).
 * - **MODERN** – full build that adds the debug overlay, virtual on-screen
 *   keyboard, Shift/Tab speed modifiers, and cheat-code cycling.
 * - **INPUT_DISABLED** (defined when @c INPUT_ENABLED is absent in the .cpp)
 *   – stubs out all processing; useful for automated testing.
 *
 * @see InputPad
 * @see Decor::KeyChange()
 * @see ISound::PlayImage()
 */
#pragma once

#ifdef MODERN
#include <string>
#include <vector>
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#endif

#include "Microsoft/Devices/Sensors/Accelerometer.hpp"
#include "Microsoft/Devices/Sensors/AccelerometerReading.hpp"
#include "WindowsPhoneSpeedyBlupi/Decor.hpp"
#include "WindowsPhoneSpeedyBlupi/IPixmap.hpp"
#include "WindowsPhoneSpeedyBlupi/Slider.hpp"
#include "WindowsPhoneSpeedyBlupi/ISound.hpp"
#include "WindowsPhoneSpeedyBlupi/def/Zoom.hpp"

/// @cond INTERNAL
#define VECTOR_CONTAINS(vector, element) count( vector .begin(), vector. end(), element );
/// @endcond

namespace WindowsPhoneSpeedyBlupi
{
    class IGame1;

    /**
     * @class InputPad
     * @brief Handles all player input for the game, including touch, keyboard, and accelerometer.
     *
     * @details
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
     * - (MODERN) Displaying persistent-cheat name labels in the top-left corner.
     * - (MODERN) Providing a virtual on-screen keyboard overlay for devices
     *   without a physical keyboard.
     *
     * Does not own gameplay state. This is input code, not gameplay logic.
     *
     * @note On non-touchscreen devices, on-screen buttons may be hidden depending on
     *       Config::TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE.
     * @note The directional pad uses a circular detection zone of radius @c padRadius
     *       (140 HUD-space pixels). When accelerometer mode is active the directional
     *       pad is hidden and touch-pad input is ignored; only the Down and Jump
     *       buttons remain active.
     * @note Accelerometer callbacks arrive on a sensor thread. The shared mutable
     *       fields @c accelSpeedX and @c accelLastState are written by that thread
     *       and read by the main game thread in Update(). No explicit mutex is used;
     *       safety relies on aligned-double access being atomic on the target
     *       architectures (ARM, x86).
     * @warning Do not call Update() or Draw() before constructing with all non-null
     *          pointers; several methods dereference all five dependency pointers
     *          unconditionally.
     * @see Decor::SetSpeedX()
     * @see Decor::SetSpeedY()
     * @see Decor::KeyChange()
     */
    class InputPad
    {
        /** @brief Radius of the directional pad touch zone in HUD-space pixels.
         *  @details The hit-test uses a square bounding box around the circular
         *  pad rather than a true Euclidean distance test. */
        static constexpr int padRadius = 140;

        mutable IGame1* game1;   ///< @brief Non-owning pointer to the game host. Never null after construction.

        mutable Decor* decor;    ///< @brief Non-owning pointer to the gameplay decorator. Never null after construction.

        mutable IPixmap* pixmap; ///< @brief Non-owning pointer to the pixmap renderer used by Draw(). Never null after construction.

        mutable ISound* sound;   ///< @brief Non-owning pointer to the sound subsystem for UI click sounds. Never null after construction.

        mutable GameData* gameData; ///< @brief Non-owning pointer to persistent game configuration and state. Never null after construction.

        /** Set of button glyphs currently held down this frame. */
        mutable std::vector<Def::ButtonGlyph> pressedGlyphs;

        mutable Microsoft::Devices::Sensors::Accelerometer accelSensor; ///< @brief Platform accelerometer sensor object. Started/stopped by StartAccel()/StopAccel().

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

#ifndef LEGACY
        /** Buffer accumulating typed characters to detect cheat code names. */
        std::string typedCheatBuffer;

        /** Names of currently active persistent cheats, displayed top-left during Play. */
        std::vector<std::string> activePersistentCheats;

        /** Debounce state for letter keys A-Z (index 0=A, 25=Z). */
        bool letterPrev[26] = {};
#endif
#ifdef MODERN
        /** True when the debug overlay cheat is currently enabled. */
        bool debug_cheat_enabled = false;
        /** True when the quick cheat is currently enabled. */
        bool quick_cheat_enabled = false;
        /** True when the ghost cheat is currently enabled (mirrors decor->IsGhost()). */
        bool ghost_cheat_enabled = false;
        /** Current zoom cheat state. */
        ZoomCheat zoom_cheat_state = ZoomCheat::Zoom100;
        int cheats_display_timer = 0;  ///< Frames remaining for the cheats overlay. 0 = hidden.
        /** True when Shift was held in the previous frame (for edge detection). */
        bool shift_held_previously = false;
        /** Game speed saved before Shift was pressed, restored when Shift is released. */
        GameSpeed game_speed_before_shift = GameSpeed::Normal;
        /** True when Tab was held in the previous frame (for edge detection). */
        bool tab_held_previously = false;

        // --- Virtual on-screen keyboard (MODERN only) ---

        /**
         * @brief Immutable layout snapshot for the virtual on-screen keyboard.
         *
         * @details
         * GetVirtualKeyboardLayout() computes all geometry from the current
         * draw-bounds and returns this struct. Both Update() and Draw() must
         * call GetVirtualKeyboardLayout() independently and use the returned
         * value; the struct is not cached as a member to avoid stale data when
         * the screen is resized between frames.
         *
         * All coordinate values are in HUD-space pixels (with @c origin
         * applied where noted), consistent with the coordinate space used by
         * TinyRect hit-tests throughout InputPad.
         *
         * @note Key width and height are equal (square keys). Key count per
         *       row follows the standard QWERTY layout: F-row (4 left + 1
         *       right), QWERTY (10), ASDF (9), ZXCV (7).
         */
        struct VirtualKeyboardLayout
        {
            int keyW   = 0; ///< @brief Width (and height) of a single key in screen pixels.
            int keyH   = 0; ///< @brief Height of a single key; always equals @c keyW (square keys).
            int keyGap = 0; ///< @brief Horizontal gap between adjacent keys in the same row, in pixels.
            int rowGap = 0; ///< @brief Vertical gap between keyboard rows, in pixels. Equals @c keyGap.
            int panelX = 0; ///< @brief Left edge of the key area relative to the screen origin (before @c origin offset).
            int panelY = 0; ///< @brief Top edge of the key area relative to the screen origin (before @c origin offset).
            int panelW = 0; ///< @brief Total width of the key area (10 keys wide), excluding the @c origin offset.
            int panelH = 0; ///< @brief Total height of the key area (4 rows + F-row), excluding the @c origin offset.
            int f12OffsetX = 0; ///< @brief Horizontal offset for the lone F12 key so it aligns one slot left of the close button.
            TinyRect panelRect;  ///< @brief Screen-space bounding rectangle of the keyboard panel background (includes 4px padding, origin applied).
            TinyRect closeRect;  ///< @brief Screen-space bounding rectangle of the close (X) button (origin applied, clamped inside @c panelRect).
            TinyPoint origin;    ///< @brief Pixmap origin offset that must be added to all logical HUD coordinates to get screen coordinates.
        };

        /**
         * @brief Computes the virtual keyboard layout from the current screen dimensions.
         *
         * @details Derives all geometry from @c pixmap->getDrawBoundsProperty() so
         * the keyboard scales correctly on any screen size. The layout is computed
         * fresh on every call; callers should store it in a local variable for the
         * duration of a single frame.
         *
         * @return A fully populated VirtualKeyboardLayout struct.
         * @note Both Update() and Draw() call this independently; they must not share
         *       a cached instance across frames.
         */
        [[nodiscard]] VirtualKeyboardLayout GetVirtualKeyboardLayout() const;

        /** True when the virtual keyboard overlay is visible. */
        bool virtualKeyboardVisible = false;
        /** Number of consecutive frames the activation area has been held. */
        int virtualKeyboardHoldFrames = 0;
        /** True after the keyboard has been shown once for the current hold gesture. */
        bool virtualKeyboardActivationConsumed = false;
        /** Virtual key presses injected this frame by on-screen keyboard taps. */
        std::vector<Microsoft::Xna::Framework::Input::Keys> virtualKeysPressedThisFrame;
#endif

    public:
        /// @brief Current game phase; controls which button glyphs are active.
        DDATA(Def::Phase, Phase)
        /// @brief Index of the currently selected gamer slot (0-2).
        DDATA(int, SelectedGamer)
        /// @brief Pixmap draw-origin offset used for HUD-to-screen coordinate translation.
        DDATA(TinyPoint, PixmapOrigin)

        /**
         * @brief Returns the total number of touch or mouse-click contact points
         *        recorded in the most recent Update() call.
         *
         * @details The count is the sum of active touch-panel contacts and any
         *          mouse left-button press. Keyboard events do not contribute.
         * @return Number of simultaneous touch/click contacts (>= 0).
         */
        [[nodiscard]] int getTotalTouchOrClickProperty() const;

        /**
         * @brief Consumes and returns the button glyph that was pressed and released
         *        during the current frame.
         *
         * @details A button is "pressed" when it was held in the previous frame
         *          (lastButtonDown) but is no longer held in the current frame.
         *          The value is consumed on read: a subsequent call in the same
         *          frame returns ButtonGlyph::None.
         *
         * @return The ButtonGlyph that completed a press-release cycle, or
         *         ButtonGlyph::None if no button was released this frame.
         */
        [[nodiscard]] Def::ButtonGlyph getButtonPressedProperty() const;

        /// @brief Whether the cheat-code overlay menu is currently visible.
        DDATA(bool, ShowCheatMenu)

        /**
         * @brief Returns the ordered list of button glyphs that should be rendered
         *        and hit-tested for the current game phase.
         *
         * @details The list is rebuilt each call from the current phase, mission
         *          index, and trial/ranking mode flags. Cheat-menu glyphs are
         *          appended when @c showCheatMenu is @c true.
         *
         * @return Vector of ButtonGlyph values active in the current frame.
         *         The order determines hit-test priority: later entries take
         *         precedence (ButtonDetect iterates in reverse).
         */
        [[nodiscard]] std::vector<Def::ButtonGlyph> getButtonGlyphsProperty() const;

        /**
         * @brief Returns the screen-space centre point of the directional pad.
         *
         * @details The pad is positioned near the bottom of the draw area.
         *          Its X position depends on @c gameData->getJumpRightProperty():
         *          when @c true the pad is on the left at x=100; when @c false
         *          it is on the right at x = drawBoundsWidth - 100.
         *          Y is always drawBoundsHeight - 100.
         *
         * @return Centre of the directional pad in HUD-space pixels.
         */
        [[nodiscard]] TinyPoint getPadCenterProperty() const;

        /**
         * @brief Constructs the InputPad and binds it to the game, decor, pixmap,
         *        sound, and game-data objects.
         *
         * @details Registers the accelerometer change callback on @c accelSensor but
         *          does not start the sensor; call StartMission() before the first
         *          Update() to arm the sensor when the user has it enabled.
         *          Initialises @c lastButtonDown and @c buttonPressed to
         *          ButtonGlyph::None.
         *
         * @param[in] game1     Non-owning pointer to the game host; must not be null.
         * @param[in] decor     Non-owning pointer to the gameplay decorator; must not be null.
         * @param[in] pixmap    Non-owning pointer to the pixmap renderer; must not be null.
         * @param[in] sound     Non-owning pointer to the sound subsystem; must not be null.
         * @param[in] gameData  Non-owning pointer to persistent game configuration; must not be null.
         *
         * @pre All pointer arguments are non-null and remain valid for the lifetime
         *      of this InputPad instance.
         */
        InputPad(IGame1* game1, Decor* decor, IPixmap* pixmap, ISound* sound, GameData* gameData);

        /**
         * @brief Prepares the input pad for the start of a new mission.
         *
         * @details Stores the mission index (used by getButtonGlyphsProperty() to
         *          decide which pause-menu buttons to show) and sets @c accelWaitZero
         *          to @c true so the accelerometer waits for the device to return to
         *          neutral before applying tilt movement.
         *
         * @param[in] mission Mission/level index being started (1-based).
         * @post accelWaitZero == true until the first near-zero accelerometer reading.
         */
        void StartMission(int mission);

        /**
         * @brief Processes all input events for the current frame.
         *
         * @details
         * Each frame Update() performs the following steps in order:
         * -# Synchronises the accelerometer start/stop state with
         *    @c gameData->getAccelActiveProperty().
         * -# Collects all active touch contacts and mouse-left-button press into
         *    a unified @c touchesOrClicks list.
         * -# On Android with aspect ratio > 4:3, remaps touch coordinates from
         *    screen space to the fixed 640x480 HUD space.
         * -# Encodes special keyboard keys (arrow keys, Ctrl, Space, Escape) as
         *    synthetic touch points with @c X == -1 and @c Y == key-enum value.
         * -# (MODERN) Handles the virtual on-screen keyboard activation hold
         *    gesture and key-tap detection; consumes matching touches.
         * -# (MODERN) Processes cheat-related key bindings (F5-F8 game speed,
         *    F11 full-screen toggle, F12 cheat-menu toggle, Shift speed boost,
         *    Tab slow-motion toggle) and typed cheat-code detection.
         * -# For each remaining touch/click, calls ButtonDetect() to identify
         *    the glyph and accumulates direction flags for the D-pad.
         * -# Converts pad touch position to @c horizontalChange / @c verticalChange
         *    direction values and passes them to Decor::SetSpeedX() / SetSpeedY().
         * -# Builds a @c keyPress bitmask (Jump, Down) and calls Decor::KeyChange().
         * -# Implements press-release edge detection: @c buttonPressed is set when
         *    a glyph transitions from held to released.
         *
         * @pre StartMission() has been called at least once before the first
         *      invocation in the Play phase.
         * @post Decor::SetSpeedX(), SetSpeedY(), and KeyChange() have been called
         *       with the values derived from this frame's input.
         * @note Must be called once per game update frame, before Draw().
         * @warning Calling Update() from multiple threads concurrently is not safe.
         */
        void Update();

    private:
        /**
         * @brief Identifies which ButtonGlyph was hit by the given touch/click position.
         *
         * @details Iterates the active glyph list in reverse order (last entry has
         *          highest priority) and returns the first glyph whose bounding
         *          rectangle contains @p touchOrClick.  Action buttons (PlayJump,
         *          PlayAction, PlayDown, PlayPause) have their rectangles inflated
         *          by 20 pixels on each side to improve touch accuracy.
         *
         * @param[in] touchOrClick Position in HUD-space pixels to test. Keyboard
         *            synthetic events (X == -1) always test against TinyPoint(1,1)
         *            and will not match any real button rectangle.
         * @return The matched ButtonGlyph, or ButtonGlyph::None if no button
         *         contains the given point.
         */
        Def::ButtonGlyph ButtonDetect(TinyPoint touchOrClick);

    public:
        /**
         * @brief Draws the on-screen button overlay for the current game phase.
         *
         * @details
         * Renders in this order:
         * -# Directional pad background and thumb sprites (Play phase only, when
         *    accelerometer is inactive).
         * -# All active ButtonGlyph sprites via @c pixmap->DrawInputButton(), with
         *    pressed/selected highlight state.
         * -# Accelerometer sensitivity slider (Setup phases when accel is enabled).
         * -# (MODERN) Persistent-cheat name labels in the top-left corner.
         * -# (MODERN) Debug overlay panel in the top-right corner when
         *    @c debug_cheat_enabled is @c true.
         * -# (MODERN) Full cheat-reference overlay for @c cheats_display_timer frames.
         * -# (MODERN) Current game-speed label at the bottom-left when speed is not
         *    Normal.
         * -# (MODERN) Virtual on-screen keyboard panel when @c virtualKeyboardVisible
         *    is @c true.
         *
         * On non-touchscreen devices, buttons may be hidden per
         * Config::TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE.
         *
         * @note Must be called after Update() in the same frame so that
         *       @c pressedGlyphs reflects the current frame's input.
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
        /**
         * @brief Starts the hardware accelerometer sensor.
         *
         * @details Calls @c accelSensor.Start() inside a try-catch block.
         * Sets @c accelStarted to @c true on success. Both
         * AccelerometerFailedException and UnauthorizedAccessException are
         * silently caught so that gameplay continues without tilt control when
         * the sensor is unavailable or the app lacks permission.
         *
         * @post accelStarted == true iff the sensor started without throwing.
         * @warning The sensor may begin delivering callbacks on a sensor thread
         *          immediately after Start() returns.
         */
        void StartAccel();

        /**
         * @brief Stops the hardware accelerometer sensor.
         *
         * @details No-op if @c accelStarted is already @c false. Otherwise calls
         * @c accelSensor.Stop() (silently discarding AccelerometerFailedException)
         * and sets @c accelStarted to @c false.
         *
         * @post accelStarted == false.
         */
        void StopAccel();

        /**
         * @brief Sensor callback: converts a raw accelerometer reading into a
         *        lateral movement speed.
         *
         * @details
         * Uses only the Y component of the acceleration vector.  Applies a
         * configurable dead-zone (see @c accelWaitZero, @c accelLastState) and
         * a linear speed ramp.  The result is stored in @c accelSpeedX which is
         * read by Update() on the next game frame.
         *
         * Dead-zone threshold formula:
         * @code
         *   sensitivityThreshold = (1 - accelSensitivity) * 0.06 + 0.04
         * @endcode
         * When the previous sample was active, the threshold is reduced to
         * @code
         *   adjustedThreshold = sensitivityThreshold * 0.6  // hysteresis
         * @endcode
         *
         * Speed ramp (when |y| > adjustedThreshold):
         * @code
         *   speed = clamp(|y| * 0.25 / sensitivityThreshold + 0.25, 0.0, 1.0)
         * @endcode
         *
         * Positive Y maps to left movement (negative @c accelSpeedX);
         * negative Y maps to right movement (positive @c accelSpeedX).
         *
         * @param[in] e Sensor-reading event from the accelerometer framework.
         *
         * @note Runs on the sensor thread. Only @c accelSpeedX and
         *       @c accelLastState are modified; both are aligned scalars.
         * @see HandleAccelSensorCurrentValueChanged implementation notes in
         *      InputPad.cpp for the rationale on the thread-safety approach.
         */
        void HandleAccelSensorCurrentValueChanged(
            Microsoft::Devices::Sensors::SensorReadingEventArgs<Microsoft::Devices::Sensors::AccelerometerReading> e);
    };
}
