// SPDX-License-Identifier: MS-PL
#include "CNA/Internal/Input/SdlInputBridge.hpp"

#include "CNA/Input/InputDevices.hpp"
#include "CNA/Input/Joysticks.hpp"
#include "CNA/Internal/Backends/Common/IGraphicsBackend.hpp"
#include "CNA/Internal/Input/InputManager.hpp"
#include "CNA/Internal/Input/SdlGamepadBackend.hpp"
#include "CNA/Internal/Input/SdlJoystickBackend.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/TextInputEXT.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <array>
#include "System/Nullable.hpp"
#include <unordered_map>

namespace
{
    using CNA::Internal::Input::GamePadAxis;
    using CNA::Internal::Input::GamePadButton;
    using Microsoft::Xna::Framework::PlayerIndex;
    using Microsoft::Xna::Framework::Input::ButtonState;
    using Microsoft::Xna::Framework::Input::Touch::TouchLocationState;
    using Microsoft::Xna::Framework::Input::Keys;
    using SharpRuntime::charcs;

    constexpr std::size_t MaxSupportedGamePads = 4;

    // Mirrors FNA's GamePad.GAMEPAD_COUNT / FNA_GAMEPAD_NUM_GAMEPADS env override
    // (GamePad.cs:34-53). FNA's comment notes that going *above* the default also
    // requires adding more PlayerIndex names; CNA's PlayerIndex is frozen XNA API
    // (only One-Four), so an override above MaxSupportedGamePads is clamped down.
    // The practically useful direction — reducing/disabling gamepad tracking — works.
    // Pure parse of the FNA_GAMEPAD_NUM_GAMEPADS value: nullptr / negative / non-numeric ->
    // MaxSupportedGamePads; a valid non-negative value is clamped to MaxSupportedGamePads (PlayerIndex
    // is the frozen XNA enum One-Four). Extracted so the parsing is directly unit-testable.
    std::size_t parse_gamepad_count(const char* envValue)
    {
        if (envValue != nullptr)
        {
            try
            {
                const long parsed = std::stol(envValue);
                if (parsed >= 0)
                    return std::min(static_cast<std::size_t>(parsed), MaxSupportedGamePads);
            }
            catch (...)
            {
            }
        }
        return MaxSupportedGamePads;
    }

    // Test-only override for effective_gamepad_count(); nullopt means "use the cached env value".
    // The env value is read once (cached), so tests use this to exercise 0/1/4-slot behavior.
    System::Nullable<std::size_t> g_gamepadCountTestOverride;

    std::size_t effective_gamepad_count()
    {
        if (g_gamepadCountTestOverride.getHasValueProperty())
            return g_gamepadCountTestOverride.getValueProperty();
        static const std::size_t count = parse_gamepad_count(std::getenv("FNA_GAMEPAD_NUM_GAMEPADS"));
        return count;
    }

    // Test-only override for use_scancode_mode(): nullopt means "use the cached env value".
    // Because the env value is cached once (below), tests can't toggle FNA_KEYBOARD_USE_SCANCODES
    // in-process; this hook lets a test exercise both modes without a subprocess.
    System::Nullable<bool> g_scancodeModeTestOverride;

    // Mirrors FNA's UseScancodes static readonly bool (SDL3_FNAPlatform.cs:33-35):
    // evaluated once, so setting the env var after the first key event/lookup has no
    // effect, matching FNA's own readonly-at-startup semantics.
    bool use_scancode_mode()
    {
        if (g_scancodeModeTestOverride.getHasValueProperty())
            return g_scancodeModeTestOverride.getValueProperty();
        static const bool useScancodes = []() -> bool {
            const char* envValue = std::getenv("FNA_KEYBOARD_USE_SCANCODES");
            return envValue != nullptr && std::string(envValue) == "1";
        }();
        return useScancodes;
    }

    // --- Text input control-character synthesis ---
    // SDL does not deliver TEXT_INPUT events for these control keys, so FNA synthesizes
    // them on KEY_DOWN. Indices match kTextInputCharacters.
    // (FNAPlatform.cs:261-280, SDL3_FNAPlatform.cs:903-953)
    constexpr char kTextInputCharacters[7] = {
        static_cast<char>(2),   // Home
        static_cast<char>(3),   // End
        static_cast<char>(8),   // Back (Backspace)
        static_cast<char>(9),   // Tab
        static_cast<char>(13),  // Enter
        static_cast<char>(127), // Delete
        static_cast<char>(22)   // Ctrl+V (Paste)
    };

    // True while the matching control character is held (index 6 = Ctrl+V).
    bool g_textInputControlDown[7] = {};
    // Suppresses the literal 'v' TEXT_INPUT that SDL emits alongside a Ctrl+V paste.
    bool g_textInputSuppress = false;

    // Decodes a NUL-terminated UTF-8 string into UTF-16 code units, invoking `emit` for each.
    // This mirrors FNA's TEXT_INPUT handling (SDL3_FNAPlatform.cs:1166-1184), which runs SDL's
    // UTF-8 bytes through Encoding.UTF8.GetChars() and dispatches each resulting C# char (a UTF-16
    // code unit) to TextInputEXT.OnTextInput. A code point above U+FFFF is emitted as a high/low
    // surrogate pair. A self-contained decoder is used here rather than sharp-runtime's Encoding
    // (which is byte/std::string-oriented and has no UTF-16-code-unit output) — this is internal
    // backend plumbing, the same category as the other file-local SDL translation helpers above.
    // Malformed sequences are skipped defensively; SDL always delivers well-formed UTF-8.
    template <typename Emit>
    void decode_utf8_to_utf16(const char* text, Emit&& emit)
    {
        const auto* s = reinterpret_cast<const unsigned char*>(text);
        while (*s != 0)
        {
            const unsigned char b0 = s[0];
            std::uint32_t cp;
            int len;
            std::uint32_t minCp; // smallest code point legally encodable in `len` bytes (overlong guard)
            if (b0 < 0x80)                 { cp = b0;        len = 1; minCp = 0x0; }
            else if ((b0 & 0xE0) == 0xC0)  { cp = b0 & 0x1F; len = 2; minCp = 0x80; }
            else if ((b0 & 0xF0) == 0xE0)  { cp = b0 & 0x0F; len = 3; minCp = 0x800; }
            else if ((b0 & 0xF8) == 0xF0)  { cp = b0 & 0x07; len = 4; minCp = 0x10000; }
            else
            {
                // Invalid lead byte. FNA decodes via Encoding.UTF8, which substitutes U+FFFD for
                // malformed input rather than dropping it (DEC-08) — match that.
                emit(static_cast<charcs>(0xFFFD));
                ++s;
                continue;
            }

            int i = 1;
            for (; i < len; ++i)
            {
                if ((s[i] & 0xC0) != 0x80) break; // truncated/invalid continuation
                cp = (cp << 6) | (s[i] & 0x3F);
            }
            if (i != len)
            {
                // Ill-formed sequence: one U+FFFD for its maximal subpart, then resync at s[i].
                emit(static_cast<charcs>(0xFFFD));
                s += i;
                continue;
            }
            s += len;

            // Reject overlong encodings, UTF-16 surrogate code points, and out-of-range code points;
            // Encoding.UTF8 treats all of these as invalid and substitutes U+FFFD.
            if (cp < minCp || (cp >= 0xD800 && cp <= 0xDFFF) || cp > 0x10FFFF)
            {
                emit(static_cast<charcs>(0xFFFD));
                continue;
            }

            if (cp <= 0xFFFF)
            {
                emit(static_cast<charcs>(cp));
            }
            else
            {
                cp -= 0x10000;
                emit(static_cast<charcs>(0xD800 + (cp >> 10)));   // high surrogate
                emit(static_cast<charcs>(0xDC00 + (cp & 0x3FF))); // low surrogate
            }
        }
    }

