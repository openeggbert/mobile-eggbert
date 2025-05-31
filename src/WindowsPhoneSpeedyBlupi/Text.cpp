//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Text.h"

namespace WindowsPhoneSpeedyBlupi {

        void Text::DrawTextLeft(Pixmap& pixmap, TinyPoint& pos, string& text, double& size)
        {
            if (!text.empty())
            {
                DrawText(pixmap, pos, text, size);
            }
        }

        void Text::DrawText(Pixmap& pixmap, TinyPoint pos, string& text, double& size)
        {
            if (!text.empty())
            {
                for (char car : text)
                {
                    DrawChar(pixmap, pos, car, size);
                }
            }
        }

        void Text::DrawTextPente(Pixmap pixmap, TinyPoint pos, string text, int pente, double size)
        {
            if (!text.empty())
            {
                int y = pos.Y;
                int num = 0;
                for (char c : text)
                {
                    int charWidth = GetCharWidth(c, size);
                    DrawChar(pixmap, pos, c, size);
                    num += charWidth;
                    pos.Y = y + num / pente;
                }
            }
        }

        void Text::DrawTextCenter(Pixmap& pixmap, TinyPoint& pos, string& text, double& size)
        {
            if (!text.empty())
            {
                TinyPoint pos2;
                pos2.X = pos.X - GetTextWidth(text, size) / 2;
                pos2.Y = pos.Y;
                DrawText(pixmap, pos2, text, size);
            }
        }

        int Text::GetTextWidth(const string &text, double size)
        {
            if (text.empty())
            {
                return 0;
            }
            int num = 0;
            for (char c : text)
            {
                num += GetCharWidth(c, size);
            }
            return num;
        }

        int Text::GetOffset(char c)
        {
            for (int i = 0; i < 15; i++)
            {
                if ((short)c == table_accents[i])
                {
                    return 15 + i;
                }
            }
            if (c < '\0' || c > '\u0080')
            {
                return 1;
            }
            return c;
        }

        void Text::DrawChar(const Pixmap& pixmap, TinyPoint& pos, const char& car, const double& size)
        {
            TinyPoint pos2;
            int num = (short)car * 6;
            int rank = table_char[num];
            pos2.X = pos.X + table_char[num + 1];
            pos2.Y = pos.Y + table_char[num + 2];
            DrawCharSingle(pixmap, pos2, rank, size);
            rank = table_char[num + 3];
            if (rank != -1)
            {
                pos2.X = pos.X + table_char[num + 4];
                pos2.Y = pos.Y + table_char[num + 5];
                DrawCharSingle(pixmap, pos2, rank, size);
            }
            pos.X += GetCharWidth(car, size);
        }

        int Text::GetCharWidth(const char c, const double size)
        {
            return (int)((double)(table_width[table_char[(short)c * 6]] + 1) * size);
        }

        void Text::DrawCharSingle(Pixmap pixmap, TinyPoint pos, int rank, double size)
        {
            pixmap.DrawChar(rank, pos, size);
        }
}
