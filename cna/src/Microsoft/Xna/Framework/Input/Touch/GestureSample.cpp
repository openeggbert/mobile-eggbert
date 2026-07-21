// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Input/Touch/GestureSample.hpp"

namespace Microsoft::Xna::Framework::Input::Touch
{
    static constexpr int NO_FINGER = -1;

    GestureSample::GestureSample()
        : gestureType_(GestureType::None),
          timestamp_(System::TimeSpan::Zero),
          position_(Microsoft::Xna::Framework::Vector2::Zero),
          position2_(Microsoft::Xna::Framework::Vector2::Zero),
          delta_(Microsoft::Xna::Framework::Vector2::Zero),
          delta2_(Microsoft::Xna::Framework::Vector2::Zero),
          fingerIdEXT_(NO_FINGER),
          fingerId2EXT_(NO_FINGER)
    {
    }

    GestureSample::GestureSample(GestureType gestureType,
                                 System::TimeSpan timestamp,
                                 Microsoft::Xna::Framework::Vector2 position,
                                 Microsoft::Xna::Framework::Vector2 position2,
                                 Microsoft::Xna::Framework::Vector2 delta,
                                 Microsoft::Xna::Framework::Vector2 delta2)
        : gestureType_(gestureType),
          timestamp_(timestamp),
          position_(position),
          position2_(position2),
          delta_(delta),
          delta2_(delta2),
          fingerIdEXT_(NO_FINGER),
          fingerId2EXT_(NO_FINGER)
    {
    }

    GestureSample::GestureSample(GestureType gestureType,
                                 System::TimeSpan timestamp,
                                 Microsoft::Xna::Framework::Vector2 position,
                                 Microsoft::Xna::Framework::Vector2 position2,
                                 Microsoft::Xna::Framework::Vector2 delta,
                                 Microsoft::Xna::Framework::Vector2 delta2,
                                 int fingerId,
                                 int fingerId2)
        : gestureType_(gestureType),
          timestamp_(timestamp),
          position_(position),
          position2_(position2),
          delta_(delta),
          delta2_(delta2),
          fingerIdEXT_(fingerId),
          fingerId2EXT_(fingerId2)
    {
    }

}
