#ifndef LITL_RENDERER_COMMANDS_CLEAR_IMAGE_H__
#define LITL_RENDERER_COMMANDS_CLEAR_IMAGE_H__

#include "litl-core/math/types/color.hpp"

namespace litl
{
    struct ClearImageCommand
    {
        color clearColor;
    };
}

#endif