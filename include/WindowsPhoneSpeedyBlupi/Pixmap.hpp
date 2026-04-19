//
// Created by robertvokac on 5/24/25.
//

#ifndef PIXMAP_H
#define PIXMAP_H
#include <optional>

#include "Def.hpp"
#include "Game1I.hpp"
#include "PixmapI.hpp"

#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDeviceManager.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.hpp"
#include "WindowsPhoneSpeedyBlupi/TinyRect.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

// WindowsPhoneSpeedyBlupi, Version=1.0.0.5, Culture=neutral, PublicKeyToken=6db12cd62dbec439
// WindowsPhoneSpeedyBlupi.Pixmap
// using System;
// using System.Diagnostics;
// using Microsoft.Xna.Framework;
// using Microsoft.Xna.Framework.Graphics;
// using Microsoft.Xna.Framework.Input.Touch;
// using WindowsPhoneSpeedyBlupi;
// using static System.Net.Mime.MediaTypeNames;
// using static WindowsPhoneSpeedyBlupi.Def;
// using static WindowsPhoneSpeedyBlupi.EnvClasses;

namespace WindowsPhoneSpeedyBlupi
{
using Microsoft::Xna::Framework::Graphics::Texture2D;

    class Pixmap : public PixmapI
    {

    public:
        virtual ~Pixmap();

        //WindowsPhoneSpeedyBlupi::TinyPoint Origin;
        //
        // void DrawPart(int i, WindowsPhoneSpeedyBlupi::TinyPoint dest, const WindowsPhoneSpeedyBlupi::TinyRect & rect, double x);
        //
        // void DrawIcon(int i, int i1, const WindowsPhoneSpeedyBlupi::TinyRect & rect, double x, bool cond);
        //
        // void DrawChar(int rank, WindowsPhoneSpeedyBlupi::TinyPoint pos, double size);
        //////
    private:
        Game1I* game1;

        Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager graphics;

        double zoom = 0.0f;

        double originX = 0.0f;

        double originY = 0.0f;

        double hotSpotZoom = 0.0f;

        double hotSpotX = 0.0f;

        double hotSpotY = 0.0f;

        Microsoft::Xna::Framework::Graphics::SpriteBatch* spriteBatch;

        Texture2D bitmapText;

        Texture2D bitmapButton;

        Texture2D bitmapJauge;

        Texture2D bitmapBlupi;

        Texture2D bitmapBlupi1;

        Texture2D bitmapObject;

        Texture2D bitmapElement;

        Texture2D bitmapExplo;

        Texture2D bitmapPad;

        Texture2D bitmapSpeedyBlupi;

        Texture2D bitmapBlupiYoupie;

        Texture2D bitmapGear;

        Texture2D bitmapBackground;

        Microsoft::Xna::Framework::Vector2 origin;

        Microsoft::Xna::Framework::Graphics::SpriteEffects effect;

    public:
        public: [[nodiscard]] TinyRect getDrawBoundsProperty() override;
        public: [[nodiscard]] TinyPoint getOriginProperty() const;

        Pixmap(Game1I* game1, Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager &graphics);

        Pixmap(const Pixmap&);
        Pixmap& operator=(const Pixmap&);
         TinyPoint HotSpotToHud(TinyPoint& pos) override;

         void SetHotSpot(double zoom, double x, double y) override;

         void DrawInputButton(TinyRect rect, Def::ButtonGlyph &glyph, bool &pressed, bool &selected) override;

         void LoadContent() override;

    private:
        void UpdateGeometry();

    public:
        void BackgroundCache(const std::string &name) override;

         bool Start() override;

         bool Finish() override;

         void DrawBackground() override;

         void DrawChar(int rank, TinyPoint& pos, double size) override;

         void HudIcon(int channel, int rank, TinyPoint& pos) override;

         void QuickIcon(int channel, int rank, TinyPoint& pos) override;

         void QuickIcon(int channel, int rank, TinyPoint& pos, double opacity, double rotation) override;

         bool DrawPart(int channel, TinyPoint& dest, TinyRect& rect) override;

         bool DrawPart(int channel, TinyPoint& dest, TinyRect& rect, double zoom) override;

         void DrawIcon(int channel, int icon, TinyRect rect, double opacity, bool useHotSpot) override;

         void DrawIcon(int channel, int icon, TinyRect rect, double opacity, double rotationDeg, bool useHotSpot) override;

    private:
        Microsoft::Xna::Framework::Rectangle GetSrcRectangle(Texture2D& bitmap, int bitmapGridX, int bitmapGridY, int iconWidth, int iconHeight, int gap, int icon);

        Microsoft::Xna::Framework::Rectangle GetDstRectangle(TinyRect& rect, int iconWidth, int iconHeight, bool useHotSpot);

         std::optional<Texture2D> GetBitmap(int channel);

    };

}

#endif //PIXMAP_H
