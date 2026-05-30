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
     * @brief Interface for the game's sprite/rendering subsystem.
     *
     * IPixmap abstracts all drawing operations needed by the gameplay and UI layers.
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
         * @param pos Point in hotspot-scaled space.
         * @return Corresponding HUD-space point.
         */
        virtual TinyPoint HotSpotToHud(TinyPoint pos) = 0;

        /**
         * @brief Configures the hotspot zoom and center used by HotSpotToHud().
         * @param zoom Hotspot zoom factor.
         * @param x X coordinate of the hotspot center.
         * @param y Y coordinate of the hotspot center.
         */
        virtual void SetHotSpot(double zoom, double x, double y) = 0;

        virtual void DrawInputButton(TinyRect rect, Def::ButtonGlyph glyph, bool pressed, bool selected) = 0;
        /**
         * @brief Loads all texture resources and creates the sprite batch.
         *
         * Must be called before any drawing method is used.
         *
         * @note Status: PARTIAL
         */
        virtual void LoadContent() = 0;

        /**
         * @brief Loads (or switches to) a background texture by name.
         *
         * The background is used by DrawBackground() to fill the game area.
         * Called when a level is loaded or the background changes between worlds.
         *
         * @param name Asset name of the background texture (without extension).
         */
        virtual void BackgroundCache(const std::string& name) = 0;
        /**
         * @brief Begins a frame by clearing the graphics device.
         *
         * @return Always true.
         *
         * @note Status: PARTIAL
         */
        virtual bool Start() = 0;
        /**
         * @brief Ends a frame.
         *
         * Currently a placeholder because SpriteBatch drawing is begun and ended
         * inside each draw call.
         *
         * @return Always true.
         *
         * @note Status: STUB
         */
        virtual bool Finish() = 0;

        /** @brief Draws the level background for the current frame. */
        virtual void DrawBackground() = 0;

        /**
         * @brief Draws one character glyph at the specified game-space position.
         * @param rank Glyph index in the font sprite sheet (channel Text).
         * @param pos Top-left position in game-space coordinates.
         * @param size Scale factor relative to the base 32x32 glyph size.
         */
        virtual void DrawChar(intcs rank, TinyPoint pos, double size) = 0;

        /**
         * @brief Draws a HUD icon at the given HUD-space position without hotspot transform.
         * @param channel Sprite sheet to use.
         * @param rank Icon index within the sprite sheet.
         * @param pos Destination position in HUD-space.
         */
        virtual void HudIcon(PixmapChannel channel, intcs rank, TinyPoint pos) = 0;

        /**
         * @brief Draws a sprite icon quickly at a HUD-space position (full opacity, no rotation).
         * @param channel Sprite sheet to use.
         * @param rank Icon index.
         * @param pos Destination position in HUD-space.
         */
        virtual void QuickIcon(PixmapChannel channel, intcs rank, TinyPoint pos) = 0;

        /**
         * @brief Draws a sprite icon with explicit opacity and rotation.
         * @param channel Sprite sheet to use.
         * @param rank Icon index.
         * @param pos Destination position in HUD-space.
         * @param opacity Opacity in the range [0,1].
         * @param rotation Rotation in radians.
         */
        virtual void QuickIcon(PixmapChannel channel, intcs rank, TinyPoint pos, double opacity, double rotation) = 0;

        /**
         * @brief Draws a rectangular sub-region of a sprite sheet at a game-space destination.
         * @param channel Sprite sheet to use.
         * @param dest Destination top-left in game-space.
         * @param rect Source rectangle in the sprite sheet (sprite sheet pixel coordinates).
         * @return True on success.
         */
        virtual bool DrawPart(PixmapChannel channel, TinyPoint dest, TinyRect rect) = 0;

        /**
         * @brief Draws a rectangular sub-region of a sprite sheet with an additional zoom.
         * @param channel Sprite sheet to use.
         * @param dest Destination top-left in game-space.
         * @param rect Source rectangle in the sprite sheet.
         * @param zoom Additional scale factor.
         * @return True on success.
         */
        virtual bool DrawPart(PixmapChannel channel, TinyPoint dest, TinyRect rect, double zoom) = 0;

        /**
         * @brief Draws a full icon slot from a sprite sheet into a destination rectangle.
         *
         * The icon index determines which cell of the sprite sheet grid is sampled.
         * Do not treat @p icon as a sprite-sheet pixel coordinate.
         *
         * @param channel Sprite sheet to use.
         * @param icon Icon slot index (data-table identifier, not a pixel offset).
         * @param rect Destination rectangle in game-space.
         * @param opacity Opacity in [0,1].
         * @param useHotSpot If true, applies the current hotspot transform to the destination.
         */
        virtual void DrawIcon(PixmapChannel channel, intcs icon, TinyRect rect, double opacity, bool useHotSpot) = 0;

        /**
         * @brief Draws a full icon slot with rotation and opacity.
         * @param channel Sprite sheet to use.
         * @param icon Icon slot index.
         * @param rect Destination rectangle in game-space.
         * @param opacity Opacity in [0,1].
         * @param rotationDeg Rotation in degrees.
         * @param useHotSpot If true, applies the current hotspot transform.
         */
        virtual void DrawIcon(PixmapChannel channel, intcs icon, TinyRect rect, double opacity, double rotationDeg,
                              bool useHotSpot) = 0;

        /**
         * @brief Opens a shared SpriteBatch Begin/End pair for the current frame.
         *
         * When CNA_SPRITE_BATCHING_ENABLED is defined, all subsequent DrawPart /
         * DrawIcon / DrawBackground calls skip their own Begin/End and accumulate
         * into one batch that is flushed by EndBatch().  Without the define this
         * is a no-op and each draw call manages its own Begin/End (original behaviour).
         */
        virtual void BeginBatch() = 0;

        /**
         * @brief Closes the shared SpriteBatch Begin/End pair opened by BeginBatch().
         *
         * No-op when CNA_SPRITE_BATCHING_ENABLED is not defined.
         */
        virtual void EndBatch() = 0;
    };
}
