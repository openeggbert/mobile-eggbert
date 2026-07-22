/**
 * @file Pixmap.hpp
 * @brief Declares the Pixmap class, the concrete SDL3/XNA SpriteBatch rendering
 *        implementation of IPixmap.
 *
 * @details
 * Pixmap owns all game texture atlases (sprite sheets), maintains the per-frame
 * viewport geometry (zoom and centering origin), and dispatches every draw
 * call through an XNA-style SpriteBatch.
 *
 * ### Coordinate spaces
 * | Space       | Description                                        |
 * |-------------|----------------------------------------------------|
 * | Game-space  | Logical 640x480, origin top-left.                  |
 * | HUD-space   | Game-space offset by (originX, originY).           |
 * | Screen-space| Physical pixels after the zoom transform.          |
 *
 * ### Sprite-batching mode
 * When `CNA_SPRITE_BATCHING_ENABLED` is defined (currently hard-coded in the
 * .cpp), callers **must** bracket every frame's draw calls with
 * BeginBatch() / EndBatch().  All DrawPart / DrawIcon / DrawBackground
 * invocations then share the same SpriteBatch::Begin-End pair, reducing GPU
 * draw-call overhead by roughly 50-100x compared with the original one-call-
 * per-sprite design.  Without the define the behaviour is unchanged.
 *
 * @see IPixmap
 */

#pragma once

#include "Def.hpp"
#include "IGame1.hpp"
#include "IPixmap.hpp"