    System::Nullable<int> text_input_binding_index(const Keys key)
    {
        switch (key)
        {
        case Keys::Home:   return 0;
        case Keys::End:    return 1;
        case Keys::Back:   return 2;
        case Keys::Tab:    return 3;
        case Keys::Enter:  return 4;
        case Keys::Delete: return 5;
        default:           return {};
        }
    }

    bool control_key_held()
    {
        const auto kb = CNA::Internal::Input::InputManager::GetKeyboardState();
        return kb.IsKeyDown(Keys::LeftControl) || kb.IsKeyDown(Keys::RightControl);
    }

    void handle_text_input_key_down(const Keys key, const bool repeat)
    {
        using Microsoft::Xna::Framework::Input::TextInputEXT;
        const auto idx = text_input_binding_index(key);
        if (idx.getHasValueProperty())
        {
            if (!repeat)
            {
                g_textInputControlDown[idx.getValueProperty()] = true;
            }
            TextInputEXT::INTERNAL_OnTextInput(static_cast<charcs>(kTextInputCharacters[idx.getValueProperty()]));
        }
        else if (control_key_held() && key == Keys::V)
        {
            if (!repeat)
            {
                g_textInputControlDown[6] = true;
                g_textInputSuppress = true;
            }
            TextInputEXT::INTERNAL_OnTextInput(static_cast<charcs>(kTextInputCharacters[6]));
        }
    }

    void handle_text_input_key_up(const Keys key)
    {
        const auto idx = text_input_binding_index(key);
        if (idx.getHasValueProperty())
        {
            g_textInputControlDown[idx.getValueProperty()] = false;
        }
        else if ((!control_key_held() && g_textInputControlDown[6]) || key == Keys::V)
        {
            g_textInputControlDown[6] = false;
            g_textInputSuppress = false;
        }
    }

    std::array<SDL_GameController*, MaxSupportedGamePads>& get_opened_gamepads()
    {
        static std::array<SDL_GameController*, MaxSupportedGamePads> openedGamePads{};
        return openedGamePads;
    }

    std::unordered_map<SDL_JoystickID, PlayerIndex>& get_gamepad_to_player_index_map()
    {
        static std::unordered_map<SDL_JoystickID, PlayerIndex> gamepadToPlayerIndex;
        return gamepadToPlayerIndex;
    }

    // NOXNA/EXT (input_noxna.md N-007): every connected raw joystick, opened for as long as it stays
    // connected. Deliberately independent of the gamepad slot map above — a device SDL also maps as a
    // gamepad is opened here too, so CNA::Input::Joysticks stays a complete, unmapped view.
    std::unordered_map<SDL_JoystickID, SDL_Joystick*>& get_opened_joysticks()
    {
        static std::unordered_map<SDL_JoystickID, SDL_Joystick*> openedJoysticks;
        return openedJoysticks;
    }

    SDL_Joystick* find_opened_joystick(const std::uint32_t id)
    {
        auto& opened = get_opened_joysticks();
        const auto it = opened.find(static_cast<SDL_JoystickID>(id));
        return it != opened.end() ? it->second : nullptr;
    }

    PlayerIndex slot_to_player_index(const std::size_t slot)
    {
        switch (slot)
        {
        case 0:
            return PlayerIndex::One;
        case 1:
            return PlayerIndex::Two;
        case 2:
            return PlayerIndex::Three;
        default:
            return PlayerIndex::Four;
        }
    }

    System::Nullable<std::size_t> try_get_slot_for_player_index(const PlayerIndex playerIndex)
    {
        const int slot = static_cast<int>(playerIndex);
        if (slot < 0 || slot >= static_cast<int>(MaxSupportedGamePads))
        {
            return {};
        }
        return static_cast<std::size_t>(slot);
    }

    System::Nullable<std::size_t> try_find_free_gamepad_slot()
    {
        const auto& openedGamePads = get_opened_gamepads();
        const std::size_t limit = effective_gamepad_count();
        for (std::size_t slot = 0; slot < limit; ++slot)
        {
            if (openedGamePads[slot] == nullptr)
            {
                return slot;
            }
        }
        return {};
    }

    System::Nullable<PlayerIndex> try_get_player_index_for_gamepad_id(const SDL_JoystickID gamePadId)
    {
        const auto& gamepadToPlayerIndex = get_gamepad_to_player_index_map();
        const auto item = gamepadToPlayerIndex.find(gamePadId);
        if (item == gamepadToPlayerIndex.end())
        {
            return {};
        }
        return item->second;
    }

    System::Nullable<GamePadButton> try_convert_sdl_gamepad_button(const SDL_GameControllerButton button)
    {
        switch (button)
        {
        case SDL_CONTROLLER_BUTTON_A:
            return GamePadButton::A;
        case SDL_CONTROLLER_BUTTON_B:
            return GamePadButton::B;
        case SDL_CONTROLLER_BUTTON_X:
            return GamePadButton::X;
        case SDL_CONTROLLER_BUTTON_Y:
            return GamePadButton::Y;
        case SDL_CONTROLLER_BUTTON_BACK:
            return GamePadButton::Back;
        case SDL_CONTROLLER_BUTTON_START:
            return GamePadButton::Start;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            return GamePadButton::LeftShoulder;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            return GamePadButton::RightShoulder;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:
            return GamePadButton::LeftStick;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
            return GamePadButton::RightStick;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return GamePadButton::DPadUp;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return GamePadButton::DPadDown;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return GamePadButton::DPadLeft;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return GamePadButton::DPadRight;
        case SDL_CONTROLLER_BUTTON_GUIDE:
            return GamePadButton::BigButton;
        case SDL_CONTROLLER_BUTTON_MISC1:
            return GamePadButton::Misc1EXT;
        // SDL2's PADDLE1-4 order (upper-left, upper-right, lower-left, lower-right) differs from
        // SDL3's LEFT_PADDLE1/RIGHT_PADDLE1/LEFT_PADDLE2/RIGHT_PADDLE2 naming -- mapped by
        // physical position, not name similarity: RIGHT_PADDLE1 (upper-right) -> PADDLE2,
        // LEFT_PADDLE1 (upper-left) -> PADDLE1, RIGHT_PADDLE2 (lower-right) -> PADDLE4,
        // LEFT_PADDLE2 (lower-left) -> PADDLE3.
        case SDL_CONTROLLER_BUTTON_PADDLE2:
            return GamePadButton::Paddle1EXT;
        case SDL_CONTROLLER_BUTTON_PADDLE1:
            return GamePadButton::Paddle2EXT;
        case SDL_CONTROLLER_BUTTON_PADDLE4:
            return GamePadButton::Paddle3EXT;
        case SDL_CONTROLLER_BUTTON_PADDLE3:
            return GamePadButton::Paddle4EXT;
        case SDL_CONTROLLER_BUTTON_TOUCHPAD:
            return GamePadButton::TouchPadEXT;
        default:
            return {};
        }
    }

    System::Nullable<GamePadAxis> try_convert_sdl_gamepad_axis(const SDL_GameControllerAxis axis)
    {
        switch (axis)
        {
        case SDL_CONTROLLER_AXIS_LEFTX:
            return GamePadAxis::LeftThumbstickX;
        case SDL_CONTROLLER_AXIS_LEFTY:
            return GamePadAxis::LeftThumbstickY;
        case SDL_CONTROLLER_AXIS_RIGHTX:
            return GamePadAxis::RightThumbstickX;
        case SDL_CONTROLLER_AXIS_RIGHTY:
            return GamePadAxis::RightThumbstickY;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            return GamePadAxis::LeftTrigger;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            return GamePadAxis::RightTrigger;
        default:
            return {};
        }
    }

    float normalize_stick_axis(const Sint16 value)
    {
        // Match FNA exactly: it divides the whole Sint16 stick range by 32767 for both the positive
        // (0..32767) and the negative (-32768..0) halves (SDL3_FNAPlatform.cs:1814-1822 — `axis / 32767`,
        // and `axis / -32767` for the inverted Y). The -32768 endpoint yields -1.00003, which FNA resolves
        // to -1.0 via GamePadThumbSticks' clamp; clamping to [-1,1] here gives the byte-identical final
        // value. (An earlier CNA build divided the negative half by 32768, which diverged from FNA at every
        // non-endpoint negative sample, e.g. -16384 → -0.5 instead of FNA's -0.50001.)
        return std::clamp(static_cast<float>(value) / 32767.0f, -1.0f, 1.0f);
    }

