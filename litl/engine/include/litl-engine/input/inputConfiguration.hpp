#ifndef LITL_ENGINE_INPUT_CONFIGURATION_H__
#define LITL_ENGINE_INPUT_CONFIGURATION_H__

#include <vector>

#include "litl-platform/inputEnums.hpp"
#include "litl-engine/input/actionMapDescriptor.hpp"

namespace litl
{
    struct InputConfiguration
    {
        CursorMode defaultCursorMode{ CursorMode::Normal };
        std::vector<ActionMapDescriptor> actionMaps;

        void sanitize() noexcept;
    };
}

#endif