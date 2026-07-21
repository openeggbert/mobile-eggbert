// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "CNA/Internal/Input/InputManager.hpp"
#include <SDL2/SDL.h>

namespace Microsoft::Xna::Framework::Input
{
    std::uintptr_t           Mouse::windowHandle_           = 0;
    // DEC-06: ClickedEXT is multicast (MulticastAction<int>), matching FNA's Action<int>.
    System::MulticastAction<int> Mouse::ClickedEXT;

    std::uintptr_t Mouse::getWindowHandleProperty()
    {
        return windowHandle_;
    }

    void Mouse::setWindowHandleProperty(const std::uintptr_t value)
    {
        windowHandle_ = value;
    }

    MouseState Mouse::GetState()
    {
        return CNA::Internal::Input::InputManager::GetMouseState();
    }

    void Mouse::SetCursor(MouseCursor& cursor)
    {
        // Guard against a disposed or empty cursor (GetSDLCursor() == nullptr): SDL_SetCursor(NULL)
        // does NOT clear the cursor — it forces a redraw of the *current* cursor — so passing a
        // disposed cursor through would silently keep the old cursor while looking like it changed.
        // No-op instead, matching MonoGame's guard against an invalid cursor (it throws on a null
        // MouseCursor; CNA takes a reference so only the disposed-handle case is reachable here).
        SDL_Cursor* handle = cursor.GetSDLCursor();
        if (handle == nullptr)
        {
            return;
        }
        SDL_SetCursor(handle);
    }

    void Mouse::INTERNAL_onClicked(int button)
    {
        if (ClickedEXT)
            ClickedEXT(button);
    }

}