    float normalize_trigger_axis(const Sint16 value)
    {
        return std::clamp(static_cast<float>(value) / 32767.0f, 0.0f, 1.0f);
    }

    std::unordered_map<SDL_FingerID, int>& get_finger_id_to_touch_id_map()
    {
        static std::unordered_map<SDL_FingerID, int> fingerIdToTouchId;
        return fingerIdToTouchId;
    }

    int& get_next_touch_id()
    {
        static int nextTouchId = 1;
        return nextTouchId;
    }

    int get_or_create_touch_id(const SDL_FingerID fingerId)
    {
        auto& fingerIdToTouchId = get_finger_id_to_touch_id_map();
        const auto existing = fingerIdToTouchId.find(fingerId);
        if (existing != fingerIdToTouchId.end())
        {
            return existing->second;
        }

        const int touchId = get_next_touch_id();
        get_next_touch_id() += 1;
        fingerIdToTouchId[fingerId] = touchId;
        return touchId;
    }

    System::Nullable<int> try_get_touch_id(const SDL_FingerID fingerId)
    {
        const auto& fingerIdToTouchId = get_finger_id_to_touch_id_map();
        const auto existing = fingerIdToTouchId.find(fingerId);
        if (existing == fingerIdToTouchId.end())
        {
            return {};
        }
        return existing->second;
    }

    void release_touch_id_mapping(const SDL_FingerID fingerId)
    {
        auto& fingerIdToTouchId = get_finger_id_to_touch_id_map();
        fingerIdToTouchId.erase(fingerId);
    }

    /// Converts window-space coordinates to logical (renderer) coordinates.
    /// When SDL_SetRenderLogicalPresentation is active (letterbox on Android),
    /// this maps physical coords into the game's virtual coordinate space.
    /// Falls back to the raw coords if no renderer is available.
    Microsoft::Xna::Framework::Vector2 to_logical_position(SDL_Window* window, float windowX, float windowY)
    {
        if (window != nullptr)
        {
            // SDL_renderer backend: use SDL's built-in logical-presentation transform.
            SDL_Renderer* renderer = SDL_GetRenderer(window);
            if (renderer != nullptr)
            {
                // SDL2's SDL_RenderWindowToLogical returns void and takes integer window
                // coordinates (unlike SDL3's bool-returning, float-input SDL_RenderCoordinatesFromWindow).
                float logX = windowX, logY = windowY;
                SDL_RenderWindowToLogical(
                    renderer, static_cast<int>(windowX), static_cast<int>(windowY), &logX, &logY);
                return Microsoft::Xna::Framework::Vector2(logX, logY);
            }
            // Other backends (e.g. EasyGL): use the backend's own transform if registered.
            auto* backend = CNA::Internal::Backends::IGraphicsBackend::GetForWindow(window);
            if (backend != nullptr)
            {
                float logX = windowX, logY = windowY;
                if (backend->TransformWindowToLogical(windowX, windowY, logX, logY))
                    return Microsoft::Xna::Framework::Vector2(logX, logY);
            }
        }
        return Microsoft::Xna::Framework::Vector2(windowX, windowY);
    }

    // INPUT-TOUCH-024: touch-state coord basis. Scales the normalized SDL coord by the SDL window size
    // then maps to logical space; the gesture path scales by DisplayWidth/Height (linear, FNA-matching).
    // Both target the logical space; they differ only inside letterbox bars (accepted).
    Microsoft::Xna::Framework::Vector2 to_touch_pixel_position(const SDL_TouchFingerEvent& touchEvent)
    {
        SDL_Window* window = nullptr;
        if (touchEvent.windowID != 0)
        {
            window = SDL_GetWindowFromID(touchEvent.windowID);
        }
        if (window == nullptr)
        {
            window = SDL_GetMouseFocus();
        }

        // SDL touch coords are normalized 0..1 relative to the window in points.
        // Convert to window-point coordinates first, then map to logical coords.
        int winW = 1, winH = 1;
        if (window != nullptr)
        {
            SDL_GetWindowSize(window, &winW, &winH);
        }
        const float windowX = touchEvent.x * static_cast<float>(winW);
        const float windowY = touchEvent.y * static_cast<float>(winH);

        return to_logical_position(window, windowX, windowY);
    }

