// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Input/MouseCursor.hpp"

#include <SDL3/SDL.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace Microsoft::Xna::Framework::Input
{
    MouseCursor MouseCursor::MakeSystem(int systemCursorId)
    {
        SDL_Cursor* c = SDL_CreateSystemCursor(static_cast<SDL_SystemCursor>(systemCursorId));
        MouseCursor cursor(c, /*owning=*/true);
        cursor.isSystemSingleton_ = true; // process-lifetime shared cursor — never disposed
        return cursor;
    }

    // Stock cursors are lazily constructed function-local statics (Meyer's singleton),
    // matching MonoGame's `static MouseCursor() { PlatformInitalize(); }` lazy static
    // constructor. Building them as plain static member initializers instead would run
    // SDL_CreateSystemCursor at static-init time, before SDL_Init() has necessarily run.
    //
    // Stock-cursor mapping audit (task 833). Each MonoGame MouseCursor maps to the closest SDL3
    // system cursor. MonoGame targets SDL2's SDL_SYSTEM_CURSOR_* names; SDL3 renamed the enum, so
    // CNA uses SDL3's equivalents:
    //   Arrow     -> DEFAULT       (SDL2 ARROW)         IBeam  -> TEXT       (SDL2 IBEAM)
    //   Wait      -> WAIT          (SDL2 WAIT)          Cross  -> CROSSHAIR  (SDL2 CROSSHAIR)
    //   SizeNWSE  -> NWSE_RESIZE   (SDL2 SIZENWSE)      SizeNESW -> NESW_RESIZE (SDL2 SIZENESW)
    //   SizeWE    -> EW_RESIZE     (SDL2 SIZEWE)        SizeNS -> NS_RESIZE  (SDL2 SIZENS)
    //   SizeAll   -> MOVE          (SDL2 SIZEALL)       No     -> NOT_ALLOWED(SDL2 NO)
    //   Hand      -> POINTER       (SDL2 HAND)
    // Only one is not a pure rename: MonoGame's WaitArrow used SDL2's SDL_SYSTEM_CURSOR_WAITARROW,
    // which SDL3 removed. WaitArrow maps to SDL3's SDL_SYSTEM_CURSOR_PROGRESS ("WAIT with an
    // arrow") — the closest available match, and the exact meaning MonoGame's WaitArrow conveyed.
    // The concrete glyph for every system cursor is chosen by the OS/desktop theme, so exact
    // pixels differ per platform regardless of the enum used — an unavoidable SDL/OS difference.
    MouseCursor& MouseCursor::getArrowProperty()
    {
        static MouseCursor instance = MakeSystem(SDL_SYSTEM_CURSOR_DEFAULT);
        return instance;
    }

    MouseCursor::MouseCursor()
        : sdlCursor_(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT))
        , owning_(true)
    {
    }

    MouseCursor::MouseCursor(SDL_Cursor* sdlCursor, bool owning)
        : sdlCursor_(sdlCursor)
        , owning_(owning)
    {
    }

    MouseCursor::MouseCursor(MouseCursor&& other) noexcept
        : sdlCursor_(other.sdlCursor_)
        , owning_(other.owning_)
        , isDisposed_(other.isDisposed_)
        , isSystemSingleton_(other.isSystemSingleton_)
    {
        other.sdlCursor_  = nullptr;
        other.owning_     = false;
        other.isDisposed_ = true;
    }

    MouseCursor& MouseCursor::operator=(MouseCursor&& other) noexcept
    {
        if (this != &other)
        {
            Dispose();
            sdlCursor_         = other.sdlCursor_;
            owning_            = other.owning_;
            isDisposed_        = other.isDisposed_;
            isSystemSingleton_ = other.isSystemSingleton_;
            other.sdlCursor_  = nullptr;
            other.owning_     = false;
            other.isDisposed_ = true;
        }
        return *this;
    }

    MouseCursor::~MouseCursor()
    {
        Dispose();
    }

    void MouseCursor::Dispose()
    {
        if (isDisposed_)
        {
            return;
        }
        // Stock system-cursor singletons are shared for the process lifetime: disposing one must
        // not free the SDL cursor (that would corrupt it for every other holder and risk freeing
        // after SDL_Quit at static teardown). Leave it fully intact and usable.
        if (isSystemSingleton_)
        {
            return;
        }
        if (owning_ && sdlCursor_ != nullptr)
        {
            SDL_DestroyCursor(sdlCursor_);
        }
        sdlCursor_  = nullptr;
        isDisposed_ = true;
    }
}
