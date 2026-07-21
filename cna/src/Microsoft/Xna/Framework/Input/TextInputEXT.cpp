// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Input/TextInputEXT.hpp"

#include "CNA/Input/TextInputType.hpp"

#include <SDL2/SDL.h>

namespace
{
    // WindowHandle stores an SDL_Window* as an integer (FNA models it as IntPtr).
    inline SDL_Window* ToSdlWindow(std::uintptr_t handle)
    {
        return reinterpret_cast<SDL_Window*>(handle);
    }
}

namespace Microsoft::Xna::Framework::Input
{
    System::MulticastAction<charcs>                       TextInputEXT::TextInput;
    System::MulticastAction<const std::string&, int, int> TextInputEXT::TextEditing;
    System::MulticastAction<const std::vector<std::string>&, int, bool> TextInputEXT::TextEditingCandidatesEXT;
    std::uintptr_t                                  TextInputEXT::windowHandle_ = 0;

    std::uintptr_t TextInputEXT::getWindowHandleProperty()
    {
        return windowHandle_;
    }

    void TextInputEXT::setWindowHandleProperty(std::uintptr_t value)
    {
        windowHandle_ = value;
    }

    bool TextInputEXT::IsTextInputActive()
    {
        // SDL2's text-input API is process-global, not per-window (unlike SDL3's
        // window-parameterized SDL_TextInputActive) -- the window handle is only still checked
        // here to preserve this method's existing "no window yet -> false" guard.
        if (ToSdlWindow(windowHandle_) != nullptr)
        {
            return SDL_IsTextInputActive() != SDL_FALSE;
        }
        return false;
    }

    bool TextInputEXT::IsScreenKeyboardShown()
    {
        return IsScreenKeyboardShown(windowHandle_);
    }

    bool TextInputEXT::IsScreenKeyboardShown(std::uintptr_t window)
    {
        if (SDL_Window* w = ToSdlWindow(window))
        {
            return SDL_IsScreenKeyboardShown(w) != SDL_FALSE;
        }
        return false;
    }

    void TextInputEXT::StartTextInput()
    {
        // Guard against a null window: WindowHandle is not populated until the window
        // is created (plan_input.md Task 703). SDL2's SDL_StartTextInput takes no window
        // argument (process-global, unlike SDL3's per-window version) -- the guard is kept so
        // this call is a no-op before a window exists, matching the pre-migration behavior.
        if (ToSdlWindow(windowHandle_) != nullptr)
        {
            SDL_StartTextInput();
        }
    }

    void TextInputEXT::StopTextInput()
    {
        if (ToSdlWindow(windowHandle_) != nullptr)
        {
            SDL_StopTextInput();
        }
    }

    void TextInputEXT::StartTextInputWithTypeEXT(CNA::Input::TextInputTypeEXT /*type*/)
    {
        // SDL2 has no equivalent of SDL3's per-input IME type-hint properties system
        // (SDL_StartTextInputWithProperties/SDL_PROP_TEXTINPUT_TYPE_NUMBER) -- falls back to a
        // plain, type-hint-less StartTextInput(). A disclosed behavioral simplification: on-screen
        // keyboards that adapt their layout to the requested type (e.g. a numeric-only IME for
        // TextInputTypeEXT::Number) will show their default text layout instead. Not exercised by
        // mobile-eggbert itself (no XNA/NOXNA text-entry UI calls this) -- see plan_lite.md's
        // Phase 4 status.
        if (ToSdlWindow(windowHandle_) != nullptr)
        {
            SDL_StartTextInput();
        }
    }

    void TextInputEXT::SetInputRectangle(const Microsoft::Xna::Framework::Rectangle& rectangle)
    {
        if (ToSdlWindow(windowHandle_) != nullptr)
        {
            SDL_Rect rect;
            rect.x = rectangle.X;
            rect.y = rectangle.Y;
            rect.w = rectangle.Width;
            rect.h = rectangle.Height;
            // SDL2's SDL_SetTextInputRect is process-global (no window argument, no IME cursor
            // x-offset parameter) -- unlike SDL3's window-scoped, cursor-offset-aware
            // SDL_SetTextInputArea.
            SDL_SetTextInputRect(&rect);
        }
    }

    void TextInputEXT::INTERNAL_OnTextInput(charcs c)
    {
        if (TextInput)
            TextInput(c);
    }

    void TextInputEXT::INTERNAL_OnTextEditing(const std::string& text, int start, int length)
    {
        if (TextEditing)
            TextEditing(text, start, length);
    }

    void TextInputEXT::INTERNAL_OnTextEditingCandidates(
        const std::vector<std::string>& candidates, int selected, bool horizontal)
    {
        if (TextEditingCandidatesEXT)
            TextEditingCandidatesEXT(candidates, selected, horizontal);
    }

}
