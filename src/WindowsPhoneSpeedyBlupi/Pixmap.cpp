/**
 * @file Pixmap.cpp
 * @brief Implements the Pixmap class: viewport geometry, texture loading, and all
 *        SpriteBatch draw calls for the Speedy Blupi rendering back-end.
 *
 * @details
 * ### Zoom / origin transform
 * All draw methods operate in logical 640x480 game-space.  Before any sprite is
 * submitted to SpriteBatch, game-space coordinates are converted to physical
 * screen pixels through two transforms:
 *
 * 1. **Viewport zoom** (`zoom`, computed once per geometry change):
 *    @code
 *      zoom    = min(screenWidth / 640.0, screenHeight / 480.0)
 *      originX = (screenWidth  - 640.0 * zoom) / 2.0
 *      originY = (screenHeight - 480.0 * zoom) / 2.0
 *    @endcode
 *    This letterboxes / pillarboxes the 4:3 canvas inside any viewport.
 *
 * 2. **Hotspot zoom** (`hotSpotZoom`, set per-frame via SetHotSpot()):
 *    Applied only when `useHotSpot = true`.  Scales sprite destination corners
 *    around the hotspot pivot to implement the in-game camera zoom:
 *    @code
 *      scaledX = (x - hotSpotX) * hotSpotZoom + hotSpotX
 *      scaledY = (y - hotSpotY) * hotSpotZoom + hotSpotY
 *    @endcode
 *
 * ### Resolution scaling
 * Textures may be loaded at 1x, 2x, or 4x resolution (controlled by
 * Config::RESOLUTION_SCALE).  Source rectangles passed to SpriteBatch are
 * multiplied by RESOLUTION_SCALE via ScaleSourceRect() / Config::ScaleAsset() so
 * they address the correct pixels in the higher-resolution atlas.  Destination
 * rectangles always remain in 1x logical coordinates.
 *
 * ### Batching invariant
 * When `CNA_SPRITE_BATCHING_ENABLED` is defined (hard-coded `#define` near the
 * top of this file), every draw method checks `batch_started_`:
 * - If **true**: the caller opened a batch via BeginBatch(); the method skips its
 *   own SpriteBatch::Begin/End and contributes to the outer batch.
 * - If **false**: the method opens and closes its own SpriteBatch::Begin/End pair
 *   (original one-draw-call-per-sprite behaviour).
 *
 * @note Created by robertvokac on 5/24/25.
 */

#include "WindowsPhoneSpeedyBlupi/Pixmap.hpp"

#include "CNA/Logger.hpp"
#include "CNA/TargetPlatform.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "WindowsPhoneSpeedyBlupi/Decor.hpp"
#include "WindowsPhoneSpeedyBlupi/Def.hpp"

#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "System/Math.hpp"
#include "WindowsPhoneSpeedyBlupi/Config.hpp"
#include "WindowsPhoneSpeedyBlupi/Misc.hpp"
#include "WindowsPhoneSpeedyBlupi/Tables.hpp"
#include "WindowsPhoneSpeedyBlupi/Text.hpp"
#define CNA_SPRITE_BATCHING_ENABLED

namespace WindowsPhoneSpeedyBlupi
{
    // Scales a source rectangle from 1x logical coordinates to loaded-texture pixels.
    // Destination rectangles and game coordinates are left unchanged.
    static TinyRect ScaleSourceRect(TinyRect rect)
    {
        return TinyRect(
            rect.Left   * Config::RESOLUTION_SCALE,
            rect.Right  * Config::RESOLUTION_SCALE,
            rect.Top    * Config::RESOLUTION_SCALE,
            rect.Bottom * Config::RESOLUTION_SCALE
        );
    }
    TinyRect Pixmap::getDrawBoundsProperty()
    {
        TinyRect result{};
        double screenWidth = graphics.getGraphicsDeviceProperty()->getViewportProperty().getWidthProperty();
        double screenHeight = graphics.getGraphicsDeviceProperty()->getViewportProperty().getHeightProperty();
        if (CNA::getCurrentPlatform() == CNA::TargetPlatform::Android && screenHeight > 480)
        {
            screenWidth = screenHeight * (640.0 / 480.0);
        }
        if (screenWidth != 0.0 && screenHeight != 0.0)
        {
            double drawWidth;
            double drawHeight;
            if (screenWidth / screenHeight < 1.3333333333333333)
            {
                drawWidth = 640.0;
                drawHeight = 640.0 * (screenHeight / screenWidth);
            }
            else
            {
                drawWidth = 480.0 * (screenWidth / screenHeight);
                drawHeight = 480.0;
            }
            result.Left = 0;
            result.Right = static_cast<intcs>(drawWidth);
            result.Top = 0;
            result.Bottom = static_cast<intcs>(drawHeight);
        }
        return result;
    }

