#ifndef LITL_RENDERER_GRAPHICS_PIPELINE_H__
#define LITL_RENDERER_GRAPHICS_PIPELINE_H__

#include <array>

#include "litl-core/handles.hpp"
#include "litl-renderer/resources/shaderModule.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    /// <summary>
    /// The shader module and entry point into that module.
    /// The specified stage must match the reflected stage of the entry point.
    /// </summary>
    struct GraphicsPipelineShaderDescriptor
    {
        ShaderModuleHandle handle{};
        ShaderStage stage;
        std::string entryPoint;
    };

    /// <summary>
    /// A graphics pipeline is compatible with any render target whose formats match.
    /// 
    /// As the underlying Vulkan renderer uses dynamic rendering, the pipeline does not need
    /// to name the textures it render to, that is instead set with Renderer::cmdBeginRender.
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