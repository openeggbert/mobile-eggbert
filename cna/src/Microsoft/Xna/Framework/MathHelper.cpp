// SPDX-License-Identifier: MS-PL

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <cmath>

namespace Microsoft::Xna::Framework
{
    const float MathHelper::MachineEpsilonFloat = MathHelper::GetMachineEpsilonFloat();

    float MathHelper::Clamp(float value, float min, float max)
    {
        value = (value > max) ? max : value;
        value = (value < min) ? min : value;
        return value;
    }

    bool MathHelper::WithinEpsilon(float floatA, float floatB)
    {
        return std::fabs(floatA - floatB) < MachineEpsilonFloat;
    }

    float MathHelper::GetMachineEpsilonFloat()
    {
        float machineEpsilon = 1.0f;
        float comparison;

        /* Keep halving the working value of machineEpsilon until we get a number that
         * when added to 1.0f will still evaluate as equal to 1.0f.
         */
        do
        {
            machineEpsilon *= 0.5f;
            comparison = 1.0f + machineEpsilon;
        }
        while (comparison > 1.0f);

        return machineEpsilon;
    }
}