    TinyPoint Pixmap::getOriginProperty() const
    {
        return {static_cast<intcs>(originX), static_cast<intcs>(originY)};
    }

    Pixmap::Pixmap(IGame1* game1, Microsoft::Xna::Framework::GraphicsDeviceManager& graphics) :
        game1(game1),
        graphics(graphics), spriteBatch(nullptr), bitmapText(Texture2D()), bitmapButton(Texture2D()),
        bitmapJauge(Texture2D()), bitmapBlupi(Texture2D()), bitmapBlupi1(Texture2D()),
        bitmapObject(Texture2D()),
        bitmapElement(Texture2D()),
        bitmapExplo(Texture2D()), bitmapPad(Texture2D()),
        bitmapSpeedyBlupi(Texture2D()),
        bitmapBlupiYoupie(Texture2D()),
        bitmapGear(Texture2D()),
        bitmapBackground(Texture2D()),
        origin(Microsoft::Xna::Framework::Vector2(0.0f, 0.0f))
    {
        effect = Microsoft::Xna::Framework::Graphics::SpriteEffects::None;
    }

    TinyPoint Pixmap::HotSpotToHud(TinyPoint pos)
    {
        if (hotSpotZoom == 0.0)
        {
            return pos;
        }
        return {
            static_cast<intcs>(static_cast<double>(pos.X - static_cast<intcs>(hotSpotX)) / hotSpotZoom) + static_cast<
                intcs>(hotSpotX) - static_cast<intcs>(originX),
            static_cast<intcs>(static_cast<double>(pos.Y - static_cast<intcs>(hotSpotY)) / hotSpotZoom) + static_cast<
                intcs>(hotSpotY) - static_cast<intcs>(originY)
        };
    }

    void Pixmap::SetHotSpot(const double zoom, const double x, const double y)
    {
        hotSpotZoom = zoom;
        hotSpotX = x;
        hotSpotY = y;
    }