    System::Nullable<Microsoft::Xna::Framework::Input::Keys> try_convert_sdl_key(const SDL_Keycode keycode)
    {
        using Microsoft::Xna::Framework::Input::Keys;
        switch (keycode)
        {
        case SDLK_AC_BACK: return Keys::Escape; // DEC-17: CNA-only Android/browser Back -> Escape (no FNA mapping)
        case SDLK_LEFT: return Keys::Left;
        case SDLK_RIGHT: return Keys::Right;
        case SDLK_UP: return Keys::Up;
        case SDLK_DOWN: return Keys::Down;
        case SDLK_SPACE: return Keys::Space;
        case SDLK_RETURN: return Keys::Enter;
        case SDLK_ESCAPE: return Keys::Escape;
        case SDLK_LCTRL: return Keys::LeftControl;
        case SDLK_RCTRL: return Keys::RightControl;
        case SDLK_LSHIFT: return Keys::LeftShift;
        case SDLK_RSHIFT: return Keys::RightShift;
        case SDLK_TAB: return Keys::Tab;
        case SDLK_a: return Keys::A;
        case SDLK_b: return Keys::B;
        case SDLK_c: return Keys::C;
        case SDLK_d: return Keys::D;
        case SDLK_e: return Keys::E;
        case SDLK_f: return Keys::F;
        case SDLK_g: return Keys::G;
        case SDLK_h: return Keys::H;
        case SDLK_i: return Keys::I;
        case SDLK_j: return Keys::J;
        case SDLK_k: return Keys::K;
        case SDLK_l: return Keys::L;
        case SDLK_m: return Keys::M;
        case SDLK_n: return Keys::N;
        case SDLK_o: return Keys::O;
        case SDLK_p: return Keys::P;
        case SDLK_q: return Keys::Q;
        case SDLK_r: return Keys::R;
        case SDLK_s: return Keys::S;
        case SDLK_t: return Keys::T;
        case SDLK_u: return Keys::U;
        case SDLK_v: return Keys::V;
        case SDLK_w: return Keys::W;
        case SDLK_x: return Keys::X;
        case SDLK_y: return Keys::Y;
        case SDLK_z: return Keys::Z;
        case SDLK_0: return Keys::D0;
        case SDLK_1: return Keys::D1;
        case SDLK_2: return Keys::D2;
        case SDLK_3: return Keys::D3;
        case SDLK_4: return Keys::D4;
        case SDLK_5: return Keys::D5;
        case SDLK_6: return Keys::D6;
        case SDLK_7: return Keys::D7;
        case SDLK_8: return Keys::D8;
        case SDLK_9: return Keys::D9;
        case SDLK_BACKSPACE: return Keys::Back;
        case SDLK_LALT:  return Keys::LeftAlt;
        case SDLK_RALT:  return Keys::RightAlt;
        case SDLK_LGUI:  return Keys::LeftWindows;
        case SDLK_RGUI:  return Keys::RightWindows;
        case SDLK_CAPSLOCK:  return Keys::CapsLock;
        case SDLK_NUMLOCKCLEAR: return Keys::NumLock;
        case SDLK_SCROLLLOCK:   return Keys::Scroll;
        case SDLK_F1:  return Keys::F1;
        case SDLK_F2:  return Keys::F2;
        case SDLK_F3:  return Keys::F3;
        case SDLK_F4:  return Keys::F4;
        case SDLK_F5:  return Keys::F5;
        case SDLK_F6:  return Keys::F6;
        case SDLK_F7:  return Keys::F7;
        case SDLK_F8:  return Keys::F8;
        case SDLK_F9:  return Keys::F9;
        case SDLK_F10: return Keys::F10;
        case SDLK_F11: return Keys::F11;
        case SDLK_F12: return Keys::F12;
        case SDLK_KP_0: return Keys::NumPad0;
        case SDLK_KP_1: return Keys::NumPad1;
        case SDLK_KP_2: return Keys::NumPad2;
        case SDLK_KP_3: return Keys::NumPad3;
        case SDLK_KP_4: return Keys::NumPad4;
        case SDLK_KP_5: return Keys::NumPad5;
        case SDLK_KP_6: return Keys::NumPad6;
        case SDLK_KP_7: return Keys::NumPad7;
        case SDLK_KP_8: return Keys::NumPad8;
        case SDLK_KP_9: return Keys::NumPad9;
        case SDLK_KP_MULTIPLY: return Keys::Multiply;
        case SDLK_KP_PLUS:     return Keys::Add;
        case SDLK_KP_MINUS:    return Keys::Subtract;
        case SDLK_KP_DECIMAL:  return Keys::Decimal;
        case SDLK_KP_DIVIDE:   return Keys::Divide;
        case SDLK_KP_ENTER:    return Keys::Enter;
        case SDLK_SEMICOLON:   return Keys::OemSemicolon;
        case SDLK_EQUALS:      return Keys::OemPlus;
        case SDLK_COMMA:       return Keys::OemComma;
        case SDLK_MINUS:       return Keys::OemMinus;
        case SDLK_PERIOD:      return Keys::OemPeriod;
        case SDLK_SLASH:       return Keys::OemQuestion;
        case SDLK_BACKQUOTE:       return Keys::OemTilde;
        case SDLK_LEFTBRACKET: return Keys::OemOpenBrackets;
        case SDLK_BACKSLASH:   return Keys::OemPipe;
        case SDLK_RIGHTBRACKET:return Keys::OemCloseBrackets;
        case SDLK_QUOTE:  return Keys::OemQuotes;
        case SDLK_PAGEUP:   return Keys::PageUp;
        case SDLK_PAGEDOWN: return Keys::PageDown;
        case SDLK_HOME:     return Keys::Home;
        case SDLK_END:      return Keys::End;
        case SDLK_INSERT:   return Keys::Insert;
        case SDLK_DELETE:   return Keys::Delete;
        case SDLK_PRINTSCREEN: return Keys::PrintScreen;
        case SDLK_PAUSE:       return Keys::Pause;
        case SDLK_F13: return Keys::F13;
        case SDLK_F14: return Keys::F14;
        case SDLK_F15: return Keys::F15;
        case SDLK_F16: return Keys::F16;
        case SDLK_F17: return Keys::F17;
        case SDLK_F18: return Keys::F18;
        case SDLK_F19: return Keys::F19;
        case SDLK_F20: return Keys::F20;
        case SDLK_F21: return Keys::F21;
        case SDLK_F22: return Keys::F22;
        case SDLK_F23: return Keys::F23;
        case SDLK_F24: return Keys::F24;
        case SDLK_APPLICATION: return Keys::Apps;
        case SDLK_MENU:        return Keys::Apps;
        case SDLK_SLEEP:       return Keys::Sleep;
        case SDLK_VOLUMEUP:    return Keys::VolumeUp;
        case SDLK_VOLUMEDOWN:  return Keys::VolumeDown;
        case SDLK_KP_CLEAR:    return Keys::OemClear;
        case SDLK_KP_PERIOD:   return Keys::OemPeriod;
        // Locale keyboard-layout fallbacks: SDL reports the character these physical keys
        // produce on non-US layouts, which differs from the US-layout keycode already mapped
        // above for the same physical key.
        case 0x00B2: return Keys::OemTilde;     // '²' — AZERTY
        case '|':    return Keys::OemPipe;      // Norwegian
        case '+':    return Keys::OemPlus;      // Norwegian
        case 0x00F8: return Keys::OemSemicolon; // 'ø' — Norwegian
        case 0x00E6: return Keys::OemQuotes;    // 'æ' — Norwegian
        case 0x00E9: return {};       // 'é' — BEPO; no real Keys mapping exists yet
        default: return {};
        }
    }

