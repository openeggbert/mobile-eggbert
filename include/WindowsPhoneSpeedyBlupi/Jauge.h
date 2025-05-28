//
// Created by robertvokac on 5/24/25.
//

#ifndef JAUGE_H
#define JAUGE_H

#include "WindowsPhoneSpeedyBlupi/Pixmap.h"

namespace WindowsPhoneSpeedyBlupi {
    class Jauge {
    private:
        Pixmap m_pixmap;

        Sound m_sound;

        bool m_bHide;

        TinyPoint m_pos;

        TinyPoint m_dim;

        int m_mode;

        int m_level;

        bool m_bMinimizeRedraw;

        bool m_bRedraw;

        double m_zoom;

    public:
        NeoSdk::Property<double> Zoom;

        Jauge();

        bool Create(Pixmap& pixmap, Sound& sound, TinyPoint pos, int mode, bool bMinimizeRedraw);

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

#endif //JAUGE_H
