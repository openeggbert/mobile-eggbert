//
// Created by robertvokac on 5/24/25.
//

#ifndef JAUGE_H
#define JAUGE_H

#include "Pixmap.h"

namespace WindowsPhoneSpeedyBlupi {
    class Jauge {
    private:
        std::optional<Pixmap> m_pixmap;

        std::optional<Sound> m_sound;

        bool m_bHide;

        TinyPoint m_pos;

        TinyPoint m_dim;

        int m_mode;

        int m_level;

        bool m_bMinimizeRedraw;

        bool m_bRedraw;

        double m_zoom;

    public:
        public: [[nodiscard]] double getZoom() const; public: void setZoom(const double& v);

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
