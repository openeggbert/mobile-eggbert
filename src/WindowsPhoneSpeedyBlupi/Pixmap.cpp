//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Pixmap.hpp"

#include "CNA/Platform.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "WindowsPhoneSpeedyBlupi/Decor.hpp"
#include "WindowsPhoneSpeedyBlupi/Def.hpp"

#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "WindowsPhoneSpeedyBlupi/Misc.hpp"
#include "WindowsPhoneSpeedyBlupi/Tables.hpp"
#include "WindowsPhoneSpeedyBlupi/Text.hpp"

namespace WindowsPhoneSpeedyBlupi {
    Pixmap::~Pixmap()
    {
        delete spriteBatch;
        spriteBatch = nullptr;
    }

    TinyRect Pixmap::getDrawBoundsProperty() {
        TinyRect result;
        double screenWidth = graphics.getGraphicsDeviceProperty()->getViewportProperty().getWidthProperty();
        double screenHeight = graphics.getGraphicsDeviceProperty()->getViewportProperty().getHeightProperty();
        if (CNA::getCurrentPlatform() == CNA::Platform::Android && screenHeight > 480) {
            screenWidth = screenHeight * (640.0f / 480.0f);
        }
        if (screenWidth != 0.0 && screenHeight != 0.0) {
            double num3;
            double num4;
            if (screenWidth / screenHeight < 1.3333333333333333) {
                num3 = 640.0;
                num4 = 640.0 * (screenHeight / screenWidth);
            } else {
                num3 = 480.0 * (screenWidth / screenHeight);
                num4 = 480.0;
            }
            result.Left = 0;
            result.Right = (int) num3;
            result.Top = 0;
            result.Bottom = (int) num4;
        }
        return result;
    }
    TinyPoint Pixmap::getOriginProperty() const {
        TinyPoint result;
        result.X = (int) originX;
        result.Y = (int) originY;
        return result;
    }

    Pixmap::Pixmap(IGame1* game1, Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager& graphics):
        game1(game1),
        graphics(graphics), spriteBatch(nullptr), bitmapText(Texture2D()), bitmapButton(Texture2D()), bitmapJauge(Texture2D()), bitmapBlupi(Texture2D()), bitmapBlupi1(Texture2D()),
        bitmapObject(Texture2D()),
        bitmapElement(Texture2D()),
        bitmapExplo(Texture2D()), bitmapPad(Texture2D()),
        bitmapSpeedyBlupi(Texture2D()),
        bitmapBlupiYoupie(Texture2D()),
        bitmapGear(Texture2D()),
        bitmapBackground(Texture2D()),
        origin(Microsoft::Xna::Framework::Vector2(0.0f,0.0f)) {
        effect = Microsoft::Xna::Framework::Graphics::SpriteEffects::None;
    }

    TinyPoint Pixmap::HotSpotToHud(TinyPoint& pos)
    {
        TinyPoint result;
        result.X = (int)((double)(pos.X - (int)hotSpotX) / hotSpotZoom) + (int)hotSpotX - (int)originX;
        result.Y = (int)((double)(pos.Y - (int)hotSpotY) / hotSpotZoom) + (int)hotSpotY - (int)originY;
        return result;
    }

