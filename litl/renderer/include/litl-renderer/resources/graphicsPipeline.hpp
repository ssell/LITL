#ifndef LITL_RENDERER_GRAPHICS_PIPELINE_H__
#define LITL_RENDERER_GRAPHICS_PIPELINE_H__

#include <array>

#include "litl-core/handles.hpp"
#include "litl-renderer/resources/shaderModule.hpp"

namespace litl
{
    struct GraphicsPipelineShaderDescriptor
    {
        ShaderModuleHandle handle{};
        ShaderStage stage;
        std::string entryPoint;
    };

    /// <summary>
    /// Maps to VkPipelineRenderingCreateInfo
    /// </summary>
    struct RenderTargetFormats
    {
        /// <summary>
        /// Maximum number of color attachments on a pipeline.
        /// For most desktop machines this is 8.
        /// </summary>
        static constexpr uint32_t MaxColorAttachments = 8;

        /// <summary>
        /// Format of the color attachments used in the pipeline.
        /// </summary>
        std::array<ImageFormat, MaxColorAttachments> colorAttachments;

        /// <summary>
        /// Number of entries in colorAttachments.
        /// </summary>
        uint32_t colorAttachmentCount;

        /// <summary>
        /// A bitfield of view indices describing which views are active during rendering
        /// </summary>
        uint32_t viewMask;
    };

    struct GraphicsPipelineDescriptor
    {
        GraphicsPipelineShaderDescriptor vertex;
        GraphicsPipelineShaderDescriptor fragment;
        GraphicsPipelineShaderDescriptor geometry;
        GraphicsPipelineShaderDescriptor tessellationEvaluation;
        GraphicsPipelineShaderDescriptor tessellationControl;
        GraphicsPipelineShaderDescriptor mesh;
        GraphicsPipelineShaderDescriptor task;

        RenderTargetFormats renderTargets;

        /* 
        todo:
        
        VertexInputState vertexInput;
        InputAssemblyState inputAssembly;
        std::optional<TessellationState> tessellation;

        RasterizationState rasterization;
        MultisampleState multisample;
        DepthStencilState depthStencil;
        ColorBlendState colorBlend;

        DynamicStateMask dynamicState;

        SpecializationValues specialization;
        */
    };

    struct GraphicsPipelineTag {};
    using GraphicsPipelineHandle = Handle<GraphicsPipelineTag>;
}

#endif