/**
 * @file Jauge.hpp
 * @brief Declarations for the JaugeMode enumeration and the Jauge HUD widget.
 * @details The Jauge class renders a 124 × 22-pixel gauge bar used as energy,
 *          time, and key indicators in the game HUD.  Four visual modes are
 *          supported via the JaugeMode enum.
 */

#pragma once

#include "IPixmap.hpp"
#include "ISound.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    /**
     * @brief Selects the colour/style of the filled portion of a Jauge widget.
     *
     * @details Each enumerator maps to a 22-pixel-high row in the Jauge sprite sheet
     *          (PixmapChannel::Jauge).  The row index equals the raw integer value.
     */
    enum class JaugeMode : intcs
    {
        Empty  = 0, ///< @brief No fill drawn (gauge is empty).
        Red    = 1, ///< @brief Red fill — used for danger / energy indicators.
        Blue   = 2, ///< @brief Blue fill — used for water level indicators.
        Yellow = 3  ///< @brief Yellow fill — used for charge / key indicators.
    };

    /**
     * @brief Converts a JaugeMode to its underlying integer value.
     * @param[in] mode Mode to convert.
     * @return Raw @c intcs value of @p mode.
     */
    inline intcs ToRaw(JaugeMode mode)
    {
        return static_cast<intcs>(mode);
    }

    /**
     * @brief Converts a raw integer to a JaugeMode.
     * @param[in] mode Raw integer value (0–3).
     * @return Corresponding JaugeMode enumerator.
     * @warning Passing a value outside 0–3 produces an undefined enumerator.
     */
    inline JaugeMode ToJaugeMode(intcs mode)
    {
        return static_cast<JaugeMode>(mode);
    }

    /**
     * @class Jauge
     * @brief HUD gauge widget that renders a 124 × 22-pixel progress bar.
     *
     * @details Jauge draws a two-layer sprite: first the empty gauge background
     *          (full 124 × 22 region from PixmapChannel::Jauge), then, if the
     *          current level is greater than zero, a filled sub-region whose width
     *          is proportional to the level (0–100 mapped to 0–114 pixels).  The
     *          filled region starts after a fixed 6-pixel left border.
     *
     *          A redraw-dirty flag (@c m_bRedraw) avoids redundant GPU work: when
     *          @c bMinimizeRedraw is enabled in Create(), Draw() returns immediately
     *          unless the flag has been set by a state-changing method (SetLevel(),
     *          SetMode(), SetHide(), Redraw(), or SetRedraw()).
     *
     * @note Gauge level is always clamped to [0, 100].
     * @note The @c ISound pointer is stored for API compatibility with the original
     *       C# code but is not used by any current implementation.
     * @note Status: IMPLEMENTED
     *
     * @see JaugeMode
     */
    class Jauge
    {
        IPixmap* m_pixmap;          ///< @brief Pixmap renderer used to draw the gauge (not owned).
        ISound*  m_sound;           ///< @brief Sound interface — stored for compatibility, currently unused.
        bool     m_bHide;           ///< @brief When @c true the gauge is not rendered even if Draw() is called.
        TinyPoint m_pos;            ///< @brief Top-left position of the gauge in HUD-space.
        TinyPoint m_dim;            ///< @brief Fixed sprite dimensions: always 124 × 22 pixels.
        JaugeMode m_mode;           ///< @brief Current colour mode; selects the sprite-sheet row for the fill.
        int       m_level;          ///< @brief Current fill level clamped to [0, 100].
        bool      m_bMinimizeRedraw;///< @brief When @c true, Draw() skips unless @c m_bRedraw is set.
        bool      m_bRedraw;        ///< @brief Dirty flag — set when state changes, cleared after each Draw().
        double    m_zoom;           ///< @brief Zoom factor applied when calling IPixmap::DrawPart().

    public:
        /**
         * @brief Returns the current drawing zoom factor.
         * @return Current zoom value.
         * @note Status: IMPLEMENTED
         */
        [[nodiscard]] double getZoomProperty() const;

        /**
         * @brief Sets the drawing zoom factor.
         * @param[in] v New zoom value.
         * @note Status: IMPLEMENTED
         */
        void setZoomProperty(double v);

        /**
         * @brief Default constructor — creates an uninitialised gauge.
         *
         * @details Initialises all pointers to @c nullptr, hides the gauge, resets
         *          the level to 0, and sets the zoom to 1.0.  Call Create() before
         *          using the gauge.
         *
         * @note Status: IMPLEMENTED
         */
        Jauge();

        /**
         * @brief Initialises the gauge with rendering dependencies and display settings.
         *
         * @details Stores the pixmap and sound interfaces, records the position and
         *          mode, sets fixed sprite dimensions to 124 × 22, hides the gauge,
         *          resets the level to 0, and sets the dirty flag so the first Draw()
         *          always renders.
         *
         * @param[in] pixmap         Pixmap renderer used to draw the gauge.
         * @param[in] sound          Sound interface (stored for API compatibility; not used).
         * @param[in] pos            Top-left position of the gauge in HUD-space.
         * @param[in] mode           Sprite row / colour mode for the filled portion.
         * @param[in] bMinimizeRedraw If @c true, Draw() is a no-op unless @c m_bRedraw is set.
         * @return Always @c true.
         *
         * @note Status: IMPLEMENTED
         */
        bool Create(IPixmap* pixmap, ISound* sound, TinyPoint pos, JaugeMode mode, bool bMinimizeRedraw);

        /**
         * @brief Draws the gauge to the screen if visible and the dirty flag is set.
         *
         * @details When @c m_bMinimizeRedraw is @c true and @c m_bRedraw is @c false,
         *          the method returns immediately without issuing any draw calls
         *          (redraw-dirty optimisation).  Otherwise it clears the dirty flag
         *          and, if the gauge is visible, draws:
         *          1. The full 124 × 22 empty gauge background.
         *          2. If level > 0: the filled sub-region @c [0, 6 + filledWidth] ×
         *             [mode*22, (mode+1)*22], where @c filledWidth = level * 114 / 100.
         *
         * @note Status: IMPLEMENTED
         */
        void Draw();

        /**
         * @brief Sets the dirty flag so the next Draw() call redraws the gauge.
         * @note Status: IMPLEMENTED
         */
        void Redraw();

        /**
         * @brief Returns the current fill level.
         * @return Level in [0, 100].
         * @note Status: IMPLEMENTED
         */
        int GetLevel();

        /**
         * @brief Sets the fill level, clamping to [0, 100].
         *
         * @details If the new value differs from the current level, sets the dirty flag.
         *
         * @param[in] level Desired fill level (clamped to [0, 100]).
         * @note Status: IMPLEMENTED
         */
        void SetLevel(int level);

        /**
         * @brief Returns the current colour mode.
         *
         * @details The mode selects the 22-pixel-high sprite-sheet row used for
         *          the filled portion of the gauge.
         *
         * @return Current JaugeMode.
         * @note Status: IMPLEMENTED
         */
        JaugeMode GetMode();

        /**
         * @brief Sets the colour mode.
         *
         * @details If the mode changes, sets the dirty flag.
         *
         * @param[in] mode New JaugeMode.
         * @note Status: IMPLEMENTED
         */
        void SetMode(JaugeMode mode);

        /**
         * @brief Sets the colour mode from a raw integer.
         *
         * @details Converts @p mode via ToJaugeMode() and delegates to
         *          SetMode(JaugeMode).  If the mode changes, sets the dirty flag.
         *
         * @param[in] mode Raw integer mode value (0–3).
         * @note Status: IMPLEMENTED
         */
        void SetMode(int mode)
        {
            SetMode(ToJaugeMode(mode));
        }

        /**
         * @brief Returns whether the gauge is currently hidden.
         * @return @c true if hidden; @c false if visible.
         * @note Status: IMPLEMENTED
         */
        bool GetHide();

        /**
         * @brief Shows or hides the gauge.
         *
         * @details If the visibility state changes, sets the dirty flag.
         *
         * @param[in] bHide @c true to hide; @c false to show.
         * @note Status: IMPLEMENTED
         */
        void SetHide(bool bHide);

        /**
         * @brief Returns the top-left position of the gauge in HUD-space.
         * @return Current position.
         * @note Status: IMPLEMENTED
         */
        [[nodiscard]] TinyPoint GetPos() const;

        /**
         * @brief Sets the dirty flag so the next Draw() call redraws the gauge.
         *
         * @details Equivalent to Redraw().  Provided for symmetry with other
         *          property-style setters.
         *
         * @note Status: IMPLEMENTED
         */
        void SetRedraw();
    };
}
