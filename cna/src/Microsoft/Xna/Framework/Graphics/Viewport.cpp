// SPDX-License-Identifier: MS-PL
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    IMPL_PROP(int, Height,     getter1, setter1, member0, static0, constret1, ref1, constmet1, Viewport, nothing)
    IMPL_PROP(float, MaxDepth, getter1, setter1, member0, static0, constret1, ref1, constmet1, Viewport, nothing)
    IMPL_PROP(float, MinDepth, getter1, setter1, member0, static0, constret1, ref1, constmet1, Viewport, nothing)
    IMPL_PROP(int, Width,      getter1, setter1, member0, static0, constret1, ref1, constmet1, Viewport, nothing)
    IMPL_PROP(int, Y,         getter1, setter1, member0, static0, constret1, ref1, constmet1, Viewport, nothing)
    IMPL_PROP(int, X,         getter1, setter1, member0, static0, constret1, ref1, constmet1, Viewport, nothing)

    Viewport::Viewport()
        : Height_(0), MaxDepth_(1.0f), MinDepth_(0.0f), Width_(0), Y_(0), X_(0)
    {
    }

    Viewport::Viewport(int x_, int y_, int width, int height)
        : Height_(height), MaxDepth_(1.0f), MinDepth_(0.0f), Width_(width), Y_(y_), X_(x_)
    {
    }
}
