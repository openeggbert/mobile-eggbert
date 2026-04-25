#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"

#include "WindowsPhoneSpeedyBlupi/Jauge.hpp"

namespace WindowsPhoneSpeedyBlupi
{
    double Jauge::getZoomProperty() const { return m_zoom; }
    void Jauge::setZoomProperty(double v) { m_zoom = v; }
    Jauge::Jauge() : m_pixmap(nullptr), m_sound(nullptr), m_mode(0),
                     m_bHide(true),
                     m_bMinimizeRedraw(false),
                     m_bRedraw(false),
                     m_zoom(1.0),
                     m_level(0)
    {
    }


        bool Jauge::Create(IPixmap* pixmap, ISound* sound, TinyPoint pos, int mode, bool bMinimizeRedraw)
        {
            m_pixmap = pixmap;
            m_sound = sound;
            m_mode = mode;
            m_bMinimizeRedraw = bMinimizeRedraw;
            m_bHide = true;
            m_pos = pos;
            m_dim.X = 124;
            m_dim.Y = 22;
            m_level = 0;
            m_bRedraw = true;
            return true;
        }

        void Jauge::Draw()
        {
        if (m_pixmap == nullptr)
        {
            return;
        }
            TinyRect rect{};
            if (m_bMinimizeRedraw && !m_bRedraw)
            {
                return;
            }
            m_bRedraw = false;
            if (!m_bHide)
            {
                int num = m_level * 114 / 100;
                rect.Left = 0;
                rect.Right = 124;
                rect.Top = 0;
                rect.Bottom = 22;
                m_pixmap->DrawPart(5, m_pos, rect, m_zoom);
                if (num > 0)
                {
                    rect.Left = 0;
                    rect.Right = 6 + num;
                    rect.Top = 22 * m_mode;
                    rect.Bottom = 22 * (m_mode + 1);
                    m_pixmap->DrawPart(5, m_pos, rect, m_zoom);
                }
            }
        }

        void Jauge::Redraw()
        {
            m_bRedraw = true;
        }

        int Jauge::GetLevel()
        {
            return m_level;
        }

        void Jauge::SetLevel(int level)
        {
            if (level < 0)
            {
                level = 0;
            }
            if (level > 100)
            {
                level = 100;
            }
            if (m_level != level)
            {
                m_bRedraw = true;
            }
            m_level = level;
        }

        int Jauge::GetMode()
        {
            return m_mode;
        }

        void Jauge::SetMode(int mode)
        {
            if (m_mode != mode)
            {
                m_bRedraw = true;
            }
            m_mode = mode;
        }

        bool Jauge::GetHide()
        {
            return m_bHide;
        }

        void Jauge::SetHide(bool bHide)
        {
            if (m_bHide != bHide)
            {
                m_bRedraw = true;
            }
            m_bHide = bHide;
        }

        TinyPoint Jauge::GetPos() const {
            return m_pos;
        }

        void Jauge::SetRedraw()
        {
            m_bRedraw = true;
        }


}