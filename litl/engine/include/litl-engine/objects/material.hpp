#ifndef LITL_ENGINE_MATERIAL_H__
#define LITL_ENGINE_MATERIAL_H__

#include <optional>
#include <string>

#include "litl-core/authority.hpp"
#include "litl-core/math/geometry/vertex.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer/resources/computePipeline.hpp"

namespace litl
{
    class ObjectPool;
    class Renderer;

    struct VertexInputDescriptor
    {
        static constexpr uint32_t MaxVertexAttributes = 8u;

        /// <summary>
        /// Size of the vertex structure.
        /// </summary>
        uint32_t vertexSize = sizeof(Vertex);

        /// <summary>
        /// The data format of each field of the vertex structure.
        /// </summary>
        std::array<DataFormat, MaxVertexAttributes> attributes = {
            DataFormat::RGB32_SFloat,                   // position
            DataFormat::RG32_SFloat,                    // texcoord
            DataFormat::RGB32_SFloat,                   // normal
            DataFormat::RGBA32_SFloat                   // tangent
        };
    };

    struct ShaderResourceDescriptor
    {
        std::string resource{};
        std::string entryPoint{};
        std::span<std::byte const> bytes{};
    };

    struct MaterialDescriptor
    {
        ObjectDescriptor objectInfo{};
        VertexInputDescriptor inputDescriptor{};
        ShaderResourceDescriptor vertexShader{};
        ShaderResourceDescriptor fragmentShader{};
        ShaderResourceDescriptor geometryShader{};
        ShaderResourceDescriptor tessellationControlShader{};
        ShaderResourceDescriptor tessellationEvaluationShader{};
        ShaderResourceDescriptor computeShader{};
        ShaderResourceDescriptor meshShader{};
        ShaderResourceDescriptor taskShader{};
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