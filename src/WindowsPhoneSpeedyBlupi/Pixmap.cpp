//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Pixmap.h"

#include "Microsoft/Xna/Framework/Rectangle.h"
#include "WindowsPhoneSpeedyBlupi/Def.h"

namespace WindowsPhoneSpeedyBlupi {
    Pixmap::Pixmap(Game1& game1, Microsoft::Xna::Framework::Graphics::GraphicsDeviceManager& graphics):
game1(game1),
graphics(graphics),
DrawBounds( [&]() {
        TinyRect result;
        double screenWidth = graphics.GraphicsDevice.get().Viewport.get().Width;
        double screenHeight = graphics.GraphicsDevice.get().Viewport.get().Height;
        if(Def::PLATFORM == Def::Platform::Android && screenHeight > 480) {
            screenWidth = screenHeight * (640.0f / 480.0f);
        }
        if (screenWidth != 0.0 && screenHeight != 0.0)
        {
            double num3;
            double num4;
            if (screenWidth / screenHeight < 1.3333333333333333)
            {
                num3 = 640.0;
                num4 = 640.0 * (screenHeight / screenWidth);
            }
            else
            {
                num3 = 480.0 * (screenWidth / screenHeight);
                num4 = 480.0;
            }
            result.LeftX = 0;
            result.RightX = (int)num3;
            result.TopY = 0;
            result.BottomY = (int)num4;
        }
        return result;
    }),
Origin( [&]()   {
  TinyPoint result;
  result.X = (int)originX;
  result.Y = (int)originY;
  return result;
})
    {
        effect = Microsoft::Xna::Framework::Graphics::SpriteEffects::None;
    }

    TinyPoint Pixmap::HotSpotToHud(TinyPoint& pos)
    {
        TinyPoint result;
        result.X = (int)((double)(pos.X - (int)hotSpotX) / hotSpotZoom) + (int)hotSpotX - (int)originX;
        result.Y = (int)((double)(pos.Y - (int)hotSpotY) / hotSpotZoom) + (int)hotSpotY - (int)originY;
        return result;
    }

            void Pixmap::DrawInputButton(TinyRect& rect, Def::ButtonGlyph& glyph, bool& pressed, bool& selected)
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
                        tinyPoint.X = rect.LeftX + rect.Width / 2 - (int)originX;
                        tinyPoint.Y = rect.TopY + 28;
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





}

