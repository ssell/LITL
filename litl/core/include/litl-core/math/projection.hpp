#ifndef LITL_MATH_PROJECTION_H__
#define LITL_MATH_PROJECTION_H__

#include "litl-core/math/types.hpp"

namespace litl
{
    [[nodiscard]] Ray screenToWorldSpace(vec2 screenPos, vec2 screenSize, mat4 const& viewMatrix, mat4 const& projMatrix) noexcept;
    [[nodiscard]] vec2 screenToNdcSpace(vec2 screenPos, vec2 screenSize) noexcept;
}

#endif