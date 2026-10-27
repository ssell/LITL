#ifndef LITL_ENGINE_RENDER_PASS_ITEM_H__
#define LITL_ENGINE_RENDER_PASS_ITEM_H__

#include <cstdint>
#include "litl-renderer/resources/handles.hpp"

namespace litl
{
    struct Material;
    struct Mesh;

    /// <summary>
    /// Item being drawn by a render pass, and the number of times it is being drawn.
    /// </summary>
    struct RenderPassItem
    {
        MaterialHandle materialHandle{};
        Material* material{ nullptr };
        GraphicsPipelineHandle graphicsPipelineHandle{};
        MeshHandle meshHandle{};
        Mesh* mesh{};
        uint32_t firstVertex = 0u;
        uint32_t vertexCount = 0u;
        uint32_t firstIndex = 0u;
        uint32_t indexCount = 0u;
        uint32_t instanceCount = 0u;
        uint32_t instanceOffset = 0u;
    };
}

#endif