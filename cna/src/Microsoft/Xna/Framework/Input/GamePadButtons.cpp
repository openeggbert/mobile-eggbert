// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Input/GamePadButtons.hpp"

namespace Microsoft::Xna::Framework::Input
{
    GamePadButtons::GamePadButtons()
        : buttons_(static_cast<Buttons>(0))
    {
    }

    GamePadButtons::GamePadButtons(Buttons buttons)
        : buttons_(buttons)
    {
    }

    GamePadButtons GamePadButtons::FromButtonArray(std::initializer_list<Buttons> btns)
    {
        Buttons mask = static_cast<Buttons>(0);
        for (Buttons b : btns)
            mask |= b;
        return GamePadButtons(mask);
    }

    ButtonState GamePadButtons::ButtonStateFromFlag(Buttons flag) const
    {
        return (buttons_ & flag) == flag ? ButtonState::Pressed : ButtonState::Released;
    }

    ButtonState GamePadButtons::getBackProperty() const          { return ButtonStateFromFlag(Buttons::Back); }

    bool GamePadButtons::Equals(const GamePadButtons& other) const
    {
        return buttons_ == other.buttons_;
    }

    int GamePadButtons::GetHashCode() const
    {
        return static_cast<int>(static_cast<uint32_t>(buttons_));
    }

    bool operator==(const GamePadButtons& left, const GamePadButtons& right)
    {
        return left.Equals(right);
    }

    bool operator!=(const GamePadButtons& left, const GamePadButtons& right)
    {
        return !(left == right);
    }
}
