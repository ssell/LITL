#ifndef LITL_MATH_RECT2D_H__
#define LITL_MATH_RECT2D_H__

#include "litl-core/types.hpp"
#include "litl-core/math/types/vec2.hpp"

namespace litl
{
    struct rect2D
    {
        vec2 offset{};
        vec2 extents{};
    };

    static_assert(std::is_nothrow_copy_constructible_v<rect2D>);
    static_assert(std::is_nothrow_move_constructible_v<rect2D>);
    static_assert(std::is_nothrow_copy_assignable_v<rect2D>);
    static_assert(std::is_nothrow_move_assignable_v<rect2D>);
}

LITL_REGISTER_TYPE_NAME(litl::rect2D)

#endif