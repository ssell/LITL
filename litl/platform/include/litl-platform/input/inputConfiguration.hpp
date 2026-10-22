#ifndef LITL_PLATFORM_INPUT_CONFIGURATION_H__
#define LITL_PLATFORM_INPUT_CONFIGURATION_H__

#include <string>
#include <vector>

#include "litl-platform/input/inputEnums.hpp"
#include "litl-platform/input/actionMapDescriptor.hpp"

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