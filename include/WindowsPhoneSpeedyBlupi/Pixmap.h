//
// Created by robertvokac on 5/24/25.
//

#ifndef PIXMAP_H
#define PIXMAP_H
#include <optional>

#include "Def.h"
#include "Game1.h"
#include "Misc.h"
#include "Tables.h"
#include "Text.h"
#include "Microsoft/Xna/Framework/Vector2.h"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDeviceManager.h"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.h"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.h"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.h"
#include "WindowsPhoneSpeedyBlupi/TinyPoint.h"
#include "WindowsPhoneSpeedyBlupi/TinyRect.h"
#include "Microsoft/Xna/Framework/Color.h"
#include "Microsoft/Xna/Framework/Rectangle.h"
#include "Microsoft/Xna/Framework/Graphics/BlendState.h"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.h"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.h"

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

    class Pixmap
    {

    public:
        //WindowsPhoneSpeedyBlupi::TinyPoint Origin;
        //
        // void DrawPart(int i, WindowsPhoneSpeedyBlupi::TinyPoint dest, const WindowsPhoneSpeedyBlupi::TinyRect & rect, double x);
        //
        // void DrawIcon(int i, int i1, const WindowsPhoneSpeedyBlupi::TinyRect & rect, double x, bool cond);
        //
        // void DrawChar(int rank, WindowsPhoneSpeedyBlupi::TinyPoint pos, double size);
        //////
    private:
        const Game1 game1;

        const Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager graphics;

        double zoom = 0.0f;

        double originX = 0.0f;

        double originY = 0.0f;

        double hotSpotZoom = 0.0f;

        double hotSpotX = 0.0f;

        double hotSpotY = 0.0f;

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch;

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
        NeoSdk::Property<TinyRect> DrawBounds;
        NeoSdk::Property<TinyPoint> Origin;

         Pixmap(Game1& game1, Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager& graphics);

         TinyPoint HotSpotToHud(TinyPoint& pos);

         void SetHotSpot(double zoom, double x, double y)
        {
            hotSpotZoom = zoom;
            hotSpotX = x;
            hotSpotY = y;
        }

         void DrawInputButton(TinyRect& rect, Def::ButtonGlyph& glyph, bool& pressed, bool& selected);

         void LoadContent()
        {
            spriteBatch = Microsoft::Xna::Framework::Graphics::SpriteBatch(&(game1.GraphicsDevice.get()));
            bitmapText = game1.Content.get().Load<Texture2D>("icons/text");
            bitmapButton = game1.Content.get().Load<Texture2D>("icons/button");
            bitmapJauge = game1.Content.get().Load<Texture2D>("icons/jauge");
            bitmapBlupi = game1.Content.get().Load<Texture2D>("icons/blupi");
            bitmapBlupi1 = game1.Content.get().Load<Texture2D>("icons/blupi1");
            bitmapObject = game1.Content.get().Load<Texture2D>("icons/object-m");
            bitmapElement = game1.Content.get().Load<Texture2D>("icons/element");
            bitmapExplo = game1.Content.get().Load<Texture2D>("icons/explo");
            bitmapPad = game1.Content.get().Load<Texture2D>("icons/pad");
            bitmapSpeedyBlupi = game1.Content.get().Load<Texture2D>("backgrounds/speedyblupi");
            bitmapBlupiYoupie = game1.Content.get().Load<Texture2D>("backgrounds/blupiyoupie");
            bitmapGear = game1.Content.get().Load<Texture2D>("backgrounds/gear");
            UpdateGeometry();
        }

    private:
        void UpdateGeometry()
        {
            double screenWidth = graphics.GraphicsDevice.get().Viewport.get().Width;
            double screenHeight = graphics.GraphicsDevice.get().Viewport.get().Height;
            if (Def::PLATFORM == Def::Platform::Android && screenHeight > 480)
            {
                screenWidth = screenHeight * (640.0f / 480.0f);
            }
            double val = screenWidth / 640.0;
            double val2 = screenHeight / 480.0;
            zoom = std::min(val, val2);
            originX = (screenWidth - 640.0 * zoom) / 2.0;
            originY = (screenHeight - 480.0 * zoom) / 2.0;
        }

    public:
        void BackgroundCache(string name)
        {
            bitmapBackground = game1.Content.get().Load<Texture2D>("backgrounds/" + name);
        }

         bool Start()
        {
            graphics.GraphicsDevice.Clear(Microsoft::Xna::Framework::CornflowerBlue);
            return true;
        }

         bool Finish()
        {
            return true;
        }

         void DrawBackground()
        {
            double screenWidth = graphics.GraphicsDevice.get().Viewport.get().Width;
            double screenHeight = graphics.GraphicsDevice.get().Viewport.get().Height;
            if (Def::PLATFORM == Def::Platform::Android && screenHeight > 480)
            {
                screenWidth = screenHeight * (640.0f / 480.0f);
            }
            Texture2D bitmap = GetBitmap(3).value();
            Microsoft::Xna::Framework::Rectangle srcRectangle = GetSrcRectangle(bitmap, 10, 10, 10, 10, 0, 0);
            Microsoft::Xna::Framework::Rectangle destinationRectangle = Microsoft::Xna::Framework::Rectangle(0, 0, (int)screenWidth, (int)screenHeight);
            spriteBatch.Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront, Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
            spriteBatch.Draw(bitmap, destinationRectangle, srcRectangle, Microsoft::Xna::Framework::White);
            spriteBatch.End();
            TinyPoint tinyPoint;
            tinyPoint.X = (int)originX;
            tinyPoint.Y = (int)originY;
            TinyPoint dest = tinyPoint;
            TinyRect tinyRect;
            tinyRect.LeftX = 0;
            tinyRect.TopY = 0;
            tinyRect.RightX = 640;
            tinyRect.BottomY = 480;
            TinyRect rect = tinyRect;
            DrawPart(3, dest, rect);
        }

         void DrawChar(int rank, TinyPoint pos, double size)
        {
            pos.X = (int)((double)pos.X + originX);
            pos.Y = (int)((double)pos.Y + originY);
            TinyRect tinyRect;
            tinyRect.LeftX = pos.X;
            tinyRect.TopY = pos.Y;
            tinyRect.RightX = pos.X + (int)(32.0 * size);
            tinyRect.BottomY = pos.Y + (int)(32.0 * size);
            TinyRect rect = tinyRect;
            DrawIcon(6, rank, rect, 1.0, false);
        }

         void HudIcon(int channel, int rank, TinyPoint pos)
        {
            pos.X = (int)((double)pos.X + originX);
            pos.Y = (int)((double)pos.Y + originY);
            TinyRect tinyRect;
            tinyRect.LeftX = pos.X;
            tinyRect.TopY = pos.Y;
            tinyRect.RightX = pos.X;
            tinyRect.BottomY = pos.Y;
            TinyRect rect = tinyRect;
            DrawIcon(channel, rank, rect, 1.0, false);
        }

         void QuickIcon(int channel, int rank, TinyPoint pos)
        {
            TinyRect tinyRect;
            tinyRect.LeftX = pos.X;
            tinyRect.TopY = pos.Y;
            tinyRect.RightX = pos.X;
            tinyRect.BottomY = pos.Y;
            TinyRect rect = tinyRect;
            DrawIcon(channel, rank, rect, 1.0, true);
        }

         void QuickIcon(int channel, int rank, TinyPoint pos, double opacity, double rotation)
        {
            TinyRect tinyRect;
            tinyRect.LeftX = pos.X;
            tinyRect.TopY = pos.Y;
            tinyRect.RightX = pos.X;
            tinyRect.BottomY = pos.Y;
            TinyRect rect = tinyRect;
            DrawIcon(channel, rank, rect, opacity, rotation, true);
        }

         bool DrawPart(int channel, TinyPoint dest, TinyRect rect)
        {
            return DrawPart(channel, dest, rect, 1.0);
        }

         bool DrawPart(int channel, TinyPoint dest, TinyRect rect, double zoom)
        {
            std::optional<Texture2D> bitmap = GetBitmap(channel);
            if (!bitmap.has_value())
            {
                return false;
            }
            if (channel == 5)
            {
                dest.X = (int)((double)dest.X + originX);
                dest.Y = (int)((double)dest.Y + originY);
            }
            Microsoft::Xna::Framework::Rectangle value = Microsoft::Xna::Framework::Rectangle(rect.LeftX, rect.TopY, rect.Width, rect.Height);
            Microsoft::Xna::Framework::Rectangle destinationRectangle = Microsoft::Xna::Framework::Rectangle(dest.X, dest.Y, (int)((double)rect.Width * zoom), (int)((double)rect.Height * zoom));
            spriteBatch.Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront, Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
            spriteBatch.Draw(bitmap.value(), destinationRectangle, value, Microsoft::Xna::Framework::White);
            spriteBatch.End();
            return true;
        }

         void DrawIcon(int channel, int icon, TinyRect rect, double opacity, bool useHotSpot)
        {
            DrawIcon(channel, icon, rect, opacity, 0.0, useHotSpot);
        }

         void DrawIcon(int channel, int icon, TinyRect rect, double opacity, double rotationDeg, bool useHotSpot)
        {
            if (icon == -1)
            {
                return;
            }
            if (channel == 14 && !Microsoft::Xna::Framework::Input::Touch::TouchPanel::GetCapabilities().IsConnected)
            {
                int padGameplayIconNumbers[] = { 0, 1, 2, 3, 30, 12, 23 };
                for (int iconNumber : padGameplayIconNumbers)
                {
                    if(iconNumber == icon)
                    {
                        if(iconNumber == 1 && rect.LeftX > 100) { continue; }
                        //Touch display is not connected and the icon is a gameplay icon. Nothing to do.
                        return;
                    }
                }

            }
            std::optional<Texture2D> bitmap = GetBitmap(channel);
            if (!bitmap.has_value())
            {
                return;
            }
            int bitmapGridX;
            int bitmapGridY;
            int iconWidth;
            int iconHeight;
            int gap;
            switch (channel)
            {
                case 2:
                case 11:
                case 12:
                case 13:
                    bitmapGridX = 60;
                    bitmapGridY = 60;
                    iconWidth = 60;
                    iconHeight = 60;
                    gap = 0;
                    break;
                case 1:
                    bitmapGridX = 64;
                    bitmapGridY = 64;
                    iconWidth = 64;
                    iconHeight = 64;
                    gap = 1;
                    break;
                case 10:
                    bitmapGridX = 60;
                    bitmapGridY = 60;
                    iconWidth = 60;
                    iconHeight = 60;
                    gap = 0;
                    break;
                case 9:
                    bitmapGridX = 144;
                    bitmapGridY = 144;
                    iconHeight = Tables::table_explo_size[icon];
                    iconWidth = std::max(iconHeight, 128);
                    gap = 0;
                    break;
                case 6:
                    bitmapGridX = 32;
                    bitmapGridY = 32;
                    iconWidth = 32;
                    iconHeight = 32;
                    gap = 0;
                    break;
                case 4:
                    bitmapGridX = 40;
                    bitmapGridY = 40;
                    iconWidth = 40;
                    iconHeight = 40;
                    gap = 0;
                    break;
                case 14:
                    bitmapGridX = 140;
                    bitmapGridY = 140;
                    iconWidth = 140;
                    iconHeight = 140;
                    gap = 0;
                    break;
                case 15:
                    bitmapGridX = 640;
                    bitmapGridY = 160;
                    iconWidth = 640;
                    iconHeight = 160;
                    gap = 0;
                    break;
                case 16:
                    bitmapGridX = 410;
                    bitmapGridY = 380;
                    iconWidth = 410;
                    iconHeight = 380;
                    gap = 0;
                    break;
                case 17:
                    bitmapGridX = 226;
                    bitmapGridY = 226;
                    iconWidth = 226;
                    iconHeight = 226;
                    gap = 0;
                    break;
                default:
                    bitmapGridX = 0;
                    bitmapGridY = 0;
                    iconWidth = 0;
                    iconHeight = 0;
                    gap = 0;
                    break;
            }
            if (bitmapGridX != 0)
            {
                Microsoft::Xna::Framework::Rectangle srcRectangle = GetSrcRectangle(bitmap, bitmapGridX, bitmapGridY, iconWidth, iconHeight, gap, icon);
                Microsoft::Xna::Framework::Rectangle rectangle = GetDstRectangle(rect, iconWidth, iconHeight, useHotSpot);
                float rotationRad = 0.0f;
                if (rotationDeg != 0.0)
                {
                    rotationRad = (float)Misc::DegToRad(rotationDeg);
                    rectangle = Misc::RotateAdjust(rectangle, rotationRad);
                }
                spriteBatch.Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront, Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
                spriteBatch.Draw(bitmap, rectangle, srcRectangle, Microsoft::Xna::Framework::Color::FromNonPremultiplied(255, 255, 255, (int)(255.0 * opacity)), rotationRad, origin, effect, 0.0f);
                spriteBatch.End();
            }
        }

    private:
        Microsoft::Xna::Framework::Rectangle GetSrcRectangle(Texture2D bitmap, int bitmapGridX, int bitmapGridY, int iconWidth, int iconHeight, int gap, int icon)
        {
            int width = bitmap.Bounds.Width;
            int height = bitmap.Bounds.Height;
            int num = icon % (width / bitmapGridX);
            int num2 = icon / (width / bitmapGridX);
            bitmapGridX += gap;
            bitmapGridY += gap;
            return new Rectangle(gap + num * bitmapGridX, gap + num2 * bitmapGridY, iconWidth, iconHeight);
        }

         Rectangle GetDstRectangle(TinyRect rect, int iconWidth, int iconHeight, bool useHotSpot)
        {
            int num = ((rect.Width == 0) ? iconWidth : rect.Width);
            int num2 = ((rect.Height == 0) ? iconHeight : rect.Height);
            int num3 = (int)((double)rect.LeftX * zoom);
            int num4 = (int)((double)rect.TopY * zoom);
            int num5 = (int)((double)num3 + (double)num * zoom);
            int num6 = (int)((double)num4 + (double)num2 * zoom);
            if (useHotSpot && hotSpotZoom > 1.0)
            {
                num3 -= (int)hotSpotX;
                num4 -= (int)hotSpotY;
                num5 -= (int)hotSpotX;
                num6 -= (int)hotSpotY;
                num3 = (int)((double)num3 * hotSpotZoom);
                num4 = (int)((double)num4 * hotSpotZoom);
                num5 = (int)((double)num5 * hotSpotZoom);
                num6 = (int)((double)num6 * hotSpotZoom);
                num3 += (int)hotSpotX;
                num4 += (int)hotSpotY;
                num5 += (int)hotSpotX;
                num6 += (int)hotSpotY;
            }
            return new Rectangle(num3, num4, num5 - num3, num6 - num4);
        }

         std::optional<Texture2D> GetBitmap(int channel)
        {
            switch (channel)
            {
                case 2:
                    return bitmapBlupi;
                case 11:
                case 12:
                case 13:
                    return bitmapBlupi1;
                case 1:
                    return bitmapObject;
                case 10:
                    return bitmapElement;
                case 9:
                    return bitmapExplo;
                case 6:
                    return bitmapText;
                case 4:
                    return bitmapButton;
                case 5:
                    return bitmapJauge;
                case 14:
                    return bitmapPad;
                case 15:
                    return bitmapSpeedyBlupi;
                case 16:
                    return bitmapBlupiYoupie;
                case 17:
                    return bitmapGear;
                case 3:
                    return bitmapBackground;
                default:
                    return std::nullopt;;
            }
        }
    };

}

#endif //PIXMAP_H