    /// Maps an SDL_Scancode (physical key position) directly to an XNA Keys value, mirroring
    /// FNA's INTERNAL_scanMap (SDL3_FNAPlatform.cs:2490-2618). Used only in scancode mode
    /// (FNA_KEYBOARD_USE_SCANCODES=1), where the physical key position is reported instead of
    /// the character the current keyboard layout produces there.
    System::Nullable<Microsoft::Xna::Framework::Input::Keys> try_convert_sdl_scancode(const SDL_Scancode scancode)
    {
        using Microsoft::Xna::Framework::Input::Keys;
        switch (scancode)
        {
        case SDL_SCANCODE_A: return Keys::A;
        case SDL_SCANCODE_B: return Keys::B;
        case SDL_SCANCODE_C: return Keys::C;
        case SDL_SCANCODE_D: return Keys::D;
        case SDL_SCANCODE_E: return Keys::E;
        case SDL_SCANCODE_F: return Keys::F;
        case SDL_SCANCODE_G: return Keys::G;
        case SDL_SCANCODE_H: return Keys::H;
        case SDL_SCANCODE_I: return Keys::I;
        case SDL_SCANCODE_J: return Keys::J;
        case SDL_SCANCODE_K: return Keys::K;
        case SDL_SCANCODE_L: return Keys::L;
        case SDL_SCANCODE_M: return Keys::M;
        case SDL_SCANCODE_N: return Keys::N;
        case SDL_SCANCODE_O: return Keys::O;
        case SDL_SCANCODE_P: return Keys::P;
        case SDL_SCANCODE_Q: return Keys::Q;
        case SDL_SCANCODE_R: return Keys::R;
        case SDL_SCANCODE_S: return Keys::S;
        case SDL_SCANCODE_T: return Keys::T;
        case SDL_SCANCODE_U: return Keys::U;
        case SDL_SCANCODE_V: return Keys::V;
        case SDL_SCANCODE_W: return Keys::W;
        case SDL_SCANCODE_X: return Keys::X;
        case SDL_SCANCODE_Y: return Keys::Y;
        case SDL_SCANCODE_Z: return Keys::Z;
        case SDL_SCANCODE_0: return Keys::D0;
        case SDL_SCANCODE_1: return Keys::D1;
        case SDL_SCANCODE_2: return Keys::D2;
        case SDL_SCANCODE_3: return Keys::D3;
        case SDL_SCANCODE_4: return Keys::D4;
        case SDL_SCANCODE_5: return Keys::D5;
        case SDL_SCANCODE_6: return Keys::D6;
        case SDL_SCANCODE_7: return Keys::D7;
        case SDL_SCANCODE_8: return Keys::D8;
        case SDL_SCANCODE_9: return Keys::D9;
        case SDL_SCANCODE_KP_0: return Keys::NumPad0;
        case SDL_SCANCODE_KP_1: return Keys::NumPad1;
        case SDL_SCANCODE_KP_2: return Keys::NumPad2;
        case SDL_SCANCODE_KP_3: return Keys::NumPad3;
        case SDL_SCANCODE_KP_4: return Keys::NumPad4;
        case SDL_SCANCODE_KP_5: return Keys::NumPad5;
        case SDL_SCANCODE_KP_6: return Keys::NumPad6;
        case SDL_SCANCODE_KP_7: return Keys::NumPad7;
        case SDL_SCANCODE_KP_8: return Keys::NumPad8;
        case SDL_SCANCODE_KP_9: return Keys::NumPad9;
        case SDL_SCANCODE_KP_CLEAR: return Keys::OemClear;
        case SDL_SCANCODE_KP_DECIMAL: return Keys::Decimal;
        case SDL_SCANCODE_KP_DIVIDE: return Keys::Divide;
        case SDL_SCANCODE_KP_ENTER: return Keys::Enter;
        case SDL_SCANCODE_KP_MINUS: return Keys::Subtract;
        case SDL_SCANCODE_KP_MULTIPLY: return Keys::Multiply;
        case SDL_SCANCODE_KP_PERIOD: return Keys::OemPeriod;
        case SDL_SCANCODE_KP_PLUS: return Keys::Add;
        case SDL_SCANCODE_F1: return Keys::F1;
        case SDL_SCANCODE_F2: return Keys::F2;
        case SDL_SCANCODE_F3: return Keys::F3;
        case SDL_SCANCODE_F4: return Keys::F4;
        case SDL_SCANCODE_F5: return Keys::F5;
        case SDL_SCANCODE_F6: return Keys::F6;
        case SDL_SCANCODE_F7: return Keys::F7;
        case SDL_SCANCODE_F8: return Keys::F8;
        case SDL_SCANCODE_F9: return Keys::F9;
        case SDL_SCANCODE_F10: return Keys::F10;
        case SDL_SCANCODE_F11: return Keys::F11;
        case SDL_SCANCODE_F12: return Keys::F12;
        case SDL_SCANCODE_F13: return Keys::F13;
        case SDL_SCANCODE_F14: return Keys::F14;
        case SDL_SCANCODE_F15: return Keys::F15;
        case SDL_SCANCODE_F16: return Keys::F16;
        case SDL_SCANCODE_F17: return Keys::F17;
        case SDL_SCANCODE_F18: return Keys::F18;
        case SDL_SCANCODE_F19: return Keys::F19;
        case SDL_SCANCODE_F20: return Keys::F20;
        case SDL_SCANCODE_F21: return Keys::F21;
        case SDL_SCANCODE_F22: return Keys::F22;
        case SDL_SCANCODE_F23: return Keys::F23;
        case SDL_SCANCODE_F24: return Keys::F24;
        case SDL_SCANCODE_SPACE: return Keys::Space;
        case SDL_SCANCODE_UP: return Keys::Up;
        case SDL_SCANCODE_DOWN: return Keys::Down;
        case SDL_SCANCODE_LEFT: return Keys::Left;
        case SDL_SCANCODE_RIGHT: return Keys::Right;
        case SDL_SCANCODE_LALT: return Keys::LeftAlt;
        case SDL_SCANCODE_RALT: return Keys::RightAlt;
        case SDL_SCANCODE_LCTRL: return Keys::LeftControl;
        case SDL_SCANCODE_RCTRL: return Keys::RightControl;
        case SDL_SCANCODE_LGUI: return Keys::LeftWindows;
        case SDL_SCANCODE_RGUI: return Keys::RightWindows;
        case SDL_SCANCODE_LSHIFT: return Keys::LeftShift;
        case SDL_SCANCODE_RSHIFT: return Keys::RightShift;
        case SDL_SCANCODE_APPLICATION: return Keys::Apps;
        case SDL_SCANCODE_MENU: return Keys::Apps;
        case SDL_SCANCODE_SLASH: return Keys::OemQuestion;
        case SDL_SCANCODE_BACKSLASH: return Keys::OemPipe;
        case SDL_SCANCODE_LEFTBRACKET: return Keys::OemOpenBrackets;
        case SDL_SCANCODE_RIGHTBRACKET: return Keys::OemCloseBrackets;
        case SDL_SCANCODE_CAPSLOCK: return Keys::CapsLock;
        case SDL_SCANCODE_COMMA: return Keys::OemComma;
        case SDL_SCANCODE_DELETE: return Keys::Delete;
        case SDL_SCANCODE_END: return Keys::End;
        case SDL_SCANCODE_BACKSPACE: return Keys::Back;
        case SDL_SCANCODE_RETURN: return Keys::Enter;
        case SDL_SCANCODE_ESCAPE: return Keys::Escape;
        case SDL_SCANCODE_HOME: return Keys::Home;
        case SDL_SCANCODE_INSERT: return Keys::Insert;
        case SDL_SCANCODE_MINUS: return Keys::OemMinus;
        case SDL_SCANCODE_NUMLOCKCLEAR: return Keys::NumLock;
        case SDL_SCANCODE_PAGEUP: return Keys::PageUp;
        case SDL_SCANCODE_PAGEDOWN: return Keys::PageDown;
        case SDL_SCANCODE_PAUSE: return Keys::Pause;
        case SDL_SCANCODE_PERIOD: return Keys::OemPeriod;
        case SDL_SCANCODE_EQUALS: return Keys::OemPlus;
        case SDL_SCANCODE_PRINTSCREEN: return Keys::PrintScreen;
        case SDL_SCANCODE_APOSTROPHE: return Keys::OemQuotes;
        case SDL_SCANCODE_SCROLLLOCK: return Keys::Scroll;
        case SDL_SCANCODE_SEMICOLON: return Keys::OemSemicolon;
        case SDL_SCANCODE_SLEEP: return Keys::Sleep;
        case SDL_SCANCODE_TAB: return Keys::Tab;
        case SDL_SCANCODE_GRAVE: return Keys::OemTilde;
        case SDL_SCANCODE_VOLUMEUP: return Keys::VolumeUp;
        case SDL_SCANCODE_VOLUMEDOWN: return Keys::VolumeDown;
        // INPUT-KBD-011/019: scancodes with no XNA Keys value are DROPPED (no value), never mapped to
        // Keys::None — the same DEC-16 policy already applied to unmapped keycodes, so Keys::None never
        // enters the pressed set (IsKeyDown(None) stays false; None never leaks into GetPressedKeys()).
        // This covers the no-scancode sentinel (SDL_SCANCODE_UNKNOWN, matching the keycode path's SDLK_
        // UNKNOWN drop) and the two ISO-layout extra keys (NONUSHASH on UK, NONUSBACKSLASH on most ISO
        // boards), which FNA maps to Keys.None with its own unresolved "need verification" FIXME
        // (SDL3_FNAPlatform.cs:2615-2617) and adds to its pressed list. A deliberate, DEC-16-consistent
        // deviation from FNA — recorded in docs/input-fna-fidelity.md, pinned by SdlInputBridgeKeyboardTest.
        case SDL_SCANCODE_UNKNOWN: return {};
        case SDL_SCANCODE_NONUSHASH: return {};
        case SDL_SCANCODE_NONUSBACKSLASH: return {};
        default: return {};
        }
    }

}

namespace CNA::Internal::Input
{











    static CNA::Input::PowerStateEXT sdl_power_state_to_ext(SDL_PowerState state)
    {
        using CNA::Input::PowerStateEXT;
        switch (state)
        {
            case SDL_POWERSTATE_ON_BATTERY: return PowerStateEXT::OnBattery;
            case SDL_POWERSTATE_NO_BATTERY: return PowerStateEXT::NoBattery;
            case SDL_POWERSTATE_CHARGING:   return PowerStateEXT::Charging;
            case SDL_POWERSTATE_CHARGED:    return PowerStateEXT::Charged;
            case SDL_POWERSTATE_UNKNOWN:    return PowerStateEXT::Unknown;
            // SDL2 has no SDL_POWERSTATE_ERROR value (an SDL3-only addition) -- falls through to
            // the default case below, same as it did before this migration.
            default:                        return PowerStateEXT::Error;
        }
    }














