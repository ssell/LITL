#ifndef LITL_IMPORT_MODEL_INTERMEDIATE_MODEL_H__
#define LITL_IMPORT_MODEL_INTERMEDIATE_MODEL_H__

#include <cstdint>
#include <string>
#include <vector>

#include "litl-core/math.hpp"

namespace litl::import
{
    struct Node
    {
        std::string name;
        mat4 localTransform{ mat4::identity() };
        uint32_t meshIndex{ Constants::uint32_null_index };
        uint32_t materialIndex{ Constants::uint32_null_index };     // ... todo this needs to be a vector as you can have many materials associated with a single mesh on a node ...
        std::vector<uint32_t> children;
    };

    struct Model
    {
        std::string name;
        std::vector<std::string> meshNames;
        std::vector<std::string> materialNames;
        std::vector<Node> nodes;
        std::vector<uint32_t> rootNodes;
    };
}

#endif