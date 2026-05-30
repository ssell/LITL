#ifndef LITL_RENDERER_GRAPHICS_PIPELINE_H__
#define LITL_RENDERER_GRAPHICS_PIPELINE_H__

#include <array>
#include <optional>

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
        std::array<DataFormat, MaxColorAttachments> colorAttachments;

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

        /// <summary>
        /// The winding order the interprets a polygon front-facing orientation.
        /// </summary>
        FrontFace frontFace = FrontFace::Clockwise;

        /// <summary>
        /// Controls whether to bias fragment depth values.
        /// </summary>
        bool depthBiasEnabled = false;

        /// <summary>
        /// A scalar factor controlling the constant depth value added to each fragment.
        /// </summary>
        float depthBiasConstantFactor = 0.0f;

        /// <summary>
        /// The maximum (or minimum) depth bias of a fragment.
        /// </summary>
        float depthBiasClamp = 0.0f;

        /// <summary>
        /// A scalar factor applied to a fragment’s slope in depth bias calculations.
        /// </summary>
        float depthBiasSlopeFactor = 0.0f;

        /// <summary>
        /// The width of rasterized line segments.
        /// </summary>
        float lineWidth = 1.0f;
    };

    struct MultisampleState
    {
        /// <summary>
        /// Sets the number of samples used during rasterization.
        /// 
        /// A count of 1 indicates no multisampling (single sample) while 
        /// increasing values indicate increased sample counts.
        /// </summary>
        MultisampleCount sampleCount = MultisampleCount::Count1;

        /// <summary>
        /// Enables sample shading, which runs the fragment shader per-sample 
        /// rather than per-pixel to reduce texture aliasing.
        /// </summary>
        bool sampleShadingEnabled = false;

        /// <summary>
        /// Specifies the minimum fraction of sample shading when sample shading is enabled.
        /// A value of 1.0 evaluates every sample.
        /// </summary>
        float minSampleShading = 0.0f;

        // Note: no support for sample mask yet.

        /// <summary>
        /// Converts the alpha component of the first fragment color output into a temporary 
        /// coverage mask (useful for rendering transparent foliage).
        /// </summary>
        bool alphaToCoverageEnabled = false;

        /// <summary>
        /// Replaces the alpha component of the fragment's first color output with 1.0.
        /// </summary>
        bool alphaToOneEnabled = false;
    };

    struct DepthState
    {
        /// <summary>
        /// Enables or disables checking whether a fragment's depth is closer than the existing depth in the depth buffer.
        /// </summary>
        bool depthTestEnabled = true;

        /// <summary>
        /// When enabled, the fragment's depth is written into the depth buffer upon passing the test. 
        /// Typically disabled for transparent objects.
        /// </summary>
        bool depthWriteEnabled = true;

        /// <summary>
        /// The comparison operator used for the depth test.
        /// Remember: LITL uses a reversed depth buffer by default to avoid distance artifacts. So 0 = far and 1 = near.
        /// </summary>
        CompareOperationType compareOp = CompareOperationType::Greater;

        /// <summary>
        /// Controls whether to discard fragments outside a specific minimum and maximum depth range.
        /// </summary>
        bool depthBoundsTestEnabled = false;

        /// <summary>
        /// The minimum depth bound used in the depth bounds test.
        /// </summary>
        float minDepthBounds = 0.0f;

        /// <summary>
        /// The maximum depth bound used in the depth bounds test.
        /// </summary>
        float maxDepthBounds = 1.0f;
    };

    struct StencilFaceState
    {
        /// <summary>
        /// Action performed on samples that fail the stencil test.
        /// </summary>
        StencilOperationType failOp = StencilOperationType::Keep;

        /// <summary>
        /// Action performed on samples that pass both the depth and stencil tests.
        /// </summary>
        StencilOperationType passOp = StencilOperationType::Keep;

        /// <summary>
        /// Action performed on samples that pass the stencil test and fail the depth test.
        /// </summary>
        StencilOperationType depthFailOp = StencilOperationType::Keep;

        /// <summary>
        /// Comparison operator used in the stencil test.
        /// </summary>
        CompareOperationType compareOp = CompareOperationType::Always;

        /// <summary>
        /// Selects the bits of the unsigned integer stencil values participating in the stencil test.
        /// </summary>
        uint32_t compareMask = 0xFFFFFFFF;

        /// <summary>
        /// Selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment.
        /// </summary>
        uint32_t writeMask = 0xFFFFFFFF;

        /// <summary>
        /// An integer stencil reference value that is used in the unsigned stencil comparison.
        /// </summary>
        uint32_t reference = 0x00000000;
    };

    struct StencilState
    {
        /// <summary>
        /// Enables or disables stencil testing.
        /// </summary>
        bool stencilTestEnabled = false;

        /// <summary>
        /// Controls front stencil test.
        /// </summary>
        StencilFaceState frontStencilState{};

        /// <summary>
        /// Controls back stencil test.
        /// </summary>
        StencilFaceState backStencilState{};
    };

    struct DepthStencilState
    {
        DepthState depthState{};
        StencilState stencilState{};
    };

    struct ColorBlendAttachmentState
    {
        /// <summary>
        /// Controls whether blending is enabled for the corresponding color attachment. 
        /// If blending is not enabled, the source fragment’s color for that attachment is passed through unmodified.
        /// </summary>
        bool attachmentBlendEnabled = false;

        /// <summary>
        /// Selects which blend factor is used to determine the destination factors (Sr,Sg,Sb).
        /// </summary>
        BlendFactor srcColorBlendFactor = BlendFactor::Zero;

        /// <summary>
        /// Selects which blend factor is used to determine the destination factors (Dr,Dg,Db).
        /// </summary>
        BlendFactor dstColorBlendFactor = BlendFactor::Zero;

        /// <summary>
        /// Selects which blend operation is used to calculate the RGB values to write to the color attachment.
        /// </summary>
        BlendOperationType colorBlendOp = BlendOperationType::Add;

        /// <summary>
        /// Selects which blend factor is used to determine the source factor Sa.
        /// </summary>
        BlendFactor srcAlphaBlendFactor = BlendFactor::Zero;

        /// <summary>
        /// Selects which blend factor is used to determine the destination factor Da.
        /// </summary>
        BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;

        /// <summary>
        /// Selects which blend operation is used to calculate the alpha values to write to the color attachment.
        /// </summary>
        BlendOperationType alphaBlendOp = BlendOperationType::Add;

        /// <summary>
        /// A bitmask specifying which of R, G, B, and/or A components are enabled for writing.
        /// </summary>
        ColorComponentFlag colorWriteMask = static_cast<ColorComponentFlag>(ColorComponentFlagBits::RGBA);
    };

    struct ColorBlendState
    {
        /// <summary>
        /// Enable a logical operation between the fragment's color values and the existing value in the framebuffer attachment.
        /// </summary>
        bool logicOpEnabled = false;
        
        /// <summary>
        /// Selects which logical operation to apply.
        /// </summary>
        LogicOperationType logicOp = LogicOperationType::Clear;

        /// <summary>
        /// Configures per-target color and alpha blending behavior for an individual framebuffer color attachments.
        /// </summary>
        std::vector<ColorBlendAttachmentState> colorAttachmentBlendState;
    };

    /// <summary>
    /// Indicate which dynamic state is taken from dynamic state commands.
    /// 
    /// Note some states such as viewport region, scissor region, etc. are always dynamic
    /// and thus do not have a corresponding DynamicState entry.
    /// </summary>
    struct DynamicStateMask
    {
        std::vector<DynamicState> states;
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

        RasterizationState rasterization;
        MultisampleState multisample;
        DepthStencilState depthStencil;
        ColorBlendState colorBlend;
        DynamicStateMask dynamicState;
    };

    struct GraphicsPipelineTag {};
    using GraphicsPipelineHandle = Handle<GraphicsPipelineTag>;
}

#endif