    // NOXNA/EXT (input_noxna.md N-007): SDL_JoystickType -> the raw-joystick CNA::Input enum. Distinct
    // from sdl_joystick_type_to_gamepad_type above (that one maps into XNA's GamePadType for mapped
    // pads; this one is the unmapped CNA::Input::Joysticks surface).
    static CNA::Input::JoystickTypeEXT sdl_joystick_type_to_ext(SDL_JoystickType t)
    {
        using CNA::Input::JoystickTypeEXT;
        switch (t)
        {
        case SDL_JOYSTICK_TYPE_GAMECONTROLLER: return JoystickTypeEXT::Gamepad; // SDL2 name (SDL3: SDL_JOYSTICK_TYPE_GAMEPAD)
        case SDL_JOYSTICK_TYPE_WHEEL:        return JoystickTypeEXT::Wheel;
        case SDL_JOYSTICK_TYPE_ARCADE_STICK: return JoystickTypeEXT::ArcadeStick;
        case SDL_JOYSTICK_TYPE_FLIGHT_STICK: return JoystickTypeEXT::FlightStick;
        case SDL_JOYSTICK_TYPE_DANCE_PAD:    return JoystickTypeEXT::DancePad;
        case SDL_JOYSTICK_TYPE_GUITAR:       return JoystickTypeEXT::Guitar;
        case SDL_JOYSTICK_TYPE_DRUM_KIT:     return JoystickTypeEXT::DrumKit;
        case SDL_JOYSTICK_TYPE_ARCADE_PAD:   return JoystickTypeEXT::ArcadePad;
        case SDL_JOYSTICK_TYPE_THROTTLE:     return JoystickTypeEXT::Throttle;
        default:                             return JoystickTypeEXT::Unknown;
        }
    }

    // NOXNA/EXT (input_noxna.md N-007): SDL's SDL_HAT_* bitmask -> the 9-value hat-position enum.
    static CNA::Input::JoystickHatPositionEXT sdl_hat_to_ext(const Uint8 hat)
    {
        using CNA::Input::JoystickHatPositionEXT;
        switch (hat)
        {
        case SDL_HAT_UP:        return JoystickHatPositionEXT::Up;
        case SDL_HAT_RIGHT:     return JoystickHatPositionEXT::Right;
        case SDL_HAT_DOWN:      return JoystickHatPositionEXT::Down;
        case SDL_HAT_LEFT:      return JoystickHatPositionEXT::Left;
        case SDL_HAT_RIGHTUP:   return JoystickHatPositionEXT::RightUp;
        case SDL_HAT_RIGHTDOWN: return JoystickHatPositionEXT::RightDown;
        case SDL_HAT_LEFTUP:    return JoystickHatPositionEXT::LeftUp;
        case SDL_HAT_LEFTDOWN:  return JoystickHatPositionEXT::LeftDown;
        default:                return JoystickHatPositionEXT::Centered;
        }
    }


    std::vector<CNA::Input::JoystickInfoEXT> SdlInputBridge::GetJoysticks()
    {
        std::vector<CNA::Input::JoystickInfoEXT> result;
        for (const auto& [id, joystick] : get_opened_joysticks())
        {
            CNA::Input::JoystickInfoEXT info;
            info.id = static_cast<std::uint32_t>(id);
            info.name = sdl_joystick_backend().GetJoystickName(joystick);
            info.type = sdl_joystick_type_to_ext(sdl_joystick_backend().GetJoystickType(joystick));
            result.push_back(std::move(info));
        }
        return result;
    }

    CNA::Input::JoystickCapabilitiesEXT SdlInputBridge::GetJoystickCapabilities(const std::uint32_t id)
    {
        SDL_Joystick* joystick = find_opened_joystick(id);
        if (joystick == nullptr)
            return CNA::Input::JoystickCapabilitiesEXT{};

        CNA::Input::JoystickCapabilitiesEXT caps;
        caps.isConnected = true;
        caps.axisCount = sdl_joystick_backend().GetNumJoystickAxes(joystick);
        caps.buttonCount = sdl_joystick_backend().GetNumJoystickButtons(joystick);
        caps.hatCount = sdl_joystick_backend().GetNumJoystickHats(joystick);
        caps.ballCount = sdl_joystick_backend().GetNumJoystickBalls(joystick);
        caps.type = sdl_joystick_type_to_ext(sdl_joystick_backend().GetJoystickType(joystick));
        caps.name = sdl_joystick_backend().GetJoystickName(joystick);
        caps.guid = sdl_joystick_backend().GetJoystickGUID(joystick);

        int percent = -1;
        caps.powerState = sdl_power_state_to_ext(sdl_joystick_backend().GetJoystickPowerInfo(joystick, &percent));
        caps.powerPercent = percent;

        return caps;
    }

    CNA::Input::JoystickStateEXT SdlInputBridge::GetJoystickState(const std::uint32_t id)
    {
        CNA::Input::JoystickStateEXT state;

        SDL_Joystick* joystick = find_opened_joystick(id);
        if (joystick == nullptr)
            return state;

        auto& backend = sdl_joystick_backend();

        const int axisCount = backend.GetNumJoystickAxes(joystick);
        state.axes.reserve(static_cast<std::size_t>(std::max(axisCount, 0)));
        for (int axis = 0; axis < axisCount; ++axis)
            state.axes.push_back(backend.GetJoystickAxis(joystick, axis));

        const int buttonCount = backend.GetNumJoystickButtons(joystick);
        state.buttons.reserve(static_cast<std::size_t>(std::max(buttonCount, 0)));
        for (int button = 0; button < buttonCount; ++button)
            state.buttons.push_back(backend.GetJoystickButton(joystick, button));

        const int hatCount = backend.GetNumJoystickHats(joystick);
        state.hats.reserve(static_cast<std::size_t>(std::max(hatCount, 0)));
        for (int hat = 0; hat < hatCount; ++hat)
            state.hats.push_back(sdl_hat_to_ext(backend.GetJoystickHat(joystick, hat)));

        const int ballCount = backend.GetNumJoystickBalls(joystick);
        state.balls.reserve(static_cast<std::size_t>(std::max(ballCount, 0)));
        for (int ball = 0; ball < ballCount; ++ball)
        {
            int dx = 0;
            int dy = 0;
            backend.GetJoystickBall(joystick, ball, &dx, &dy);
            state.balls.emplace_back(dx, dy);
        }

        return state;
    }

    SDL_Joystick* SdlInputBridge::GetOpenedJoystickHandle(const std::uint32_t id)
    {
        return find_opened_joystick(id);
    }

