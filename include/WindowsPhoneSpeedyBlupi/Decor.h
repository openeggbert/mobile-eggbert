//
// Created by robertvokac on 5/24/25.
//

#ifndef DECOR_H
#define DECOR_H
#include "Def.h"

namespace WindowsPhoneSpeedyBlupi{

class Decor {
public: int a;
    static std::string GetCheatTinyText(Def::ButtonGlyph glyph);

    void SetSpeedX(double horizontal_change) const;

    void SetSpeedY(double vertical_change) const;

    void KeyChange(int key_press) const;
};
}


#endif //DECOR_H
