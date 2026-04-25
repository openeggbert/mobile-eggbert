#pragma once

#include "IPixmap.hpp"
#include "ISound.hpp"

namespace WindowsPhoneSpeedyBlupi {

    class Jauge {
    private:
        IPixmap* m_pixmap;
        //Todo: Check (m_sound is not used).
        ISound* m_sound;

        bool m_bHide;

        TinyPoint m_pos;

        TinyPoint m_dim;

        int m_mode;

        int m_level;

        bool m_bMinimizeRedraw;

        bool m_bRedraw;

        double m_zoom;

    public:
        [[nodiscard]] double getZoomProperty() const;

        void setZoomProperty(double v);

        Jauge();

        bool Create(IPixmap* pixmap, ISound* sound, TinyPoint pos, int mode, bool bMinimizeRedraw);

        void Draw();

        void Redraw();

        int GetLevel();

        void SetLevel(int level);

        int GetMode();

        void SetMode(int mode);

        bool GetHide();

        void SetHide(bool bHide);

        [[nodiscard]] TinyPoint GetPos() const;

        void SetRedraw();


    };
}

