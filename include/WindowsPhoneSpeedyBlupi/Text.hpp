#pragma once

#include "IPixmap.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    using std::string;

    /**
     * @brief Static utility class for drawing bitmap text using the game's font sprite sheet.
     *
     * All text rendering in the game goes through this class. Characters are drawn
     * by looking up glyph indices in table_char and rendering them from the Text
     * sprite sheet (PixmapChannel::Text) via IPixmap::DrawChar().
     *
     * Supports left-aligned, centered, and slanted (pente) text, as well as
     * text width measurement. Does not render to any system font — only the
     * bitmap font embedded in the game's sprite sheet is supported.
     *
     * @note All positions are in game-space (640x480 logical) coordinates.
     * @note This is rendering code. It does not modify any gameplay state.
     */
    class Text
    {
    public:
        Text() = delete;
        ~Text() = delete;

    private:
        /** Character-to-glyph mapping table. Each entry is a glyph index in the font sprite sheet. */
        static const SharpRuntime::shortcs table_char[1536];

        /** Accent/combining-character offsets for multi-byte character rendering. */
        static const SharpRuntime::ubytecs table_accents[15];

        /** Per-character pixel widths for proportional spacing (at base size). */
        static const SharpRuntime::ubytecs table_width[128];

    public:
        /**
         * @brief Draws text left-aligned starting at @p pos.
         * @param pixmap Rendering subsystem to draw into.
         * @param pos Top-left starting position in game-space.
         * @param text String to render.
         * @param size Scale factor relative to the base glyph size.
         */
        static void DrawTextLeft(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        /**
         * @brief Draws text left-aligned starting at @p pos (alias for DrawTextLeft).
         * @param pixmap Rendering subsystem.
         * @param pos Top-left starting position in game-space.
         * @param text String to render.
         * @param size Scale factor.
         */
        static void DrawText(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        /**
         * @brief Draws text with a horizontal skew (italic-like slant).
         * @param pixmap Rendering subsystem.
         * @param pos Top-left starting position in game-space.
         * @param text String to render.
         * @param pente Horizontal pixel offset applied per character (slant amount).
         * @param size Scale factor.
         */
        static void DrawTextPente(IPixmap& pixmap, TinyPoint pos, const string& text, intcs pente, double size);

        /**
         * @brief Draws text centered horizontally around @p pos.
         * @param pixmap Rendering subsystem.
         * @param pos Center position in game-space (text is centered on this X).
         * @param text String to render.
         * @param size Scale factor.
         */
        static void DrawTextCenter(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        /**
         * @brief Returns the pixel width of @p text at the given scale.
         * @param text String to measure.
         * @param size Scale factor.
         * @return Width in game-space pixels.
         */
        static int GetTextWidth(const string& text, double size);

    private:
        static intcs GetOffset(SharpRuntime::charcs c);

        static void DrawChar(IPixmap& pixmap, TinyPoint& pos, const SharpRuntime::charcs car, const double size);

        static intcs GetCharWidth(SharpRuntime::charcs c, double size);

        static void DrawCharSingle(IPixmap& pixmap, TinyPoint pos, intcs rank, double size);
    };
}
