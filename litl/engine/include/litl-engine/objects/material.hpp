#ifndef LITL_ENGINE_MATERIAL_H__
#define LITL_ENGINE_MATERIAL_H__

#include <optional>
#include <string>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer/resources/computePipeline.hpp"

namespace litl
{
    class ObjectPool;
    class Renderer;

    struct ShaderResourceDescriptor
    {
        std::string resource{};
        std::string entryPoint{};
        std::span<std::byte const> bytes{};
    };

    struct MaterialDescriptor : ObjectDescriptor
    {
        ShaderResourceDescriptor vertexShader;
        ShaderResourceDescriptor fragmentShader;
        ShaderResourceDescriptor geometryShader;
        ShaderResourceDescriptor tessellationControlShader;
        ShaderResourceDescriptor tessellationEvaluationShader;
    };

    class Material
    {
    public:

        bool create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, Renderer const& renderer) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

        [[nodiscard]] GraphicsPipelineHandle getGraphicsPipelineHandle() const noexcept;
        [[nodiscard]] ComputePipelineHandle getComputePipelineHandle() const noexcept;

    private:

        [[nodiscard]] ShaderModuleHandle createShaderModuleHandle(ShaderResourceDescriptor& descriptor) const noexcept;

        Renderer const* m_pRenderer = nullptr;
        MaterialDescriptor m_descriptor;
        GraphicsPipelineHandle m_graphicsPipelineHandle{};
        ComputePipelineHandle m_computePipelineHandle{};

    };
}

#endif