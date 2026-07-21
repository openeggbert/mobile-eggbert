// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Input/MouseCursor.hpp"

#include <SDL2/SDL.h>

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
    // Post-Phase-4 (SDL2 migration): only the Arrow stock cursor (SDL_SYSTEM_CURSOR_ARROW) is
    // reachable after Phase 2.6's method-pruning series -- the other MonoGame stock-cursor names
    // (IBeam/Wait/Cross/SizeNWSE/.../WaitArrow) this file used to map to SDL3's renamed enum
    // values were already trimmed as unused before this migration, so no SDL3->SDL2 cursor-enum
    // remapping is needed beyond this one name.
    MouseCursor& MouseCursor::getArrowProperty()
    {
        static MouseCursor instance = MakeSystem(SDL_SYSTEM_CURSOR_ARROW);
        return instance;
    }

    MouseCursor::MouseCursor()
        : sdlCursor_(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW))
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
            SDL_FreeCursor(sdlCursor_);
        }
        sdlCursor_  = nullptr;
        isDisposed_ = true;
    }
}
