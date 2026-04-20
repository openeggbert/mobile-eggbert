#pragma once

#include "IPixmap.hpp"

namespace WindowsPhoneSpeedyBlupi {

using std::string;

    class Text
    {
    public:
        Text() = delete;
        ~Text() = delete;
    private:
        static const CppDotNet::shortcs table_char[1536];

        static const CppDotNet::ubytecs table_accents[15];

        static const CppDotNet::ubytecs table_width[128];

    public:
        static void DrawTextLeft(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        static void DrawText(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        static void DrawTextPente(IPixmap& pixmap, TinyPoint pos, const string& text, intcs pente, double size);

        static void DrawTextCenter(IPixmap& pixmap, TinyPoint pos, const string& text, double size);

        static int GetTextWidth(const string& text, double size);

    private:
        static intcs GetOffset(CppDotNet::charcs c);

        static void DrawChar(IPixmap& pixmap, TinyPoint& pos, const CppDotNet::charcs car, const double size);

        static intcs GetCharWidth(CppDotNet::charcs c, double size);

        static void DrawCharSingle(IPixmap& pixmap, TinyPoint pos, intcs rank, double size);
    };

}