#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyRect.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /**
     * @class Pixmap
     * @brief Concrete SDL3/XNA SpriteBatch implementation of IPixmap.
     *
     * @details
     * Pixmap is the rendering back-end for Speedy Blupi.  It loads all texture
     * atlases, computes the viewport zoom/origin each frame, and issues every
     * sprite draw through a shared or per-call SpriteBatch (depending on whether
     * `CNA_SPRITE_BATCHING_ENABLED` is defined).
     *
     * ### Lifecycle
     * 1. Construct with a valid IGame1 pointer and a GraphicsDeviceManager.
     * 2. Call LoadContent() once to load all textures and compute initial geometry.
     * 3. Each frame: Start() -> BeginBatch() -> draw calls -> EndBatch() -> Finish().
     *
     * @warning This class is non-copyable.  Do not attempt to copy or move it.
     *
     * @note The base game resolution is fixed at 640x480 logical pixels.
     *       All draw coordinates must be expressed in that space unless they are
     *       explicitly converted to screen-space first.
     *
     * @see IPixmap
     */
    class Pixmap : public IPixmap
    {
        IGame1* game1; ///< @brief Non-owning pointer to the IGame1 host object.

        Microsoft::Xna::Framework::GraphicsDeviceManager& graphics; ///< @brief Reference to the graphics device manager used to query viewport dimensions.

        /**
         * @brief Uniform zoom factor that maps the 640x480 logical canvas to the
         *        physical viewport while preserving aspect ratio.
         *
         * @details Computed by UpdateGeometry() as
         *          `min(screenWidth/640, screenHeight/480)`.
         *          A value of 0.0 means UpdateGeometry() has not yet run.
         */
        double zoom = 0.0;

        /**
         * @brief Horizontal pixel offset from the left edge of the physical
         *        viewport to the left edge of the centered 640-wide game canvas.
         *
         * @details Equals `(screenWidth - 640 * zoom) / 2`.  Used by HUD and
         *          game-space drawing methods to center the game area.
         */
        double originX = 0.0;

        /**
         * @brief Vertical pixel offset from the top edge of the physical viewport
         *        to the top edge of the centered 480-high game canvas.
         *
         * @details Equals `(screenHeight - 480 * zoom) / 2`.
         */
        double originY = 0.0;

        /**
         * @brief Zoom factor applied by the hotspot (camera zoom) feature.
         *
         * @details Used in GetDstRectangle() to scale sprite destination
         *          rectangles around the hotspot center.  A value of 1.0 means
         *          no extra zoom.  Values greater than 1.0 zoom in; values
         *          between 0.0 and 1.0 zoom out (guarded by the MODERN define).
         */
        double hotSpotZoom = 1.0;

        /**
         * @brief X coordinate of the hotspot center in game-space pixels.
         *
         * @details The hotspot is the pivot point around which the camera-zoom
         *          transform is applied.  Set via SetHotSpot().
         */
        double hotSpotX = 0.0;

        /**
         * @brief Y coordinate of the hotspot center in game-space pixels.
         *
         * @details See hotSpotX for details.
         */
        double hotSpotY = 0.0;

        std::unique_ptr<Microsoft::Xna::Framework::Graphics::SpriteBatch> spriteBatch; ///< @brief The XNA/CNA SpriteBatch used for all draw calls.  Created in LoadContent().

        Texture2D bitmapText;          ///< @brief Texture atlas for font/text glyphs (PixmapChannel::Text).
        Texture2D bitmapButton;        ///< @brief Texture atlas for HUD button icons (PixmapChannel::Button).
        Texture2D bitmapJauge;         ///< @brief Texture atlas for the health/energy gauge (PixmapChannel::Jauge).
        Texture2D bitmapBlupi;         ///< @brief Primary Blupi character sprite sheet (PixmapChannel::Blupi).
        Texture2D bitmapBlupi1;        ///< @brief Secondary Blupi sprite sheet shared by Blupi1_11/12/13 channels.
        Texture2D bitmapObject;        ///< @brief Collectible and interactive object sprites (PixmapChannel::Object).
        Texture2D bitmapElement;       ///< @brief Level-tile/element sprites (PixmapChannel::Element).
        Texture2D bitmapExplo;         ///< @brief Explosion animation frames (PixmapChannel::Explosion).
        Texture2D bitmapPad;           ///< @brief Touch-pad / on-screen input button icons (PixmapChannel::Pad).
        Texture2D bitmapSpeedyBlupi;   ///< @brief "Speedy Blupi" title-screen background (PixmapChannel::SpeedyBlupiBackground).
        Texture2D bitmapBlupiYoupie;   ///< @brief "Blupi Youpie" victory background (PixmapChannel::BlupiYoupieBackground).
        Texture2D bitmapGear;          ///< @brief Gear / settings background graphic (PixmapChannel::GearBackground).
        Texture2D bitmapBackground;    ///< @brief Current level scrolling background, swapped by BackgroundCache() (PixmapChannel::Background).

        Microsoft::Xna::Framework::Vector2 origin; ///< @brief SpriteBatch rotation origin passed to every Draw call; always (0,0).
        Microsoft::Xna::Framework::Graphics::SpriteEffects effect; ///< @brief SpriteEffects flags passed to every Draw call; always None.

    public:
        /**
         * @brief Returns the bounding rectangle of the drawable game area in
         *        virtual 640x480 coordinates.
         *
         * @details For non-Android platforms the result is always
         *          `{0, 0, drawWidth, drawHeight}` where drawWidth/drawHeight
         *          are the logical dimensions of the viewport that maintain the
         *          4:3 aspect ratio.  On Android with a tall viewport the
         *          effective screen width is normalized first.
         *
         * @return TinyRect describing the drawable bounds (Left=0, Top=0,
         *         Right=logical width, Bottom=logical height).
         */
        [[nodiscard]] TinyRect getDrawBoundsProperty() override;

        /**
         * @brief Returns the top-left origin of the centered game canvas in
         *        physical screen pixels.
         *
         * @return TinyPoint with the (originX, originY) offset as integer pixels.
         *
         * @note Do not cache this value across frames; it changes when the window
         *       is resized and UpdateGeometry() is re-run.
         */
        [[nodiscard]] TinyPoint getOriginProperty() const override;

        /**
         * @brief Constructs a Pixmap and initialises all texture handles to empty.
         *
         * @param[in] game1    Non-owning pointer to the IGame1 host; must remain
         *                     valid for the lifetime of this Pixmap.
         * @param[in] graphics Reference to the GraphicsDeviceManager; must remain
         *                     valid for the lifetime of this Pixmap.
         *
         * @pre  game1 must not be nullptr.
         * @note Call LoadContent() before issuing any draw calls.
         */
        Pixmap(IGame1* game1, Microsoft::Xna::Framework::GraphicsDeviceManager& graphics);

        Pixmap(const Pixmap&) = delete;            ///< @brief Copying is disabled; Pixmap owns GPU resources.
        Pixmap& operator=(const Pixmap&) = delete; ///< @brief Copy assignment is disabled.

        ~Pixmap() override = default;

        /**
         * @brief Converts a point from hotspot-scaled coordinates to HUD coordinates.
         *
         * @details
         * Applies the inverse of the hotspot zoom around the hotspot center, then
         * subtracts the pixmap origin (originX, originY) so the result is
         * expressed in HUD-space (suitable for positioning UI elements).
         *
         * The transform is:
         * @code
         *   hudX = (pos.X - hotSpotX) / hotSpotZoom + hotSpotX - originX
         *   hudY = (pos.Y - hotSpotY) / hotSpotZoom + hotSpotY - originY
         * @endcode
         *
         * If hotSpotZoom is 0.0 the input point is returned unchanged to avoid
         * a division by zero.
         *
         * @param[in] pos  Point in hotspot-scaled game-space coordinates.
         * @return         Corresponding point in HUD-space coordinates.
         *
         * @note SetHotSpot() must have been called at least once before calling
         *       this method if a non-trivial transform is required.
         */
        TinyPoint HotSpotToHud(TinyPoint pos) override;

        /**
         * @brief Sets the hotspot transform parameters used by HotSpotToHud() and
         *        GetDstRectangle().
         *
         * @details
         * Stores the camera-zoom factor and pivot point that are applied whenever
         * a draw call passes `useHotSpot = true`.  Call this once per frame when
         * the camera zoom changes.
         *
         * @param[in] zoom  Hotspot zoom factor.  Values > 1.0 zoom in; values
         *                  in (0,1) zoom out (requires MODERN define).
         * @param[in] x     X coordinate of the hotspot pivot in game-space pixels.
         * @param[in] y     Y coordinate of the hotspot pivot in game-space pixels.
         */
        void SetHotSpot(double zoom, double x, double y) override;

        /**
         * @brief Draws an on-screen input button for the given button glyph.
         *
         * @details
         * Maps each ButtonGlyph value to a specific pad icon index and delegates
         * to DrawIcon(PixmapChannel::Pad, ...).  The opacity is reduced to 0.8 when
         * @p pressed is true (0.6 for gameplay action buttons).  Toggle-style
         * setup buttons use @p selected to switch between the active/inactive icon.
         *
         * Cheat digit buttons (Cheat1..Cheat9) draw a generic background icon and
         * then render the cheat label via Text::DrawTextCenter().
         *
         * Multi-part cheat glyphs (Cheat11, Cheat12, Cheat21, Cheat22, Cheat31,
         * Cheat32) draw nothing and return immediately.
         *
         * @param[in] rect      Destination rectangle in screen-space pixels.
         * @param[in] glyph     Button glyph identifying which icon or label to draw.
         * @param[in] pressed   True if the button is currently held down.
         * @param[in] selected  True if the button is in its active/selected state
         *                      (used only for toggle-style setup buttons and gamer
         *                      selection buttons).
         *
         * @pre LoadContent() must have been called successfully.
         */
        void DrawInputButton(TinyRect rect, Def::ButtonGlyph glyph, bool pressed, bool selected) override;

        /**
         * @brief Loads all texture atlases from content and initialises the
         *        SpriteBatch.
         *
         * @details
         * Selects the correct asset sub-directory based on Config::RESOLUTION_SCALE
         * (1x -> "icons/", 2x -> "icons2x/", 4x -> "icons4x/") and loads every
         * sprite sheet into the corresponding bitmap member.  Then calls
         * UpdateGeometry() to compute the initial zoom/origin.
         *
         * @pre  The GraphicsDevice and content pipeline must be fully initialised.
         * @post spriteBatch is non-null; all bitmap members hold valid textures;
         *       zoom, originX, and originY are set for the current viewport.
         *
         * @note Must be called exactly once before any draw method is used.
         */
        void LoadContent() override;

    private:
        /**
         * @brief Recomputes zoom, originX, and originY for the current viewport.
         *
         * @details
         * Queries the current viewport dimensions from the GraphicsDeviceManager,
         * then computes:
         * @code
         *   zoom    = min(screenWidth/640, screenHeight/480)
         *   originX = (screenWidth  - 640 * zoom) / 2
         *   originY = (screenHeight - 480 * zoom) / 2
         * @endcode
         * On Android, when the viewport height exceeds 480 px, the effective
         * screenWidth is replaced with `screenHeight * (640/480)` before the
         * calculation to normalise tall screens to a 4:3 virtual canvas.
         *
         * @post zoom > 0; originX >= 0; originY >= 0.
         * @note Called automatically by LoadContent().  Re-call whenever the
         *       window is resized.
         */
        void UpdateGeometry();

    public:
        /**
         * @brief Loads (or switches) the level background texture.
         *
         * @details
         * Selects the correct asset sub-directory based on Config::RESOLUTION_SCALE
         * and loads the texture named @p name into bitmapBackground.  Subsequent
         * DrawBackground() calls will use this texture.
         *
         * @param[in] name  Asset name (without extension or directory prefix).
         *                  Example: "level1".
         *
         * @pre  LoadContent() must have been called so the content pipeline is
         *       available.
         * @post bitmapBackground holds the newly loaded texture.
         */
        void BackgroundCache(const std::string& name) override;

        /**
         * @brief Begins a frame by clearing the graphics device to CornflowerBlue.
         *
         * @return Always returns true.
         *
         * @note Call this once per frame, before any draw calls.
         */
        bool Start() override;

        /**
         * @brief Ends a frame.
         *
         * @details Currently a no-op placeholder.  SpriteBatch flushing is handled
         *          by EndBatch() (batching mode) or inline within each draw call
         *          (non-batching mode).
         *
         * @return Always returns true.
         */
        bool Finish() override;

        /**
         * @brief Draws the full-viewport background for the current frame.
         *
         * @details
         * Performs two draws:
         * 1. Stretches bitmapBackground to fill the entire physical screen.
         * 2. Draws the 640x480 game-area portion of bitmapBackground at the
         *    current (originX, originY) so the game canvas is correctly centered.
         *
         * On Android with tall viewports, the effective screenWidth is normalized
         * to `screenHeight * (640/480)` before the first draw.
         *
         * Respects the batching invariant: if a batch is already open
         * (batch_started_ == true) the method reuses it; otherwise it opens and
         * closes its own Begin/End pair.
         *
         * @pre  LoadContent() and BackgroundCache() must have been called.
         * @warning Calling this method before LoadContent() logs an error and
         *          returns without drawing.
         */
        void DrawBackground() override;

        /**
         * @brief Draws one character glyph at the given game-space position.
         *
         * @details
         * Translates @p pos from game-space to screen-space by adding (originX,
         * originY), constructs a `32*size x 32*size` destination rectangle, then
         * delegates to DrawIcon(PixmapChannel::Text, rank, ...).
         *
         * @param[in] rank  Zero-based index of the glyph in the Text sprite sheet.
         * @param[in] pos   Top-left position of the glyph in game-space coordinates.
         * @param[in] size  Uniform scale factor; the base glyph cell is 32x32 px.
         *
         * @pre  LoadContent() must have been called.
         */
        void DrawChar(intcs rank, TinyPoint pos, double size) override;

        /**
         * @brief Draws a HUD icon at a HUD-space position (no hotspot transform).
         *
         * @details
         * Translates @p pos to screen-space by adding (originX, originY), then
         * calls DrawIcon() with full opacity and useHotSpot = false.
         *
         * @param[in] channel  Sprite sheet to sample.
         * @param[in] rank     Icon slot index within the sprite sheet.
         * @param[in] pos      Destination position in HUD-space.
         *
         * @pre  LoadContent() must have been called.
         */
        void HudIcon(PixmapChannel channel, intcs rank, TinyPoint pos) override;

        /**
         * @brief Draws a sprite icon at a game-space position with full opacity and
         *        no rotation, applying the hotspot transform.
         *
         * @param[in] channel  Sprite sheet to sample.
         * @param[in] rank     Icon slot index.
         * @param[in] pos      Destination position in game-space (hotspot transform applied).
         *
         * @pre  LoadContent() must have been called.
         */
        void QuickIcon(PixmapChannel channel, intcs rank, TinyPoint pos) override;

        /**
         * @brief Draws a sprite icon at a game-space position with explicit opacity
         *        and rotation, applying the hotspot transform.
         *
         * @param[in] channel   Sprite sheet to sample.
         * @param[in] rank      Icon slot index.
         * @param[in] pos       Destination position in game-space.
         * @param[in] opacity   Opacity in [0,1].
         * @param[in] rotation  Rotation in radians.
         *
         * @pre  LoadContent() must have been called.
         */
        void QuickIcon(PixmapChannel channel, intcs rank, TinyPoint pos, double opacity, double rotation) override;

        /**
         * @brief Draws a rectangular sub-region of a sprite sheet at unity zoom.
         *
         * @details Delegates to DrawPart(channel, dest, rect, 1.0).
         *
         * @param[in] channel  Sprite sheet to sample.
         * @param[in] dest     Destination top-left in game-space.
         * @param[in] rect     Source rectangle in logical (1x) sprite-sheet coordinates.
         * @return True on success; false if the channel texture is unavailable or
         *         the SpriteBatch is not initialised.
         */
        bool DrawPart(PixmapChannel channel, TinyPoint dest, TinyRect rect) override;

        /**
         * @brief Draws a rectangular sub-region of a sprite sheet with an additional
         *        scale factor.
         *
         * @details
         * The source rectangle (@p rect) is first multiplied by
         * Config::RESOLUTION_SCALE so it addresses the correct pixels in the
         * loaded (possibly 2x or 4x) texture.  The destination rectangle stays in
         * logical (1x) coordinates so on-screen sizes are independent of the asset
         * resolution.
         *
         * For the Jauge channel, @p dest is first translated by (originX, originY).
         *
         * Respects the batching invariant (see class details).
         *
         * @param[in] channel  Sprite sheet to sample.
         * @param[in] dest     Destination top-left in game-space.
         * @param[in] rect     Source rectangle in logical (1x) sprite-sheet coordinates.
         * @param[in] zoom     Additional scale factor applied to destination width/height.
         * @return True on success; false if the channel texture is unavailable or
         *         the SpriteBatch is not initialised.
         *
         * @pre  LoadContent() must have been called.
         * @warning Calling before LoadContent() logs an error and returns false.
         */
        bool DrawPart(PixmapChannel channel, TinyPoint dest, TinyRect rect, double zoom) override;

        /**
         * @brief Draws a full icon slot from a sprite sheet into a destination
         *        rectangle at the given opacity.
         *
         * @details Delegates to DrawIcon(channel, icon, rect, opacity, 0.0, useHotSpot).
         *
         * @param[in] channel     Sprite sheet to sample.
         * @param[in] icon        Icon slot index (grid cell in the sprite sheet).
         * @param[in] rect        Destination rectangle in game-space (may be zero-sized to
         *                        use the natural icon dimensions).
         * @param[in] opacity     Draw opacity in [0,1].
         * @param[in] useHotSpot  If true, the hotspot zoom/pivot transform is applied to
         *                        the destination rectangle.
         *
         * @pre  LoadContent() must have been called.
         * @warning An @p icon value of -1 is treated as a no-op (nothing drawn).
         */
        void DrawIcon(PixmapChannel channel, intcs icon, TinyRect rect, double opacity, bool useHotSpot) override;

        /**
         * @brief Draws a full icon slot with rotation and opacity.
         *
         * @details
         * Computes source and destination rectangles from the channel's sprite-sheet
         * grid layout, then calls SpriteBatch::Draw() with the rotation converted
         * from degrees to radians.  When @p useHotSpot is true and hotSpotZoom >
         * 1.0 (or < 1.0 with MODERN defined), the destination rectangle corners are
         * scaled around the hotspot pivot before drawing.
         *
         * Icons on the Pad channel are suppressed on non-Android platforms when
         * `TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE` is set and no
         * touch device is connected.
         *
         * Respects the batching invariant (see class details).
         *
         * @param[in] channel      Sprite sheet to sample.
         * @param[in] icon         Icon slot index.  Pass -1 to skip drawing.
         * @param[in] rect         Destination rectangle in game-space.
         * @param[in] opacity      Draw opacity in [0,1].
         * @param[in] rotationDeg  Clockwise rotation in degrees.
         * @param[in] useHotSpot   If true, applies the hotspot transform.
         *
         * @pre  LoadContent() must have been called.
         * @warning Calling before LoadContent() logs an error and returns silently.
         */
        void DrawIcon(PixmapChannel channel, intcs icon, TinyRect rect, double opacity, double rotationDeg,
                      bool useHotSpot) override;

        /**
         * @brief Opens a shared SpriteBatch Begin/End pair for the current frame.
         *
         * @details
         * When `CNA_SPRITE_BATCHING_ENABLED` is defined, this method calls
         * SpriteBatch::Begin(BackToFront, AlphaBlend) and sets the internal flag
         * `batch_started_`.  All subsequent draw calls detect this flag and omit
         * their own Begin/End, accumulating into the single batch opened here.
         *
         * Call EndBatch() once all drawing for the frame is complete to flush the
         * batch to the GPU.
         *
         * When `CNA_SPRITE_BATCHING_ENABLED` is not defined this method is a no-op.
         *
         * @pre  LoadContent() must have been called (spriteBatch must be non-null).
         * @pre  BeginBatch() must not already have been called without a matching
         *       EndBatch() (double-begin is silently ignored).
         * @post batch_started_ == true (when the define is active).
         *
         * @note Reduces GL draw calls by ~50-100x versus the original per-sprite design.
         */
        void BeginBatch() override;

        /**
         * @brief Closes the shared SpriteBatch Begin/End pair opened by BeginBatch().
         *
         * @details
         * Calls SpriteBatch::End() and clears `batch_started_`.  Must be called
         * exactly once per BeginBatch() call, after all frame draw calls are done.
         *
         * When `CNA_SPRITE_BATCHING_ENABLED` is not defined this method is a no-op.
         *
         * @pre  BeginBatch() must have been called for this frame.
         * @post batch_started_ == false.
         */
        void EndBatch() override;

    private:
        bool batch_started_ = false; ///< @brief True while a BeginBatch()/EndBatch() pair is open; draw calls skip their own Begin/End when this flag is set.

        /**
         * @brief Computes the source rectangle for an icon within a sprite-sheet texture.
         *
         * @details
         * Divides the texture into a uniform grid of bitmapGridX x bitmapGridY cells,
         * locates the cell at position @p icon (row-major), and returns a
         * Rectangle of size iconWidth x iconHeight with an optional @p gap offset.
         *
         * @param[in] bitmap       Texture whose pixel dimensions are used for grid division.
         * @param[in] bitmapGridX  Cell width in pixels (already scaled to texture resolution).
         * @param[in] bitmapGridY  Cell height in pixels (already scaled to texture resolution).
         * @param[in] iconWidth    Exact source-pixel width to sample.
         * @param[in] iconHeight   Exact source-pixel height to sample.
         * @param[in] gap          Padding pixels between cells (added to grid stride).
         * @param[in] icon         Zero-based icon slot index (row-major within the grid).
         * @return Source rectangle in texture-pixel coordinates.
         */
        Microsoft::Xna::Framework::Rectangle GetSrcRectangle(const Texture2D& bitmap, intcs bitmapGridX,
                                                             intcs bitmapGridY,
                                                             intcs iconWidth, intcs iconHeight, intcs gap, intcs icon);

        /**
         * @brief Computes the screen-space destination rectangle for a draw call.
         *
         * @details
         * Scales @p rect.Left and @p rect.Top by the current viewport @c zoom,
         * sizes the rectangle using @p iconWidth x @p iconHeight (or the rect's own
         * dimensions when non-zero), then optionally applies the hotspot zoom/pivot
         * transform when @p useHotSpot is true.
         *
         * The hotspot transform (applied when useHotSpot && hotSpotZoom > 1.0):
         * @code
         *   // Translate to hotspot-relative coordinates
         *   scaledX -= hotSpotX;  scaledY -= hotSpotY;
         *   // Scale around origin
         *   scaledX *= hotSpotZoom;  scaledY *= hotSpotZoom;
         *   // Translate back
         *   scaledX += hotSpotX;  scaledY += hotSpotY;
         * @endcode
         *
         * @param[in] rect        Destination hint in game-space; zero Width/Height
         *                        causes the natural icon dimensions to be used.
         * @param[in] iconWidth   Natural icon width in logical (1x) pixels.
         * @param[in] iconHeight  Natural icon height in logical (1x) pixels.
         * @param[in] useHotSpot  Apply hotspot transform when true and hotSpotZoom != 1.
         * @return Rectangle in physical screen-pixel coordinates ready for SpriteBatch.
         */
        Microsoft::Xna::Framework::Rectangle GetDstRectangle(TinyRect rect, intcs iconWidth, intcs iconHeight,
                                                             bool useHotSpot);

        /**
         * @brief Returns a pointer to the Texture2D for the given sprite channel.
         *
         * @param[in] channel  The PixmapChannel identifying which sprite sheet is needed.
         * @return Non-null pointer to the corresponding Texture2D member, or nullptr
         *         for unknown/unmapped channel values.
         *
         * @note The returned pointer is valid for the lifetime of this Pixmap object.
         *       Do not store it beyond a single draw call.
         */
        const Texture2D* GetBitmap(PixmapChannel channel);
    };
}
