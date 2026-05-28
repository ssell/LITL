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

    /// <summary>
    /// Is the vertex binding stepped at a per-vertex or per-instance rate?
    /// </summary>
    enum class VertexInputRate
    {
        PerVertex = 0,
        PerInstance = 1
    };

    /// <summary>
    /// Defines how the vertex data is structured at the buffer level.
    /// </summary>
    struct VertexBinding
    {
        /// <summary>
        /// An integer identifier mapping to a specific vertex buffer slot.
        /// </summary>
        uint32_t binding;

        /// <summary>
        /// The byte size separating one entire vertex from the next in the buffer.
        /// </summary>
        uint32_t stride;

        /// <summary>
        /// Defines whether the data steps forward per-vertex or per-instance.
        /// </summary>
        VertexInputRate rate;
    };

    /// <summary>
    /// Defines how the GPU should interpret the data inside of the vertex buffers.
    /// Examples include: position, normal, UV, color, etc.
    /// </summary>
    struct VertexAttribute
    {
        /// <summary>
        /// The integer location tied directly to a variable in your vertex shader.
        /// </summary>
        uint32_t location;

        /// <summary>
        /// Which buffer binding this attribute belongs to.
        /// </summary>
        uint32_t binding;

        /// <summary>
        /// The data type and number of components.
        /// For example, DataFormat::RGBA32_SFloat for a vec4.
        /// </summary>
        DataFormat format;

        /// <summary>
        /// The byte offset from the start of the vertex where this specific attribute's data begins.
        /// </summary>
        uint32_t offset = 0;
    };

    /// <summary>
    /// Dictates how raw vertex data is pulled from memory and passed into the vertex shader.
    /// It bridges the gap between CPU-allocated vertex buffers and GPU shader inputs.
    /// </summary>
    struct VertexInputState
    {
        std::vector<VertexBinding> bindings;
        std::vector<VertexAttribute> attributes;
    };

    struct InputAssemblyState
    {
        /// <summary>
        /// The topology of the generated primitives.
        /// </summary>
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;

        /// <summary>
        /// Controls whether a special vertex index value is treated as restarting the assembly of primitives. 
        /// This enable only applies to indexed draws.
        /// </summary>
        bool primitiveRestartEnabled = false;
    };

    struct TessellationState
    {
        /// <summary>
        /// The number of control points per patch.
        /// </summary>
        uint32_t patchControlPoints = 0;
    };

    struct RasterizationState
    {
        /// <summary>
        /// When enabled, clamps the z value of fragments between the min and max depth bounds 
        /// instead of clipping them, preserving shadow mapping precision.
        /// </summary>
        bool depthClampEnabled = false;

        /// <summary>
        /// If enabled, primitives are immediately discarded before rasterization. 
        /// This is useful when running stream-out setups or only tracking side-effects from vertex buffers.
        /// </summary>
        bool rasterizerDiscardEnabled = false;

        /// <summary>
        /// Determines how polygons are drawn.
        /// </summary>
        PolygonMode polygonMode = PolygonMode::Fill;

        /// <summary>
        /// Specifies which primitives are culled to save performance on invisible geometry.
        /// </summary>
        CullMode cullMode = CullMode::Back;

        // todo front face
        // todo depth bias
        // todo line width
    };

    struct MultisampleState
    {

    };

    struct DepthStencilState
    {

    };

    struct ColorBlendState
    {

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

        VertexInputState vertexInput;
        InputAssemblyState inputAssembly;
        std::optional<TessellationState> tessellation;


        // ? ViewportState

        RasterizationState rasterization;
        MultisampleState multisample;
        DepthStencilState depthStencil;
        ColorBlendState colorBlend;

        /* 
        todo:
        

        DynamicStateMask dynamicState;

        SpecializationValues specialization;
        */
    };

    struct GraphicsPipelineTag {};
    using GraphicsPipelineHandle = Handle<GraphicsPipelineTag>;
}

#endif