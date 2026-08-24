#ifndef LITL_ENGINE_MATERIAL_H__
#define LITL_ENGINE_MATERIAL_H__

#include <optional>
#include <string>

#include "litl-core/authority.hpp"
#include "litl-core/math/geometry/vertex.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPipeline.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer/resources/computePipeline.hpp"

namespace litl
{
    struct MaterialDescriptor
    {
        ObjectDescriptor objectInfo{};
        MaterialPipelineDescriptor pipelineDescriptor{};
    };

    class Material
    {
    public:

        bool create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, Renderer const& renderer, ObjectPool& objectPool) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

        [[nodiscard]] MaterialPipelineHandle getMaterialPipelineHandle() const noexcept;

    private:

        Renderer const* m_pRenderer = nullptr;
        MaterialPipelineHandle m_materialPipelineHandle{};

    };
}

#endif