    void Pixmap::SetHotSpot(const double zoom, const double x, const double y)
    {
        hotSpotZoom = zoom;
        hotSpotX = x;
        hotSpotY = y;
    }
            void Pixmap::DrawInputButton(TinyRect rect, Def::ButtonGlyph& glyph, bool& pressed, bool& selected)
        {
            switch (glyph)
            {
                case Def::ButtonGlyph::InitGamerB:
                    DrawIcon(14, selected ? 17 : 5, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::InitGamerA:
                    DrawIcon(14, selected ? 16 : 4, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::InitGamerC:
                    DrawIcon(14, selected ? 18 : 6, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::InitSetup:
                case Def::ButtonGlyph::PauseSetup:
                    DrawIcon(14, 19, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::InitPlay:
                    DrawIcon(14, 7, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::PauseMenu:
                case Def::ButtonGlyph::ResumeMenu:
                    DrawIcon(14, 11, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::PauseBack:
                    DrawIcon(14, 8, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::PauseRestart:
                    DrawIcon(14, 9, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::PauseContinue:
                case Def::ButtonGlyph::ResumeContinue:
                    DrawIcon(14, 10, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::WinLostReturn:
                    DrawIcon(14, 3, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::InitBuy:
                case Def::ButtonGlyph::TrialBuy:
                    DrawIcon(14, 22, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::InitRanking:
                    DrawIcon(14, 12, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::TrialCancel:
                case Def::ButtonGlyph::RankingContinue:
                    DrawIcon(14, 8, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::SetupSounds:
                case Def::ButtonGlyph::SetupJump:
                case Def::ButtonGlyph::SetupZoom:
                case Def::ButtonGlyph::SetupAccel:
                    DrawIcon(14, selected ? 13 : 21, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::SetupReset:
                    DrawIcon(14, 20, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::SetupReturn:
                    DrawIcon(14, 8, rect, pressed ? 0.8 : 1.0, false);
                    break;
                case Def::ButtonGlyph::PlayJump:
                    DrawIcon(14, 2, rect, pressed ? 0.6 : 1.0, false);
                    break;
                case Def::ButtonGlyph::PlayAction:
                    DrawIcon(14, 12, rect, pressed ? 0.6 : 1.0, false);
                    break;
                case Def::ButtonGlyph::PlayDown:
                    DrawIcon(14, 23, rect, pressed ? 0.6 : 1.0, false);
                    break;
                case Def::ButtonGlyph::PlayPause:
                    DrawIcon(14, 3, rect, pressed ? 0.6 : 1.0, false);
                    break;
                case Def::ButtonGlyph::Cheat1:
                case Def::ButtonGlyph::Cheat2:
                case Def::ButtonGlyph::Cheat3:
                case Def::ButtonGlyph::Cheat4:
                case Def::ButtonGlyph::Cheat5:
                case Def::ButtonGlyph::Cheat6:
                case Def::ButtonGlyph::Cheat7:
                case Def::ButtonGlyph::Cheat8:
                case Def::ButtonGlyph::Cheat9:
                    {
                        DrawIcon(14, 0, rect, pressed ? 0.6 : 1.0, false);
                        TinyPoint tinyPoint;
                        tinyPoint.X = rect.Left + rect.getWidthProperty() / 2 - (int)originX;
                        tinyPoint.Y = rect.Top + 28;
                        TinyPoint pos = tinyPoint;
                        Text::DrawTextCenter(this, pos, Decor::GetCheatTinyText(glyph), 1.0);
                        break;
                    }
                case Def::ButtonGlyph::Cheat11:
                case Def::ButtonGlyph::Cheat12:
                case Def::ButtonGlyph::Cheat21:
                case Def::ButtonGlyph::Cheat22:
                case Def::ButtonGlyph::Cheat31:
                case Def::ButtonGlyph::Cheat32:
                    break;
            }
        }

         void Pixmap::LoadContent()
        {
            Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDeviceProperty = game1-> getGraphicsDeviceProperty();
            spriteBatch = new Microsoft::Xna::Framework::Graphics::SpriteBatch(game1->getGraphicsDeviceProperty());
            bitmapText = game1->getContentProperty().Load<Texture2D>("icons/text");
            bitmapButton = game1->getContentProperty().Load<Texture2D>("icons/button");
            bitmapJauge = game1->getContentProperty().Load<Texture2D>("icons/jauge");
            bitmapBlupi = game1->getContentProperty().Load<Texture2D>("icons/blupi");
            bitmapBlupi1 = game1->getContentProperty().Load<Texture2D>("icons/blupi1");
            bitmapObject = game1->getContentProperty().Load<Texture2D>("icons/object-m");
            bitmapElement = game1->getContentProperty().Load<Texture2D>("icons/element");
            bitmapExplo = game1->getContentProperty().Load<Texture2D>("icons/explo");
            bitmapPad = game1->getContentProperty().Load<Texture2D>("icons/pad");
            bitmapSpeedyBlupi = game1->getContentProperty().Load<Texture2D>("backgrounds/speedyblupi");
            bitmapBlupiYoupie = game1->getContentProperty().Load<Texture2D>("backgrounds/blupiyoupie");
            bitmapGear = game1->getContentProperty().Load<Texture2D>("backgrounds/gear");
            UpdateGeometry();
        }

        void Pixmap::UpdateGeometry()
        {
            double screenWidth = graphics.getGraphicsDeviceProperty()->getViewportProperty().getWidthProperty();
            double screenHeight = graphics.getGraphicsDeviceProperty()->getViewportProperty().getHeightProperty();
            if (CNA::getCurrentPlatform() == CNA::Platform::Android && screenHeight > 480)
            {
                screenWidth = screenHeight * (640.0f / 480.0f);
            }
            double val = screenWidth / 640.0;
            double val2 = screenHeight / 480.0;
            zoom = std::min(val, val2);
            originX = (screenWidth - 640.0 * zoom) / 2.0;
            originY = (screenHeight - 480.0 * zoom) / 2.0;
        }

        void Pixmap::BackgroundCache(const string &name)
        {
            bitmapBackground = game1->getContentProperty().Load<Texture2D>("backgrounds/" + name);
        }

         bool Pixmap::Start()
        {
            graphics.getGraphicsDeviceProperty()->Clear(Microsoft::Xna::Framework::CornflowerBlue);
            return true;
        }

         bool Pixmap::Finish()
        {
            return true;
        }

         void Pixmap::DrawBackground()
        {
            double screenWidth = graphics.getGraphicsDeviceProperty()->getViewportProperty().getWidthProperty();
            double screenHeight = graphics.getGraphicsDeviceProperty()->getViewportProperty().getHeightProperty();
            if (CNA::getCurrentPlatform() == CNA::Platform::Android && screenHeight > 480)
            {
                screenWidth = screenHeight * (640.0f / 480.0f);
            }
            Texture2D bitmap = GetBitmap(3).value();
            Microsoft::Xna::Framework::Rectangle srcRectangle = GetSrcRectangle(bitmap, 10, 10, 10, 10, 0, 0);
            Microsoft::Xna::Framework::Rectangle destinationRectangle = Microsoft::Xna::Framework::Rectangle(0, 0, (int)screenWidth, (int)screenHeight);
            spriteBatch->Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront, Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
            spriteBatch->Draw(bitmap, destinationRectangle, srcRectangle, Microsoft::Xna::Framework::White);
            spriteBatch->End();
            TinyPoint tinyPoint;
            tinyPoint.X = (int)originX;
            tinyPoint.Y = (int)originY;
            TinyPoint dest = tinyPoint;
            TinyRect tinyRect;
            tinyRect.Left = 0;
            tinyRect.Top = 0;
            tinyRect.Right = 640;
            tinyRect.Bottom = 480;
            TinyRect rect = tinyRect;
            DrawPart(3, dest, rect);
        }

         void Pixmap::DrawChar(int rank, TinyPoint& pos, double size)
        {
            pos.X = (int)((double)pos.X + originX);
            pos.Y = (int)((double)pos.Y + originY);
            TinyRect tinyRect;
            tinyRect.Left = pos.X;
            tinyRect.Top = pos.Y;
            tinyRect.Right = pos.X + (int)(32.0 * size);
            tinyRect.Bottom = pos.Y + (int)(32.0 * size);
            TinyRect rect = tinyRect;
            DrawIcon(6, rank, rect, 1.0, false);
        }

         void Pixmap::HudIcon(int channel, int rank, TinyPoint& pos)
        {
            pos.X = (int)((double)pos.X + originX);
            pos.Y = (int)((double)pos.Y + originY);
            TinyRect tinyRect;
            tinyRect.Left = pos.X;
            tinyRect.Top = pos.Y;
            tinyRect.Right = pos.X;
            tinyRect.Bottom = pos.Y;
            TinyRect rect = tinyRect;
            DrawIcon(channel, rank, rect, 1.0, false);
        }

         void Pixmap::QuickIcon(int channel, int rank, TinyPoint& pos)
        {
            TinyRect tinyRect;
            tinyRect.Left = pos.X;
            tinyRect.Top = pos.Y;
            tinyRect.Right = pos.X;
            tinyRect.Bottom = pos.Y;
            TinyRect rect = tinyRect;
            DrawIcon(channel, rank, rect, 1.0, true);
        }

         void Pixmap::QuickIcon(int channel, int rank, TinyPoint& pos, double opacity, double rotation)
        {
            TinyRect tinyRect;
            tinyRect.Left = pos.X;
            tinyRect.Top = pos.Y;
            tinyRect.Right = pos.X;
            tinyRect.Bottom = pos.Y;
            TinyRect rect = tinyRect;
            DrawIcon(channel, rank, rect, opacity, rotation, true);
        }

         bool Pixmap::DrawPart(int channel, TinyPoint& dest, TinyRect& rect)
        {
            return DrawPart(channel, dest, rect, 1.0);
        }

         bool Pixmap::DrawPart(int channel, TinyPoint& dest, TinyRect& rect, double zoom)
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
            Microsoft::Xna::Framework::Rectangle value = Microsoft::Xna::Framework::Rectangle(rect.Left, rect.Top, rect.getWidthProperty(), rect.getHeightProperty());
            Microsoft::Xna::Framework::Rectangle destinationRectangle = Microsoft::Xna::Framework::Rectangle(dest.X, dest.Y, (int)((double)rect.getWidthProperty() * zoom), (int)((double)rect.getHeightProperty() * zoom));
            spriteBatch->Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront, Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
            spriteBatch->Draw(bitmap.value(), destinationRectangle, value, Microsoft::Xna::Framework::White);
            spriteBatch->End();
            return true;
        }

         void Pixmap::DrawIcon(int channel, int icon, TinyRect rect, double opacity, bool useHotSpot)
        {
            DrawIcon(channel, icon, rect, opacity, 0.0, useHotSpot);
        }

         void Pixmap::DrawIcon(int channel, int icon, TinyRect rect, double opacity, double rotationDeg, bool useHotSpot)
        {
            if (icon == -1)
            {
                return;
            }
        using Microsoft::Xna::Framework::Input::Touch::TouchPanel;
            if (channel == 14 && !TouchPanel::GetCapabilities().getIsConnectedProperty())
            {
                int padGameplayIconNumbers[] = { 0, 1, 2, 3, 30, 12, 23 };
                for (int iconNumber : padGameplayIconNumbers)
                {
                    if(iconNumber == icon)
                    {
                        if(iconNumber == 1 && rect.Left > 100) { continue; }
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
                Microsoft::Xna::Framework::Rectangle srcRectangle = GetSrcRectangle(bitmap.value(), bitmapGridX, bitmapGridY, iconWidth, iconHeight, gap, icon);
                Microsoft::Xna::Framework::Rectangle rectangle = GetDstRectangle(rect, iconWidth, iconHeight, useHotSpot);
                float rotationRad = 0.0f;
                if (rotationDeg != 0.0)
                {
                    rotationRad = (float)Misc::DegToRad(rotationDeg);
                    rectangle = Misc::RotateAdjust(rectangle, rotationRad);
                }
                spriteBatch->Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront, Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
                spriteBatch->Draw(bitmap, rectangle, srcRectangle, Microsoft::Xna::Framework::Color::FromNonPremultiplied(255, 255, 255, (int)(255.0 * opacity)), rotationRad, origin, effect, 0.0f);
                spriteBatch->End();
            }
        }

        Microsoft::Xna::Framework::Rectangle Pixmap::GetSrcRectangle(Texture2D& bitmap, int bitmapGridX, int bitmapGridY, int iconWidth, int iconHeight, int gap, int icon)
        {
            int width = bitmap.getBoundsProperty().Width;
            int height = bitmap.getBoundsProperty().Height;
            int num = icon % (width / bitmapGridX);
            int num2 = icon / (width / bitmapGridX);
            bitmapGridX += gap;
            bitmapGridY += gap;
            return Microsoft::Xna::Framework::Rectangle(gap + num * bitmapGridX, gap + num2 * bitmapGridY, iconWidth, iconHeight);
        }

        Microsoft::Xna::Framework::Rectangle Pixmap::GetDstRectangle(TinyRect& rect, int iconWidth, int iconHeight, bool useHotSpot)
        {
            int finalWidth = ((rect.getWidthProperty() == 0) ? iconWidth : rect.getWidthProperty());
            int finalHeight = ((rect.getHeightProperty() == 0) ? iconHeight : rect.getHeightProperty());
            int scaledLeftX = (int)((double)rect.Left * zoom);
            int scaledTopY = (int)((double)rect.Top * zoom);
            int scaledRightX = (int)((double)scaledLeftX + (double)finalWidth * zoom);
            int scaledBottomY = (int)((double)scaledTopY + (double)finalHeight * zoom);
            if (useHotSpot && hotSpotZoom > 1.0)
            {
                scaledLeftX -= (int)hotSpotX;
                scaledTopY -= (int)hotSpotY;
                scaledRightX -= (int)hotSpotX;
                scaledBottomY -= (int)hotSpotY;
                scaledLeftX = (int)((double)scaledLeftX * hotSpotZoom);
                scaledTopY = (int)((double)scaledTopY * hotSpotZoom);
                scaledRightX = (int)((double)scaledRightX * hotSpotZoom);
                scaledBottomY = (int)((double)scaledBottomY * hotSpotZoom);
                scaledLeftX += (int)hotSpotX;
                scaledTopY += (int)hotSpotY;
                scaledRightX += (int)hotSpotX;
                scaledBottomY += (int)hotSpotY;
            }
            return Microsoft::Xna::Framework::Rectangle(scaledLeftX, scaledTopY, scaledRightX - scaledLeftX, scaledBottomY - scaledTopY);
        }

         std::optional<Texture2D> Pixmap::GetBitmap(int channel)
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

}

