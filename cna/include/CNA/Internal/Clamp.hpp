// SPDX-License-Identifier: MS-PL
#pragma once

namespace CNA::Internal
{
    /**
     * @brief Clamps @p v to the closed range [@p lo, @p hi].
     *
     * C++14-compatible replacement for std::clamp (C++17). Internal implementation utility,
     * not part of the XNA API surface -- callers needing XNA's own clamping semantics should
     * use Microsoft::Xna::Framework::MathHelper::Clamp() instead.
     *
     * @param[in] v  Value to clamp.
     * @param[in] lo Lower bound.
     * @param[in] hi Upper bound.
     * @return @p lo if @p v is less than @p lo; @p hi if @p hi is less than @p v; otherwise @p v.
     */
    template <typename T>
    constexpr const T& Clamp(const T& v, const T& lo, const T& hi)
    {
        return v < lo ? lo : (hi < v ? hi : v);
    }
}
