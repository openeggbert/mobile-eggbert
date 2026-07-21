// SPDX-License-Identifier: MS-PL
#pragma once

#include "SharpRuntime/Prop.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    /** @brief Describes the view bounds for the render-target surface. */
    class Viewport
    {
    private:
        int Height_;
        float MaxDepth_;
        float MinDepth_;
        int Width_;
        int Y_;
        int X_;

    public:
        DEF_PROP(int, Height,   getter1, setter1, member0, static0, constret1, ref1, constmet1)
        DEF_PROP(float, MaxDepth, getter1, setter1, member0, static0, constret1, ref1, constmet1)
        DEF_PROP(float, MinDepth, getter1, setter1, member0, static0, constret1, ref1, constmet1)
        DEF_PROP(int, Width,    getter1, setter1, member0, static0, constret1, ref1, constmet1)
        DEF_PROP(int, Y,        getter1, setter1, member0, static0, constret1, ref1, constmet1)
        DEF_PROP(int, X,        getter1, setter1, member0, static0, constret1, ref1, constmet1)

        /** @brief Constructs an empty viewport with all fields set to zero. */
        Viewport();

        /**
         * @brief Constructs a viewport from a position and size; MinDepth=0, MaxDepth=1.
         *
         * @param x      X coordinate of the upper-left corner in pixels.
         * @param y      Y coordinate of the upper-left corner in pixels.
         * @param width  Width of the viewport in pixels.
         * @param height Height of the viewport in pixels.
         */
        Viewport(int x, int y, int width, int height);
    };
}
