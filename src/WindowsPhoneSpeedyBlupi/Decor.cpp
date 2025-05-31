//
// Created by robertvokac on 5/24/25.
//

#include "WindowsPhoneSpeedyBlupi/Decor.h"

namespace WindowsPhoneSpeedyBlupi {
    TinyRect Decor::getDrawBounds() const { return m_drawBounds; }
    void Decor::setDrawBounds(const TinyRect &v) { m_drawBounds = v; }
    idata(Def::ButtonGlyph, ButtonPressed, Decor)

    void Decor::MoveObjectCopy(MoveObject& dst, const MoveObject &src)
    {
        dst.type = src.type;
        dst.stepAdvance = src.stepAdvance;
        dst.stepRecede = src.stepRecede;
        dst.timeStopStart = src.timeStopStart;
        dst.timeStopEnd = src.timeStopEnd;
        dst.posStart = src.posStart;
        dst.posEnd = src.posEnd;
        dst.posCurrent = src.posCurrent;
        dst.step = src.step;
        dst.time = src.time;
        dst.phase = src.phase;
        dst.channel = src.channel;
        dst.icon = src.icon;
    }
}