    void Pixmap::DrawInputButton(TinyRect rect, Def::ButtonGlyph glyph, bool pressed, bool selected)
    {
        switch (glyph)
        {
        case Def::ButtonGlyph::InitGamerA:
            DrawIcon(PixmapChannel::Pad, selected ? 16 : 4, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::InitGamerB:
            DrawIcon(PixmapChannel::Pad, selected ? 17 : 5, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::InitGamerC:
            DrawIcon(PixmapChannel::Pad, selected ? 18 : 6, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::InitSetup:
        case Def::ButtonGlyph::PauseSetup:
            DrawIcon(PixmapChannel::Pad, 19, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::InitPlay:
            DrawIcon(PixmapChannel::Pad, 7, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::PauseMenu:
        case Def::ButtonGlyph::ResumeMenu:
            DrawIcon(PixmapChannel::Pad, 11, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::PauseBack:
            DrawIcon(PixmapChannel::Pad, 8, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::PauseRestart:
            DrawIcon(PixmapChannel::Pad, 9, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::PauseContinue:
        case Def::ButtonGlyph::ResumeContinue:
            DrawIcon(PixmapChannel::Pad, 10, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::WinLostReturn:
            DrawIcon(PixmapChannel::Pad, 3, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::InitBuy:
        case Def::ButtonGlyph::TrialBuy:
            DrawIcon(PixmapChannel::Pad, 22, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::InitRanking:
            DrawIcon(PixmapChannel::Pad, 12, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::TrialCancel:
        case Def::ButtonGlyph::RankingContinue:
            DrawIcon(PixmapChannel::Pad, 8, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::SetupSounds:
        case Def::ButtonGlyph::SetupJump:
        case Def::ButtonGlyph::SetupZoom:
        case Def::ButtonGlyph::SetupAccel:
            DrawIcon(PixmapChannel::Pad, selected ? 13 : 21, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::SetupReset:
            DrawIcon(PixmapChannel::Pad, 20, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::SetupReturn:
            DrawIcon(PixmapChannel::Pad, 8, rect, pressed ? 0.8 : 1.0, false);
            break;
        case Def::ButtonGlyph::PlayJump:
            DrawIcon(PixmapChannel::Pad, 2, rect, pressed ? 0.6 : 1.0, false);
            break;
        case Def::ButtonGlyph::PlayAction:
            DrawIcon(PixmapChannel::Pad, 12, rect, pressed ? 0.6 : 1.0, false);
            break;
        case Def::ButtonGlyph::PlayDown:
            DrawIcon(PixmapChannel::Pad, 23, rect, pressed ? 0.6 : 1.0, false);
            break;
        case Def::ButtonGlyph::PlayPause:
            DrawIcon(PixmapChannel::Pad, 3, rect, pressed ? 0.6 : 1.0, false);
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
                DrawIcon(PixmapChannel::Pad, 0, rect, pressed ? 0.6 : 1.0, false);
                TinyPoint tinyPoint{
                    rect.Left + rect.getWidthProperty() / 2 - static_cast<intcs>(originX),
                    rect.Top + 28
                };
                Text::DrawTextCenter(*this, tinyPoint, Decor::GetCheatTinyText(glyph), 1.0);
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
        CNA::Logger::Info("SpeedyBlupi: Pixmap::LoadContent entered");
        CNA::Logger::Info("SpeedyBlupi: asset root = " + game1->getContentProperty().getRootDirectoryProperty());
        spriteBatch = std::make_unique<Microsoft::Xna::Framework::Graphics::SpriteBatch>(
            game1->getGraphicsDeviceProperty());

        // Select asset sub-folders based on the configured resolution scale.
        // Icons:
        //   1x -> "icons/"    (original assets)
        //   2x -> "icons2x/"  (TODO: 2x assets not yet available)
        //   4x -> "icons4x/"  (4x upscaled assets)
        // Backgrounds:
        //   1x -> "backgrounds/"
        //   2x -> "backgrounds2x/"  (TODO: 2x assets not yet available)
        //   4x -> "backgrounds4x/"
        std::string iconPrefix;
        std::string backgroundPrefix;
        switch (Config::RESOLUTION_SCALE)
        {
        case 4:
            iconPrefix       = "icons4x/";
            backgroundPrefix = "backgrounds4x/";
            break;
        case 2:
            // TODO: 2x assets are not yet available. Add icons2x/ and backgrounds2x/ when ready.
            iconPrefix       = "icons2x/";
            backgroundPrefix = "backgrounds2x/";
            break;
        default:
            iconPrefix       = "icons/";
            backgroundPrefix = "backgrounds/";
            break;
        }
        CNA::Logger::Info("SpeedyBlupi: loading icons (prefix=" + iconPrefix + ")");
        CNA::Logger::Info("SpeedyBlupi: loading backgrounds (prefix=" + backgroundPrefix + ")");

        bitmapText    = game1->getContentProperty().Load<Texture2D>(iconPrefix + "text");
        bitmapButton  = game1->getContentProperty().Load<Texture2D>(iconPrefix + "button");
        bitmapJauge   = game1->getContentProperty().Load<Texture2D>(iconPrefix + "jauge");
        bitmapBlupi   = game1->getContentProperty().Load<Texture2D>(iconPrefix + "blupi");
        bitmapBlupi1  = game1->getContentProperty().Load<Texture2D>(iconPrefix + "blupi1");
        bitmapObject  = game1->getContentProperty().Load<Texture2D>(iconPrefix + "object-m");
        bitmapElement = game1->getContentProperty().Load<Texture2D>(iconPrefix + "element");
        bitmapExplo   = game1->getContentProperty().Load<Texture2D>(iconPrefix + "explo");
        bitmapPad     = game1->getContentProperty().Load<Texture2D>(iconPrefix + "pad");

        bitmapSpeedyBlupi = game1->getContentProperty().Load<Texture2D>(backgroundPrefix + "speedyblupi");
        bitmapBlupiYoupie = game1->getContentProperty().Load<Texture2D>(backgroundPrefix + "blupiyoupie");
        bitmapGear        = game1->getContentProperty().Load<Texture2D>(backgroundPrefix + "gear");

        CNA::Logger::Info("SpeedyBlupi: Pixmap::LoadContent done");
        UpdateGeometry();
    }

    void Pixmap::UpdateGeometry()
    {
        double screenWidth = graphics.getGraphicsDeviceProperty()->getViewportProperty().getWidthProperty();
        double screenHeight = graphics.getGraphicsDeviceProperty()->getViewportProperty().getHeightProperty();
        if (CNA::getCurrentPlatform() == CNA::TargetPlatform::Android && screenHeight > 480)
        {
            screenWidth = screenHeight * (640.0 / 480.0);
        }
        double widthScale = screenWidth / 640.0;
        double heightScale = screenHeight / 480.0;
        zoom = System::Math::Min(widthScale, heightScale);
        originX = (screenWidth - 640.0 * zoom) / 2.0;
        originY = (screenHeight - 480.0 * zoom) / 2.0;
    }

    void Pixmap::BackgroundCache(const string& name)
    {
        std::string backgroundPrefix;
        switch (Config::RESOLUTION_SCALE)
        {
        case 4:  backgroundPrefix = "backgrounds4x/"; break;
        case 2:  backgroundPrefix = "backgrounds2x/"; break;
        default: backgroundPrefix = "backgrounds/";   break;
        }
        CNA::Logger::Info("SpeedyBlupi: BackgroundCache loading " + backgroundPrefix + name);
        bitmapBackground = game1->getContentProperty().Load<Texture2D>(backgroundPrefix + name);
        CNA::Logger::Info("SpeedyBlupi: BackgroundCache done " + backgroundPrefix + name);
    }

    bool Pixmap::Start()
    {
        graphics.getGraphicsDeviceProperty()->Clear(Microsoft::Xna::Framework::Color::CornflowerBlue);
        return true;
    }

    bool Pixmap::Finish()
    {
        return true;
    }

    void Pixmap::BeginBatch()
    {
        // UpdateGeometry() was previously only ever called once, from LoadContent() at startup --
        // its own doc comment already said "re-call whenever the window is resized", but nothing
        // did. zoom/originX/originY then stayed frozen at the startup viewport size forever, so
        // any later resize (most visibly: toggling fullscreen) left every sprite drawn at the
        // stale scale/offset while DrawBackground()'s own full-screen stretch (which re-queries
        // the live viewport every call already) correctly filled the new size -- the game content
        // pinned to a small stale corner against an otherwise-blank background. BeginBatch(), not
        // Start(), is the real once-per-frame hook: Game1::Draw() calls pixmap->BeginBatch() every
        // frame unconditionally, but nothing anywhere ever calls IPixmap::Start() despite its own
        // doc comment claiming it's part of the per-frame contract. Placed outside the
        // CNA_SPRITE_BATCHING_ENABLED guard below so it runs regardless of that flag.
        UpdateGeometry();
#ifdef CNA_SPRITE_BATCHING_ENABLED
        if (!spriteBatch || batch_started_) return;
        spriteBatch->Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront,
                           Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
        batch_started_ = true;
#endif
    }

    void Pixmap::EndBatch()
    {
#ifdef CNA_SPRITE_BATCHING_ENABLED
        if (!spriteBatch || !batch_started_) return;
        spriteBatch->End();
        batch_started_ = false;
#endif
    }

    void Pixmap::DrawBackground()
    {
        if (!spriteBatch)
        {
            CNA::Logger::Error("Pixmap::DrawBackground called before LoadContent()");
            return;
        }
        double screenWidth = graphics.getGraphicsDeviceProperty()->getViewportProperty().getWidthProperty();
        double screenHeight = graphics.getGraphicsDeviceProperty()->getViewportProperty().getHeightProperty();
        if (CNA::getCurrentPlatform() == CNA::TargetPlatform::Android && screenHeight > 480)
        {
            screenWidth = screenHeight * (640.0 / 480.0);
        }
        const Texture2D* bitmap_ptr = GetBitmap(PixmapChannel::Background);
        if (bitmap_ptr == nullptr)
        {
            CNA::Logger::Error("GetBitmap returned nullptr for channel=3");
            return;
        }
        const Texture2D& bitmap = *bitmap_ptr;
        Microsoft::Xna::Framework::Rectangle srcRectangle = GetSrcRectangle(bitmap, 10, 10, 10, 10, 0, 0);
        Microsoft::Xna::Framework::Rectangle destinationRectangle = Microsoft::Xna::Framework::Rectangle(
            0, 0, static_cast<intcs>(screenWidth), static_cast<intcs>(screenHeight));
        if (!batch_started_) spriteBatch->Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront,
                           Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
        spriteBatch->Draw(bitmap, destinationRectangle, srcRectangle, Microsoft::Xna::Framework::Color::White);
        if (!batch_started_) spriteBatch->End();
        TinyPoint dest{
            static_cast<intcs>(originX),
            static_cast<intcs>(originY)
        };
        TinyRect rect{};
        rect.Left = 0;
        rect.Top = 0;
        rect.Right = 640;
        rect.Bottom = 480;
        DrawPart(PixmapChannel::Background, dest, rect);
    }

    void Pixmap::DrawChar(intcs rank, TinyPoint pos, double size)
    {
        pos.X = static_cast<intcs>(static_cast<double>(pos.X) + originX);
        pos.Y = static_cast<intcs>(static_cast<double>(pos.Y) + originY);
        TinyRect tinyRect{};
        tinyRect.Left = pos.X;
        tinyRect.Top = pos.Y;
        tinyRect.Right = pos.X + static_cast<intcs>(32.0 * size);
        tinyRect.Bottom = pos.Y + static_cast<intcs>(32.0 * size);
        DrawIcon(PixmapChannel::Text, rank, tinyRect, 1.0, false);
    }

    void Pixmap::HudIcon(PixmapChannel channel, intcs rank, TinyPoint pos)
    {
        pos.X = static_cast<intcs>(static_cast<double>(pos.X) + originX);
        pos.Y = static_cast<intcs>(static_cast<double>(pos.Y) + originY);
        TinyRect tinyRect{pos};
        DrawIcon(channel, rank, tinyRect, 1.0, false);
    }

    void Pixmap::QuickIcon(PixmapChannel channel, intcs rank, TinyPoint pos)
    {
        TinyRect tinyRect{pos};
        DrawIcon(channel, rank, tinyRect, 1.0, true);
    }

    void Pixmap::QuickIcon(PixmapChannel channel, intcs rank, TinyPoint pos, double opacity, double rotation)
    {
        TinyRect tinyRect{pos};
        DrawIcon(channel, rank, tinyRect, opacity, rotation, true);
    }

    bool Pixmap::DrawPart(PixmapChannel channel, TinyPoint dest, TinyRect rect)
    {
        return DrawPart(channel, dest, rect, 1.0);
    }

    bool Pixmap::DrawPart(PixmapChannel channel, TinyPoint dest, TinyRect rect, double zoom)
    {
        if (!spriteBatch)
        {
            CNA::Logger::Error("Pixmap::DrawPart called before LoadContent()");
            return false;
        }
        const Texture2D* bitmap = GetBitmap(channel);
        if (bitmap == nullptr)
        {
            return false;
        }
        if (channel == PixmapChannel::Jauge)
        {
            CNA::Logger::Debug(
                "Pixmap::DrawPart ch5 BEFORE origin: dest=(" + std::to_string(dest.X) + "," + std::to_string(dest.Y) +
                "), srcRect=(" + std::to_string(rect.Left) + "," + std::to_string(rect.Top) + "," +
                std::to_string(rect.Right) + "," + std::to_string(rect.Bottom) + "), zoom=" + std::to_string(zoom));

            dest.X = static_cast<intcs>(static_cast<double>(dest.X) + originX);
            dest.Y = static_cast<intcs>(static_cast<double>(dest.Y) + originY);
            CNA::Logger::Debug(
                "Pixmap::DrawPart ch5 AFTER origin: dest=(" + std::to_string(dest.X) + "," + std::to_string(dest.Y) +
                "), origin=(" + std::to_string(originX) + "," + std::to_string(originY) + ")");
        }
        // Scale the source rectangle from 1x logical coordinates to the actual
        // texture pixels of the loaded (possibly 2x or 4x) sprite sheet.
        TinyRect scaledRect = ScaleSourceRect(rect);
        using Microsoft::Xna::Framework::Rectangle;
        Rectangle value = Rectangle(
            scaledRect.Left, scaledRect.Top, scaledRect.getWidthProperty(), scaledRect.getHeightProperty());
        // Destination rectangle stays in logical (1x) coordinates so that on-screen
        // sizes remain identical regardless of asset resolution.
        Rectangle destinationRectangle = Rectangle(
            dest.X,
            dest.Y,
            static_cast<intcs>(static_cast<double>(rect.getWidthProperty()) * zoom),
            static_cast<intcs>(static_cast<double>(rect.getHeightProperty()) * zoom)
        );
        if (!batch_started_) spriteBatch->Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront,
                           Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
        spriteBatch->Draw(*bitmap, destinationRectangle, value, Microsoft::Xna::Framework::Color::White);
        if (!batch_started_) spriteBatch->End();
        return true;
    }

    void Pixmap::DrawIcon(PixmapChannel channel, intcs icon, TinyRect rect, double opacity, bool useHotSpot)
    {
        if (!spriteBatch)
        {
            CNA::Logger::Error("Pixmap::DrawIcon called before LoadContent()");
            return;
        }
        DrawIcon(channel, icon, rect, opacity, 0.0, useHotSpot);
    }

    void Pixmap::DrawIcon(PixmapChannel channel, intcs icon, TinyRect rect, double opacity, double rotationDeg, bool useHotSpot)
    {
        if (!spriteBatch)
        {
            CNA::Logger::Error("Pixmap::DrawIcon called before LoadContent()");
            return;
        }
        if (icon == -1)
        {
            return;
        }
        using Microsoft::Xna::Framework::Input::Touch::TouchPanel;
        if (Config::TOUCH_BUTTONS_SHOWN_ONLY_IF_TOUCHSCREEN_IS_AVAILABLE && CNA::getCurrentPlatform() !=
            CNA::TargetPlatform::Android && channel == PixmapChannel::Pad && !TouchPanel::GetCapabilities().getIsConnectedProperty())
        {
            static intcs padGameplayIconNumbers[] = {0, 1, 2, 3, 30, 12, 23};
            for (intcs iconNumber : padGameplayIconNumbers)
            {
                if (iconNumber == icon)
                {
                    if (iconNumber == 1 && rect.Left > 100) { continue; }
                    // Touch display is not connected and the icon is a gameplay icon. Nothing to do.
                    return;
                }
            }
        }
        const Texture2D* bitmap = GetBitmap(channel);
        if (bitmap == nullptr)
        {
            return;
        }
        // Source dimensions: address pixels in the loaded (possibly scaled) texture.
        intcs srcGridX;
        intcs srcGridY;
        intcs srcIconWidth;
        intcs srcIconHeight;
        intcs srcGap;
        // Destination dimensions: logical 1x on-screen sizes (never scaled).
        intcs dstIconWidth;
        intcs dstIconHeight;

        switch (channel)
        {
        case PixmapChannel::Blupi:
        case PixmapChannel::Blupi1_11:
        case PixmapChannel::Blupi1_12:
        case PixmapChannel::Blupi1_13:
            srcGridX     = Config::ScaleAsset(60);
            srcGridY     = Config::ScaleAsset(60);
            srcIconWidth  = Config::ScaleAsset(60);
            srcIconHeight = Config::ScaleAsset(60);
            srcGap       = 0;
            dstIconWidth  = 60;
            dstIconHeight = 60;
            break;
        case PixmapChannel::Object:
            srcGridX     = Config::ScaleAsset(64);
            srcGridY     = Config::ScaleAsset(64);
            srcIconWidth  = Config::ScaleAsset(64);
            srcIconHeight = Config::ScaleAsset(64);
            srcGap       = Config::ScaleAsset(1);
            dstIconWidth  = 64;
            dstIconHeight = 64;
            break;
        case PixmapChannel::Element:
            srcGridX     = Config::ScaleAsset(60);
            srcGridY     = Config::ScaleAsset(60);
            srcIconWidth  = Config::ScaleAsset(60);
            srcIconHeight = Config::ScaleAsset(60);
            srcGap       = 0;
            dstIconWidth  = 60;
            dstIconHeight = 60;
            break;
        case PixmapChannel::Explosion:
            {
                intcs baseIconHeight = Tables::table_explo_size[icon];
                intcs baseIconWidth  = System::Math::Max(baseIconHeight, 128);
                srcGridX     = Config::ScaleAsset(144);
                srcGridY     = Config::ScaleAsset(144);
                srcIconHeight = Config::ScaleAsset(baseIconHeight);
                srcIconWidth  = Config::ScaleAsset(baseIconWidth);
                srcGap       = 0;
                dstIconWidth  = baseIconWidth;
                dstIconHeight = baseIconHeight;
            }
            break;
        case PixmapChannel::Text:
            srcGridX     = Config::ScaleAsset(32);
            srcGridY     = Config::ScaleAsset(32);
            srcIconWidth  = Config::ScaleAsset(32);
            srcIconHeight = Config::ScaleAsset(32);
            srcGap       = 0;
            dstIconWidth  = 32;
            dstIconHeight = 32;
            break;
        case PixmapChannel::Button:
            srcGridX     = Config::ScaleAsset(40);
            srcGridY     = Config::ScaleAsset(40);
            srcIconWidth  = Config::ScaleAsset(40);
            srcIconHeight = Config::ScaleAsset(40);
            srcGap       = 0;
            dstIconWidth  = 40;
            dstIconHeight = 40;
            break;
        case PixmapChannel::Pad:
            srcGridX     = Config::ScaleAsset(140);
            srcGridY     = Config::ScaleAsset(140);
            srcIconWidth  = Config::ScaleAsset(140);
            srcIconHeight = Config::ScaleAsset(140);
            srcGap       = 0;
            dstIconWidth  = 140;
            dstIconHeight = 140;
            break;
        case PixmapChannel::SpeedyBlupiBackground:
            srcGridX     = Config::ScaleAsset(640);
            srcGridY     = Config::ScaleAsset(160);
            srcIconWidth  = Config::ScaleAsset(640);
            srcIconHeight = Config::ScaleAsset(160);
            srcGap       = 0;
            dstIconWidth  = 640;
            dstIconHeight = 160;
            break;
        case PixmapChannel::BlupiYoupieBackground:
            srcGridX     = Config::ScaleAsset(410);
            srcGridY     = Config::ScaleAsset(380);
            srcIconWidth  = Config::ScaleAsset(410);
            srcIconHeight = Config::ScaleAsset(380);
            srcGap       = 0;
            dstIconWidth  = 410;
            dstIconHeight = 380;
            break;
        case PixmapChannel::GearBackground:
            srcGridX     = Config::ScaleAsset(226);
            srcGridY     = Config::ScaleAsset(226);
            srcIconWidth  = Config::ScaleAsset(226);
            srcIconHeight = Config::ScaleAsset(226);
            srcGap       = 0;
            dstIconWidth  = 226;
            dstIconHeight = 226;
            break;
        default:
            srcGridX     = 0;
            srcGridY     = 0;
            srcIconWidth  = 0;
            srcIconHeight = 0;
            srcGap       = 0;
            dstIconWidth  = 0;
            dstIconHeight = 0;
            break;
        }
        if (srcGridX != 0)
        {
            using Microsoft::Xna::Framework::Rectangle;
            Rectangle srcRectangle = GetSrcRectangle(
                *bitmap, srcGridX, srcGridY, srcIconWidth, srcIconHeight, srcGap, icon);
            // GetDstRectangle uses the unscaled logical destination size so that
            // on-screen sprite sizes remain unchanged at any RESOLUTION_SCALE.
            Rectangle rectangle = GetDstRectangle(rect, dstIconWidth, dstIconHeight, useHotSpot);
            float rotationRad = 0.0f;
            if (rotationDeg != 0.0)
            {
                rotationRad = static_cast<float>(Misc::DegToRad(rotationDeg));
                rectangle = Misc::RotateAdjust(rectangle, rotationRad);
            }
            if (!batch_started_) spriteBatch->Begin(Microsoft::Xna::Framework::Graphics::SpriteSortMode::BackToFront,
                               Microsoft::Xna::Framework::Graphics::BlendState::AlphaBlend);
            spriteBatch->Draw(*bitmap, rectangle, srcRectangle,
                              Microsoft::Xna::Framework::Color::FromNonPremultiplied(
                                  255, 255, 255, static_cast<intcs>(255.0 * opacity)), rotationRad, origin, effect,
                              0.0f);
            if (!batch_started_) spriteBatch->End();
        }
    }

    Microsoft::Xna::Framework::Rectangle Pixmap::GetSrcRectangle(const Texture2D& bitmap, intcs bitmapGridX,
                                                                 intcs bitmapGridY,
                                                                 intcs iconWidth, intcs iconHeight, intcs gap,
                                                                 intcs icon)
    {
        intcs width = bitmap.getBoundsProperty().Width;
        // warning: variable height is not used
        intcs height = bitmap.getBoundsProperty().Height;
        intcs column = icon % (width / bitmapGridX);
        intcs row = icon / (width / bitmapGridX);
        bitmapGridX += gap;
        bitmapGridY += gap;
        return Microsoft::Xna::Framework::Rectangle(gap + column * bitmapGridX, gap + row * bitmapGridY, iconWidth,
                                                    iconHeight);
    }

    Microsoft::Xna::Framework::Rectangle Pixmap::GetDstRectangle(TinyRect rect, intcs iconWidth, intcs iconHeight,
                                                                 bool useHotSpot)
    {
        intcs finalWidth = ((rect.getWidthProperty() == 0) ? iconWidth : rect.getWidthProperty());
        intcs finalHeight = ((rect.getHeightProperty() == 0) ? iconHeight : rect.getHeightProperty());
        intcs scaledLeftX = (intcs)((double)rect.Left * zoom);
        intcs scaledTopY = (intcs)((double)rect.Top * zoom);
        intcs scaledRightX = (intcs)((double)scaledLeftX + (double)finalWidth * zoom);
        intcs scaledBottomY = (intcs)((double)scaledTopY + (double)finalHeight * zoom);
        if (useHotSpot && hotSpotZoom > 1.0)
        {
            scaledLeftX -= (intcs)hotSpotX;
            scaledTopY -= (intcs)hotSpotY;
            scaledRightX -= (intcs)hotSpotX;
            scaledBottomY -= (intcs)hotSpotY;
            scaledLeftX = (intcs)((double)scaledLeftX * hotSpotZoom);
            scaledTopY = (intcs)((double)scaledTopY * hotSpotZoom);
            scaledRightX = (intcs)((double)scaledRightX * hotSpotZoom);
            scaledBottomY = (intcs)((double)scaledBottomY * hotSpotZoom);
            scaledLeftX += (intcs)hotSpotX;
            scaledTopY += (intcs)hotSpotY;
            scaledRightX += (intcs)hotSpotX;
            scaledBottomY += (intcs)hotSpotY;
        }
#ifdef MODERN
        else if (useHotSpot && hotSpotZoom > 0.0 && hotSpotZoom < 1.0)
        {
            // Zoom-out: scale coordinates relative to hotspot center.
            scaledLeftX -= (intcs)hotSpotX;
            scaledTopY -= (intcs)hotSpotY;
            scaledRightX -= (intcs)hotSpotX;
            scaledBottomY -= (intcs)hotSpotY;
            scaledLeftX = (intcs)((double)scaledLeftX * hotSpotZoom);
            scaledTopY = (intcs)((double)scaledTopY * hotSpotZoom);
            scaledRightX = (intcs)((double)scaledRightX * hotSpotZoom);
            scaledBottomY = (intcs)((double)scaledBottomY * hotSpotZoom);
            scaledLeftX += (intcs)hotSpotX;
            scaledTopY += (intcs)hotSpotY;
            scaledRightX += (intcs)hotSpotX;
            scaledBottomY += (intcs)hotSpotY;
        }
#endif
        return Microsoft::Xna::Framework::Rectangle(scaledLeftX, scaledTopY, scaledRightX - scaledLeftX,
                                                    scaledBottomY - scaledTopY);
    }

    const Texture2D* Pixmap::GetBitmap(PixmapChannel channel)
    {
        switch (channel)
        {
        case PixmapChannel::Blupi:
            return &bitmapBlupi;
        case PixmapChannel::Blupi1_11:
        case PixmapChannel::Blupi1_12:
        case PixmapChannel::Blupi1_13:
            return &bitmapBlupi1;
        case PixmapChannel::Object:
            return &bitmapObject;
        case PixmapChannel::Element:
            return &bitmapElement;
        case PixmapChannel::Explosion:
            return &bitmapExplo;
        case PixmapChannel::Text:
            return &bitmapText;
        case PixmapChannel::Button:
            return &bitmapButton;
        case PixmapChannel::Jauge:
            return &bitmapJauge;
        case PixmapChannel::Pad:
            return &bitmapPad;
        case PixmapChannel::SpeedyBlupiBackground:
            return &bitmapSpeedyBlupi;
        case PixmapChannel::BlupiYoupieBackground:
            return &bitmapBlupiYoupie;
        case PixmapChannel::GearBackground:
            return &bitmapGear;
        case PixmapChannel::Background:
            return &bitmapBackground;
        default:
            return nullptr;
        }
    }
}