    void SdlInputBridge::EnsureGamepadSubsystemInitialized()
    {
        if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) & SDL_INIT_GAMECONTROLLER)
        {
            return;
        }
        // Deliver gamepad button/axis events even when the game window is not focused, matching
        // FNA (SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS = "1").
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    void SdlInputBridge::ShutdownGamepadSubsystem()
    {
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    void SdlInputBridge::ProcessEvent(const SDL_Event& event)
    {
        // SDL_INIT_VIDEO does not init the gamepad subsystem; do it lazily on first event so
        // gamepad add/remove/axis/button events flow (and already-connected pads are enumerated).
        EnsureGamepadSubsystemInitialized();

        switch (event.type)
        {
        case SDL_MOUSEMOTION:
            {
                SDL_Window* win = (event.motion.windowID != 0)
                                      ? SDL_GetWindowFromID(event.motion.windowID)
                                      : SDL_GetMouseFocus();
                const auto pos = to_logical_position(win, event.motion.x, event.motion.y);
                InputManager::SetMousePosition(static_cast<int>(pos.X), static_cast<int>(pos.Y));
                InputManager::AddMouseRelativeDelta(event.motion.xrel, event.motion.yrel);
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            {
                const auto state =
                    event.type == SDL_MOUSEBUTTONDOWN
                        ? Microsoft::Xna::Framework::Input::ButtonState::Pressed
                        : Microsoft::Xna::Framework::Input::ButtonState::Released;

                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    InputManager::SetMouseButtonState(MouseButton::Left, state);
                    break;
                case SDL_BUTTON_RIGHT:
                    InputManager::SetMouseButtonState(MouseButton::Right, state);
                    break;
                case SDL_BUTTON_MIDDLE:
                    InputManager::SetMouseButtonState(MouseButton::Middle, state);
                    break;
                case SDL_BUTTON_X1:
                    InputManager::SetMouseButtonState(MouseButton::XButton1, state);
                    break;
                case SDL_BUTTON_X2:
                    InputManager::SetMouseButtonState(MouseButton::XButton2, state);
                    break;
                default:
                    break;
                }

                {
                    SDL_Window* win = (event.button.windowID != 0)
                                          ? SDL_GetWindowFromID(event.button.windowID)
                                          : SDL_GetMouseFocus();
                    const auto pos = to_logical_position(win, event.button.x, event.button.y);
                    InputManager::SetMousePosition(static_cast<int>(pos.X), static_cast<int>(pos.Y));
                }

                if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                    Microsoft::Xna::Framework::Input::Mouse::INTERNAL_onClicked(event.button.button - 1);
                }
                break;
            }
        case SDL_MOUSEWHEEL:
            // Vertical wheel = the XNA-faithful cumulative ScrollWheelValue. FNA truncates the SDL wheel
            // delta to whole notches BEFORE scaling by 120 (`(int) evt.wheel.y * 120`, SDL3_FNAPlatform.cs)
            // — the cast binds tighter than the multiply, so sub-notch fractional motion from high-
            // resolution / precision trackpads is discarded, keeping ScrollWheelValue a clean multiple of
            // 120 exactly as XNA reports. We cast first to match that; do NOT multiply the float then cast
            // (that would leak fractional deltas and diverge from FNA/XNA).
            InputManager::AddScrollWheelDelta(
                static_cast<int>(event.wheel.y) * 120
            );
            // Horizontal wheel = a NOXNA/EXT extension (XNA/FNA have no horizontal member). Previously
            // dropped (DEC-18); now surfaced via MouseState::getHorizontalScrollWheelValueEXTProperty. Same
            // cast-then-scale-by-120 truncation as the vertical wheel so it stays a clean notch multiple.
            InputManager::AddHorizontalScrollWheelDelta(
                static_cast<int>(event.wheel.x) * 120
            );
            break;
        // NOXNA/EXT (input_noxna.md N-017b): device hot-plug events routed to
        // CNA::Input::InputDevices. Post-Phase-4 (SDL2 migration): SDL2 has no per-device
        // mouse/keyboard added/removed events at all (SDL_EVENT_MOUSE_ADDED/REMOVED/
        // SDL_EVENT_KEYBOARD_ADDED/REMOVED are SDL3-only additions -- SDL2 treats every physical
        // mouse as one combined logical pointer and every physical keyboard as one combined
        // logical keyboard, matching SystemDeviceBackend.cpp's identical gap for GetMice()/
        // GetKeyboards()). MouseConnectedEXT/MouseDisconnectedEXT/KeyboardConnectedEXT/
        // KeyboardDisconnectedEXT simply never fire under this backend -- a genuine SDL2
        // capability gap, not exercised by mobile-eggbert itself. See plan_lite.md's Phase 4
        // status for the full disclosure.
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            {
                // Mirrors FNA's ToXNAKey (SDL3_FNAPlatform.cs:2743-2766): in scancode mode,
                // the physical key position (scancode) is used instead of the layout-dependent
                // keycode, so games get consistent physical-key bindings across keyboard layouts.
                const auto key = use_scancode_mode()
                                      ? try_convert_sdl_scancode(event.key.keysym.scancode)
                                      : try_convert_sdl_key(event.key.keysym.sym);

#ifdef __ANDROID__
                {
                    const char* evtName = (event.type == SDL_KEYDOWN) ? "KEY_DOWN" : "KEY_UP";
                    const char* keyName = SDL_GetKeyName(event.key.keysym.sym);
                    if (key.getHasValueProperty())
                    {
                        SDL_Log("[Keyboard] SDL_%s scancode=%d keycode=%d (0x%x) keyname='%s' mod=0x%x -> XNA Keys=%d",
                                evtName,
                                static_cast<int>(event.key.keysym.scancode),
                                static_cast<int>(event.key.keysym.sym),
                                static_cast<unsigned>(event.key.keysym.sym),
                                keyName ? keyName : "?",
                                static_cast<unsigned>(event.key.keysym.mod),
                                static_cast<int>(key.getValueProperty()));
                    }
                    else
                    {
                        SDL_Log("[Keyboard] SDL_%s scancode=%d keycode=%d (0x%x) keyname='%s' mod=0x%x -> unmapped",
                                evtName,
                                static_cast<int>(event.key.keysym.scancode),
                                static_cast<int>(event.key.keysym.sym),
                                static_cast<unsigned>(event.key.keysym.sym),
                                keyName ? keyName : "?",
                                static_cast<unsigned>(event.key.keysym.mod));
                    }
                }
#endif

                if (!key.getHasValueProperty())
                {
                    break;
                }

                const bool pressed  = event.type == SDL_KEYDOWN;
                const bool isRepeat = pressed && event.key.repeat;

                // DEC-19: repeats keep the key down (state already set); FNA only re-emits text
                // input on repeat, so skip the pressed-key state update for repeats.
                if (!isRepeat)
                {
                    InputManager::SetKeyState(key.getValueProperty(), pressed);
                }

                // Synthesize TextInput for control keys SDL doesn't deliver as TEXT_INPUT
                // (Home/End/Back/Tab/Enter/Delete and Ctrl+V).
                if (pressed)
                {
                    handle_text_input_key_down(key.getValueProperty(), isRepeat);
                }
                else
                {
                    handle_text_input_key_up(key.getValueProperty());
                }

#ifdef __ANDROID__
                {
                    const auto snapshot = CNA::Internal::Input::InputManager::GetKeyboardState();
                    const auto allPressed = snapshot.GetPressedKeys();
                    std::string keyList;
                    for (const auto k : allPressed)
                    {
                        keyList += std::to_string(static_cast<int>(k));
                        keyList += ' ';
                    }
                    SDL_Log("[Keyboard] KeyboardState updated: XNA Keys=%d pressed=%s | total pressed=%zu [%s]",
                            static_cast<int>(key.getValueProperty()),
                            pressed ? "true" : "false",
                            allPressed.size(),
                            keyList.c_str());
                }
#endif
                break;
            }
        case SDL_TEXTINPUT:
            {
                // Suppress the literal character SDL emits alongside a synthesized paste
                // (Ctrl+V): the paste control char (22) was already sent on KEY_DOWN.
                if (g_textInputSuppress)
                {
                    break;
                }
                // SDL delivers UTF-8 text in event.text.text. Decode it to UTF-16 code units and
                // dispatch each, matching FNA exactly (Encoding.UTF8.GetChars ->
                // TextInputEXT.OnTextInput per char, SDL3_FNAPlatform.cs:1166-1184). CNA's
                // TextInput callback is charcs (char16_t) — one UTF-16 code unit per call, with
                // astral code points delivered as a surrogate pair, just like FNA's C# char.
                if (const char* text = event.text.text)
                {
                    decode_utf8_to_utf16(text, [](const charcs cu)
                    {
                        Microsoft::Xna::Framework::Input::TextInputEXT::INTERNAL_OnTextInput(cu);
                    });
                }
                break;
            }
        case SDL_TEXTEDITING:
            {
                // IME composition draft text (UTF-8). Pass the bytes straight through to
                // CNA's UTF-8 std::string callback. FNA passes null for an empty composition;
                // CNA maps that to an empty string with start/length 0 (std::string& can't be null).
                if (event.edit.text != nullptr && event.edit.text[0] != '\0')
                {
                    Microsoft::Xna::Framework::Input::TextInputEXT::INTERNAL_OnTextEditing(
                        std::string(event.edit.text),
                        event.edit.start,
                        event.edit.length);
                }
                else
                {
                    Microsoft::Xna::Framework::Input::TextInputEXT::INTERNAL_OnTextEditing(
                        std::string(), 0, 0);
                }
                break;
            }
        // NOXNA/EXT (input_noxna.md N-014): SDL3-only IME candidate-list event
        // (SDL_EVENT_TEXT_EDITING_CANDIDATES) -- SDL2 has no equivalent event or
        // `event.edit_candidates` union member at all, so TextEditingCandidatesEXT simply never
        // fires under this backend. A genuine SDL2 capability gap, not exercised by
        // mobile-eggbert itself -- see plan_lite.md's Phase 4 status for the full disclosure.
        case SDL_FINGERDOWN:
            {
                // Windows only notices a touch screen once it's touched (FNA SDL3_FNAPlatform.cs:972).
                Microsoft::Xna::Framework::Input::Touch::TouchPanel::setTouchDeviceExistsProperty(true);

                const int touchId = get_or_create_touch_id(event.tfinger.fingerId);
                InputManager::SetTouchState(
                    touchId,
                    TouchLocationState::Pressed,
                    to_touch_pixel_position(event.tfinger),
                    event.tfinger.pressure
                );
                Microsoft::Xna::Framework::Input::Touch::TouchPanel::INTERNAL_onTouchEvent(
                    touchId,
                    TouchLocationState::Pressed,
                    event.tfinger.x,
                    event.tfinger.y,
                    0.0f,
                    0.0f
                );
                break;
            }
        case SDL_FINGERMOTION:
            {
                const int touchId = get_or_create_touch_id(event.tfinger.fingerId);
                InputManager::SetTouchState(
                    touchId,
                    TouchLocationState::Moved,
                    to_touch_pixel_position(event.tfinger),
                    event.tfinger.pressure
                );
                Microsoft::Xna::Framework::Input::Touch::TouchPanel::INTERNAL_onTouchEvent(
                    touchId,
                    TouchLocationState::Moved,
                    event.tfinger.x,
                    event.tfinger.y,
                    event.tfinger.dx,
                    event.tfinger.dy
                );
                break;
            }
        // SDL2 has no SDL_EVENT_FINGER_CANCELED equivalent (an SDL3-only addition) -- SDL_FINGERUP
        // is the only "touch ended" signal SDL2 ever delivers, so this case list is now just the
        // one value (harmless: FNA's own "treat canceled identically to lifted" rule collapses to
        // a no-op distinction when SDL2 never generates the canceled variant in the first place).
        case SDL_FINGERUP:
            {
                const auto touchIdOpt = try_get_touch_id(event.tfinger.fingerId);
                const int touchId = touchIdOpt.getHasValueProperty()
                    ? touchIdOpt.getValueProperty()
                    : get_or_create_touch_id(event.tfinger.fingerId);

                InputManager::SetTouchState(
                    touchId,
                    TouchLocationState::Released,
                    to_touch_pixel_position(event.tfinger),
                    event.tfinger.pressure
                );
                Microsoft::Xna::Framework::Input::Touch::TouchPanel::INTERNAL_onTouchEvent(
                    touchId,
                    TouchLocationState::Released,
                    event.tfinger.x,
                    event.tfinger.y,
                    0.0f,
                    0.0f
                );
                release_touch_id_mapping(event.tfinger.fingerId);
                break;
            }
        case SDL_CONTROLLERDEVICEADDED:
            {
                if (!sdl_gamepad_backend().IsGamepad(event.cdevice.which))
                {
                    break;
                }

                auto& gamepadToPlayerIndex = get_gamepad_to_player_index_map();
                if (gamepadToPlayerIndex.contains(event.cdevice.which))
                {
                    break;
                }

                const auto freeSlot = try_find_free_gamepad_slot();
                if (!freeSlot.getHasValueProperty())
                {
                    break;
                }

                SDL_GameController* gamepad = sdl_gamepad_backend().OpenGamepad(event.cdevice.which);
                if (gamepad == nullptr)
                {
                    break;
                }

                const PlayerIndex playerIndex = slot_to_player_index(freeSlot.getValueProperty());
                get_opened_gamepads()[freeSlot.getValueProperty()] = gamepad;
                gamepadToPlayerIndex[event.cdevice.which] = playerIndex;
                InputManager::SetGamePadConnection(playerIndex, true);
                break;
            }
        case SDL_CONTROLLERDEVICEREMOVED:
            {
                auto& gamepadToPlayerIndex = get_gamepad_to_player_index_map();
                const auto playerIndex = try_get_player_index_for_gamepad_id(event.cdevice.which);
                if (!playerIndex.getHasValueProperty())
                {
                    break;
                }

                const auto slot = try_get_slot_for_player_index(playerIndex.getValueProperty());
                if (slot.getHasValueProperty())
                {
                    auto& openedGamePad = get_opened_gamepads()[slot.getValueProperty()];
                    if (openedGamePad != nullptr)
                    {
                        sdl_gamepad_backend().CloseGamepad(openedGamePad);
                        openedGamePad = nullptr;
                    }
                }

                gamepadToPlayerIndex.erase(event.cdevice.which);
                InputManager::SetGamePadConnection(playerIndex.getValueProperty(), false);
                break;
            }
        // NOXNA/EXT (input_noxna.md N-007): raw joystick hot-plug, routed to CNA::Input::Joysticks.
        // Every connected joystick is opened here — including devices SDL also maps as a gamepad
        // above — so raw axis/button/hat/trackball state stays queryable independent of GamePad.
        // Axis/button/hat/ball motion events need no handling: SDL's own event pump already updates
        // its internal joystick-state cache (that's what SDL_JoystickGetAxis/Button/Hat/Ball read),
        // so GetJoystickState can poll live values on demand, exactly like the gamepad EXT getters do.
        case SDL_JOYDEVICEADDED:
            {
                auto& opened = get_opened_joysticks();
                if (opened.contains(event.jdevice.which))
                {
                    break;
                }

                SDL_Joystick* joystick = sdl_joystick_backend().OpenJoystick(event.jdevice.which);
                if (joystick == nullptr)
                {
                    break;
                }

                opened[event.jdevice.which] = joystick;
                CNA::Input::Joysticks::ConnectedEXT.Invoke(static_cast<std::uint32_t>(event.jdevice.which));
                break;
            }
        case SDL_JOYDEVICEREMOVED:
            {
                auto& opened = get_opened_joysticks();
                const auto it = opened.find(event.jdevice.which);
                if (it == opened.end())
                {
                    break;
                }

                sdl_joystick_backend().CloseJoystick(it->second);
                opened.erase(it);
                CNA::Input::Joysticks::DisconnectedEXT.Invoke(static_cast<std::uint32_t>(event.jdevice.which));
                break;
            }
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            {
                const auto playerIndex = try_get_player_index_for_gamepad_id(event.cbutton.which);
                if (!playerIndex.getHasValueProperty())
                {
                    break;
                }

                const auto button = try_convert_sdl_gamepad_button(
                    static_cast<SDL_GameControllerButton>(event.cbutton.button)
                );
                if (!button.getHasValueProperty())
                {
                    break;
                }

                const auto state = event.type == SDL_CONTROLLERBUTTONDOWN
                                       ? ButtonState::Pressed
                                       : ButtonState::Released;

                InputManager::SetGamePadButtonState(playerIndex.getValueProperty(), button.getValueProperty(), state);
                break;
            }
        case SDL_CONTROLLERAXISMOTION:
            {
                const auto playerIndex = try_get_player_index_for_gamepad_id(event.caxis.which);
                if (!playerIndex.getHasValueProperty())
                {
                    break;
                }

                const auto axis = try_convert_sdl_gamepad_axis(
                    static_cast<SDL_GameControllerAxis>(event.caxis.axis)
                );
                if (!axis.getHasValueProperty())
                {
                    break;
                }

                float value = 0.0f;
                switch (axis.getValueProperty())
                {
                case GamePadAxis::LeftThumbstickX:
                case GamePadAxis::RightThumbstickX:
                    value = normalize_stick_axis(event.caxis.value);
                    break;
                case GamePadAxis::LeftThumbstickY:
                case GamePadAxis::RightThumbstickY:
                    value = -normalize_stick_axis(event.caxis.value);
                    break;
                case GamePadAxis::LeftTrigger:
                case GamePadAxis::RightTrigger:
                    value = normalize_trigger_axis(event.caxis.value);
                    break;
                }

                InputManager::SetGamePadAxisValue(playerIndex.getValueProperty(), axis.getValueProperty(), value);
                break;
            }
        default:
            break;
        }
    }
}
