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
        std::array<float, 16> localTransform{ mat4::identity().toArray() };     // Use array instead of mat4 directly for (de)serialization
        uint32_t meshIndex{ Constants::uint32_null_index };
        std::vector<uint32_t> materialIndices;
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