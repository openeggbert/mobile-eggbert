//
// Created by robertvokac on 5/24/25.
//
#ifndef PIXMAPI_H
#define PIXMAPI_H

#include "Def.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyRect.hpp"

namespace WindowsPhoneSpeedyBlupi {
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    class PixmapI {
    protected:
        virtual ~PixmapI() = default;

    public:
        [[nodiscard]] virtual TinyRect getDrawBoundsProperty() = 0;

        [[nodiscard]] virtual TinyPoint getOriginProperty() const = 0;

        virtual TinyPoint HotSpotToHud(TinyPoint &pos) = 0;

        virtual void SetHotSpot(double zoom, double x, double y) = 0;

        virtual void DrawInputButton(TinyRect rect, Def::ButtonGlyph &glyph, bool &pressed, bool &selected) = 0;

        virtual void LoadContent() = 0;

        virtual void BackgroundCache(const std::string &name) = 0;

        virtual bool Start() = 0;

        virtual bool Finish() = 0;

        virtual void DrawBackground() = 0;

        virtual void DrawChar(int rank, TinyPoint &pos, double size) = 0;

        virtual void HudIcon(int channel, int rank, TinyPoint &pos) = 0;

        virtual void QuickIcon(int channel, int rank, TinyPoint &pos) = 0;

        virtual void QuickIcon(int channel, int rank, TinyPoint &pos, double opacity, double rotation) = 0;

        virtual bool DrawPart(int channel, TinyPoint &dest, TinyRect &rect) = 0;

        virtual bool DrawPart(int channel, TinyPoint &dest, TinyRect &rect, double zoom) = 0;

        virtual void DrawIcon(int channel, int icon, TinyRect rect, double opacity, bool useHotSpot) = 0;

        virtual void DrawIcon(int channel, int icon, TinyRect rect, double opacity, double rotationDeg, bool useHotSpot) = 0;
    };
}


#endif // PIXMAPI_H
