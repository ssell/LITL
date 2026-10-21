#ifndef LITL_ENGINE_INPUT_CONFIGURATION_H__
#define LITL_ENGINE_INPUT_CONFIGURATION_H__

#include <string>
#include <vector>

#include "litl-platform/inputEnums.hpp"
#include "litl-engine/input/actionMapDescriptor.hpp"

namespace litl
{
    struct InputConfiguration
    {
        CursorMode defaultCursorMode{ CursorMode::Normal };
        std::vector<ActionMapDescriptor> actionMaps;
        std::string defaultActionMap;

        void sanitize() noexcept;
    };
}

#endif