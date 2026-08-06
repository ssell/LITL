#ifndef LITL_IMPORT_MESH_NODE_H__
#define LITL_IMPORT_MESH_NODE_H__

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "litl-core/math.hpp"

namespace litl::import
{
    struct Node
    {
        std::string name;
        mat4 localTransform;
        std::optional<uint32_t> meshIndex;
        std::vector<uint32_t> children;
    };
}

#endif