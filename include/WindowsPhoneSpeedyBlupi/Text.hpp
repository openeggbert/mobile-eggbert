/**
 * @file Text.hpp
 * @brief Declarations for the Text static utility class.
 * @details Provides bitmap text rendering backed by the game's font sprite sheet
 *          (PixmapChannel::Text).  Supports left-aligned, centred, and slanted
 *          (pente) text, as well as pixel-width measurement.
 *
 * ### Font sprite-sheet layout
 * The sprite sheet contains one glyph per cell arranged in a single row.
 * Each cell is a fixed square whose on-screen size scales with the @c size
 * parameter passed to the draw methods.  Glyph index 1 is a generic fallback
 * placeholder used for unmapped character codes.  Indices 32–127 correspond
 * to printable ASCII characters; the 15 entries in @c table_accents map
 * commonly used accented Latin characters (ü, à, é, etc.) to additional
 * glyph slots starting at index 15 in the table.
 *
 * ### Slant transform (pente)
 * DrawTextPente() accumulates the total rendered pixel width of all preceding
 * characters and divides it by @c pente to compute a Y-axis offset for each
 * successive glyph.  This produces a diagonal, italic-like appearance without
 * rotating individual glyphs.  Larger @c pente values give a shallower slant.
 */

#pragma once

#include "IPixmap.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using std::string;

    /**
     * @class Text
     * @brief Static utility class for drawing bitmap text using the game's font sprite sheet.
     *
     * @details All in-game text rendering passes through this class.  Characters are
     *          resolved to glyph indices via @c table_char and drawn by calling
     *          IPixmap::DrawChar() on the Text sprite channel.  No system fonts are
     *          used; only the embedded bitmap font is supported.
     *
     * @note All coordinates are in game-space (640 × 480 logical pixels).
     * @note This class is rendering-only and does not mutate any gameplay state.
     */
    class Text
    {
    public:
        /** @brief Deleted default constructor — this is a static utility class. */
        Text() = delete;
        /** @brief Deleted destructor — this is a static utility class. */
        ~Text() = delete;

    private:
        /**
         * @brief Character-to-glyph mapping table.
         *
         * @details Flat array of 1536 short integers organised as 256 records of
         *          6 values each.  For character code @c c the record begins at
         *          index @c GetOffset(c)*6 and encodes up to two glyph draws:
         *          - [0] Primary glyph index (drawn first).
         *          - [1] X pixel offset for the primary glyph relative to the pen.
         *          - [2] Y pixel offset for the primary glyph relative to the pen.
         *          - [3] Secondary glyph index, or @c -1 if only one draw is needed
         *                (used for precomposed accented characters).
         *          - [4] X pixel offset for the secondary glyph.
         *          - [5] Y pixel offset for the secondary glyph.
         */
        static const SharpRuntime::shortcs table_char[1536];

        /**
         * @brief Accent/combining-character code table.
         *
         * @details Stores the byte values of the 15 supported extended-Latin
         *          character codes (e.g. ü=252, à=224, é=233).  GetOffset()
         *          searches this table for values above U+0080 and maps matches
         *          to consecutive glyph offsets starting at index 15.
         */
        static const SharpRuntime::ubytecs table_accents[15];

        /**
         * @brief Per-glyph advance-width table for proportional spacing.
         *
         * @details Contains the pixel width (at scale 1.0) for each of the 128
         *          glyph slots.  GetCharWidth() indexes this table via the primary
         *          glyph index from @c table_char and multiplies by @c size.
         *          A value of 0 indicates a non-printing glyph (e.g. NUL, DEL).
         */
        static const SharpRuntime::ubytecs table_width[128];

    public:
        /**
         * @brief Draws text left-aligned starting at @p pos.
         *
         * @details Guards against null/empty strings, then delegates to DrawText().
         *
         * @param[in,out] pixmap Rendering subsystem to draw into.
         * @param[in]     pos    Top-left starting position in game-space.
         * @param[in]     text   String to render.
         * @param[in]     size   Scale factor relative to the base glyph size (1.0 = native).
         */
        static void DrawTextLeft(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        /**
         * @brief Draws text left-aligned starting at @p pos.
         *
         * @details Iterates over each character and calls DrawChar(), which advances
         *          the pen position by the proportional character width after each glyph.
         *          This is the primary rendering path used by DrawTextLeft() and
         *          DrawTextCenter().
         *
         * @param[in,out] pixmap Rendering subsystem.
         * @param[in]     pos    Top-left starting position in game-space.
         * @param[in]     text   String to render.
         * @param[in]     size   Scale factor.
         */
        static void DrawText(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        /**
         * @brief Draws text with a progressive Y-axis skew (diagonal/italic-like slant).
         *
         * @details For each character the Y position is offset by
         *          @c totalAccumulatedWidth / @p pente, where the accumulated width
         *          is the total rendered pixel width of all preceding characters.
         *          Larger @p pente values produce a shallower slant; smaller values
         *          produce a steeper diagonal.
         *
         * @param[in,out] pixmap Rendering subsystem.
         * @param[in]     pos    Top-left starting position of the first character.
         * @param[in]     text   String to render.
         * @param[in]     pente  Slant divisor (pixels of horizontal advance per pixel
         *                       of vertical displacement); must be non-zero.
         * @param[in]     size   Scale factor.
         *
         * @warning Passing @p pente = 0 causes integer division by zero.
         */
        static void DrawTextPente(IPixmap& pixmap, TinyPoint pos, const string& text, intcs pente, double size);

        /**
         * @brief Draws text horizontally centred around @p pos.
         *
         * @details Measures total pixel width via GetTextWidth(), offsets the
         *          X start position left by half that width, then calls DrawText().
         *
         * @param[in,out] pixmap Rendering subsystem.
         * @param[in]     pos    Centre position in game-space (text is centred on this X).
         * @param[in]     text   String to render.
         * @param[in]     size   Scale factor.
         */
        static void DrawTextCenter(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        /**
         * @brief Returns the total pixel width of @p text at the given scale.
         *
         * @details Sums GetCharWidth() for every character.  Returns 0 for null or
         *          empty strings.
         *
         * @param[in] text String to measure.
         * @param[in] size Scale factor.
         * @return Total advance width in game-space pixels, or 0 if @p text is empty.
         */
        static int GetTextWidth(const string& text, double size);

    private:
        /**
         * @brief Resolves a character to its row index into @c table_char.
         *
         * @details Searches @c table_accents for the character code first; if found,
         *          returns 15 + accent_index.  Characters above U+0080 not in the
         *          accent table return the fallback index 1.  ASCII characters (0–127)
         *          return their code value directly.
         *
         * @param[in] c Character to resolve.
         * @return Row index (each row spans 6 elements in @c table_char).
         */
        static intcs GetOffset(SharpRuntime::charcs c);

        /**
         * @brief Draws a single character and advances the pen position.
         *
         * @details Resolves @p car via GetOffset(), reads up to two glyph draws
         *          from @c table_char (skipping the second if its index is -1),
         *          calls DrawCharSingle() for each, then advances @p pos.X by
         *          GetCharWidth().
         *
         * @param[in,out] pixmap Rendering subsystem.
         * @param[in,out] pos    Current pen position; X is advanced after drawing.
         * @param[in]     car    Character to draw.
         * @param[in]     size   Scale factor.
         */
        static void DrawChar(IPixmap& pixmap, TinyPoint& pos, const SharpRuntime::charcs car, const double size);

        /**
         * @brief Returns the advance width of a single character in pixels.
         *
         * @details Looks up the primary glyph index from @c table_char then queries
         *          @c table_width for the base width, multiplying by @p size.
         *
         * @param[in] c    Character to measure.
         * @param[in] size Scale factor.
         * @return Advance width in game-space pixels.
         */
        static intcs GetCharWidth(SharpRuntime::charcs c, double size);

        /**
         * @brief Forwards a single glyph draw call to the pixmap subsystem.
         *
         * @details Thin wrapper around IPixmap::DrawChar() that isolates DrawChar()
         *          from direct IPixmap interface details.
         *
         * @param[in,out] pixmap Rendering subsystem.
         * @param[in]     pos    Position at which to draw the glyph.
         * @param[in]     rank   Glyph index in the font sprite sheet.
         * @param[in]     size   Scale factor.
         */
        static void DrawCharSingle(IPixmap& pixmap, TinyPoint pos, intcs rank, double size);
    };
}
