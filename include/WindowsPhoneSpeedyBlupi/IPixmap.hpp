/**
 * @file IPixmap.hpp
 * @brief Declares the IPixmap interface abstracting all sprite/rendering operations for the game.
 *
 * @details Provides the pure-virtual API for loading texture atlases, coordinate conversion,
 * sprite drawing, background rendering, and optional batch-draw optimisation. The concrete
 * implementation is Pixmap.
 */

#pragma once

#include "Def.hpp"
#include "def/PixmapChannel.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyRect.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /**
     * @class IPixmap
     * @brief Interface for the game's sprite/rendering subsystem.
     *
     * @details IPixmap abstracts all drawing operations needed by the gameplay and UI layers.
     * A concrete implementation (Pixmap) loads texture atlases, manages viewport
     * geometry, and issues draw calls via the XNA/CNA SpriteBatch.
     *
     * Responsibilities:
     * - Loading all texture atlases (sprite sheets) from content.
     * - Converting between game-space, HUD-space, and screen-space coordinates.
     * - Drawing sprites by channel (PixmapChannel) and icon index.
     * - Drawing partial texture regions (DrawPart) and full icon slots (DrawIcon).
     * - Drawing the scrolling level background.
     * - Drawing on-screen input buttons (touch overlay).
     *
     * Does not own or modify gameplay state. Pure rendering/resource code.
     *
     * Coordinate conventions used by this interface:
     * - Game-space (logical): 640x480 base resolution, origin top-left.
     * - HUD-space: same as game-space but offset by the pixmap origin (for viewport centering).
     * - Screen-space: physical pixels after zoom/origin transform applied by the implementation.
     *
     * @note Do not cache positions returned by getOriginProperty() across frames;
     *       they may change when the window is resized.
     */
    class IPixmap
    {
    protected:
        virtual ~IPixmap() = default;

    public:
        /** @brief Returns the bounding rectangle of the drawable area in screen-space. */
        [[nodiscard]] virtual TinyRect getDrawBoundsProperty() = 0;

        /** @brief Returns the top-left origin of the game viewport in screen-space pixels. */
        [[nodiscard]] virtual TinyPoint getOriginProperty() const = 0;

        /**
         * @brief Converts a point from hotspot-scaled coordinates to HUD coordinates.
         * @param[in] pos Point in hotspot-scaled space.
         * @return Corresponding HUD-space point.
         */
        virtual TinyPoint HotSpotToHud(TinyPoint pos) = 0;

        /**
         * @brief Configures the hotspot zoom and center used by HotSpotToHud().
         * @param[in] zoom Hotspot zoom factor (1.0 = no scaling).
         * @param[in] x    X coordinate of the hotspot center in game-space.
         * @param[in] y    Y coordinate of the hotspot center in game-space.
         */
        virtual void SetHotSpot(double zoom, double x, double y) = 0;

        /**
         * @brief Draws an on-screen touch input button at the specified position.
         *
         * @details Renders the button background sprite for @p glyph in either its normal,
         * pressed, or selected visual state. Used by the touch-pad overlay.
         *
         * @param[in] rect     Destination rectangle in screen-space.
         * @param[in] glyph    The logical button to render.
         * @param[in] pressed  True if the button is currently held down.
         * @param[in] selected True if the button is in the selected/toggled state.
         */
        virtual void DrawInputButton(TinyRect rect, Def::ButtonGlyph glyph, bool pressed, bool selected) = 0;

        /**
         * @brief Loads all texture resources and creates the sprite batch.
         *
         * @details Must be called before any drawing method is used. Calls BackgroundCache()
         * for the default background and allocates the SpriteBatch.
         *
         * @note Status: PARTIAL
         */
        virtual void LoadContent() = 0;

        /**
         * @brief Loads (or switches to) a background texture by name.
         *
         * @details The background is used by DrawBackground() to fill the game area.
         * Called when a level is loaded or the background changes between worlds.
         *
         * @param[in] name Asset name of the background texture (without file extension).
         */
        virtual void BackgroundCache(const std::string& name) = 0;

        /**
         * @brief Begins a frame by clearing the graphics device.
         *
         * @details Clears the back buffer to the background colour. Call once at the
         * start of Draw() before any other draw calls.
         *
         * @return Always true.
         *
         * @note Status: PARTIAL
         */
        virtual bool Start() = 0;

        /**
         * @brief Ends a frame.
         *
         * @details Currently a placeholder because SpriteBatch drawing is begun and ended
         * inside each draw call. Call once at the end of Draw() after all draw calls.
         *
         * @return Always true.
         *
         * @note Status: STUB
         */
        virtual bool Finish() = 0;

        /** @brief Draws the level background tile image for the current frame. */
        virtual void DrawBackground() = 0;

        /**
         * @brief Draws one character glyph at the specified game-space position.
         * @param[in] rank Glyph index in the font sprite sheet (PixmapChannel::Text).
         * @param[in] pos  Top-left destination position in game-space coordinates.
         * @param[in] size Scale factor relative to the base 32x32 glyph size (1.0 = original size).
         */
        virtual void DrawChar(intcs rank, TinyPoint pos, double size) = 0;

        /**
         * @brief Draws a HUD icon at the given HUD-space position without hotspot transform.
         * @param[in] channel Sprite sheet (texture atlas) to sample from.
         * @param[in] rank    Icon index within the sprite sheet grid.
         * @param[in] pos     Destination position in HUD-space.
         */
        virtual void HudIcon(PixmapChannel channel, intcs rank, TinyPoint pos) = 0;

        /**
         * @brief Draws a sprite icon at a HUD-space position at full opacity with no rotation.
         * @param[in] channel Sprite sheet to sample from.
         * @param[in] rank    Icon index within the sprite sheet grid.
         * @param[in] pos     Destination position in HUD-space.
         */
        virtual void QuickIcon(PixmapChannel channel, intcs rank, TinyPoint pos) = 0;

        /**
         * @brief Draws a sprite icon with explicit opacity and rotation.
         * @param[in] channel  Sprite sheet to sample from.
         * @param[in] rank     Icon index within the sprite sheet grid.
         * @param[in] pos      Destination position in HUD-space.
         * @param[in] opacity  Opacity in the range [0.0, 1.0].
         * @param[in] rotation Rotation angle in radians (clockwise).
         */
        virtual void QuickIcon(PixmapChannel channel, intcs rank, TinyPoint pos, double opacity, double rotation) = 0;

        /**
         * @brief Draws a rectangular sub-region of a sprite sheet at a game-space destination.
         * @param[in] channel Sprite sheet to sample from.
         * @param[in] dest    Destination top-left position in game-space.
         * @param[in] rect    Source rectangle in sprite-sheet pixel coordinates.
         * @return True on success; false if the channel or rectangle is invalid.
         */
        virtual bool DrawPart(PixmapChannel channel, TinyPoint dest, TinyRect rect) = 0;

        /**
         * @brief Draws a rectangular sub-region of a sprite sheet scaled by an additional zoom factor.
         * @param[in] channel Sprite sheet to sample from.
         * @param[in] dest    Destination top-left position in game-space.
         * @param[in] rect    Source rectangle in sprite-sheet pixel coordinates.
         * @param[in] zoom    Additional scale factor applied to the destination size (1.0 = no extra scale).
         * @return True on success; false if the channel or rectangle is invalid.
         */
        virtual bool DrawPart(PixmapChannel channel, TinyPoint dest, TinyRect rect, double zoom) = 0;

        /**
         * @brief Draws a full icon slot from a sprite sheet into a destination rectangle.
         *
         * @details The icon index determines which cell of the sprite-sheet grid is sampled.
         * Do not treat @p icon as a sprite-sheet pixel coordinate.
         *
         * @param[in] channel    Sprite sheet to sample from.
         * @param[in] icon       Icon slot index (data-table identifier, not a pixel offset).
         * @param[in] rect       Destination rectangle in game-space.
         * @param[in] opacity    Opacity in [0.0, 1.0].
         * @param[in] useHotSpot If true, applies the current hotspot transform to the destination.
         */
        virtual void DrawIcon(PixmapChannel channel, intcs icon, TinyRect rect, double opacity, bool useHotSpot) = 0;

        /**
         * @brief Draws a full icon slot with explicit rotation and opacity.
         * @param[in] channel     Sprite sheet to sample from.
         * @param[in] icon        Icon slot index (data-table identifier, not a pixel offset).
         * @param[in] rect        Destination rectangle in game-space.
         * @param[in] opacity     Opacity in [0.0, 1.0].
         * @param[in] rotationDeg Rotation in degrees (clockwise).
         * @param[in] useHotSpot  If true, applies the current hotspot transform to the destination.
         */
        virtual void DrawIcon(PixmapChannel channel, intcs icon, TinyRect rect, double opacity, double rotationDeg,
                              bool useHotSpot) = 0;

        /**
         * @brief Opens a shared SpriteBatch Begin/End pair for the current frame.
         *
         * @details When CNA_SPRITE_BATCHING_ENABLED is defined, all subsequent DrawPart,
         * DrawIcon, and DrawBackground calls skip their own Begin/End and accumulate
         * into one batch that is flushed by EndBatch(). Without the define this is a
         * no-op and each draw call manages its own Begin/End (original behaviour).
         *
         * @pre Must not be called while a batch is already open.
         * @post All subsequent draw calls until EndBatch() are batched together.
         */
        virtual void BeginBatch() = 0;

        /**
         * @brief Closes the shared SpriteBatch Begin/End pair opened by BeginBatch().
         *
         * @details Flushes all batched draw calls to the graphics device.
         * No-op when CNA_SPRITE_BATCHING_ENABLED is not defined.
         *
         * @pre BeginBatch() must have been called.
         * @post The batch is flushed; individual draw calls again manage their own Begin/End.
         */
        virtual void EndBatch() = 0;
    };
}
