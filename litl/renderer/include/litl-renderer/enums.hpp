#ifndef LITL_RENDERER_ENUMS_H__
#define LITL_RENDERER_ENUMS_H__

#include <cstdint>
#include <type_traits>

#include "litl-core/enumBitFlags.hpp"

namespace litl
{
    // Based on: https://docs.vulkan.org/refpages/latest/refpages/source/VkImageLayout.html
    enum class ImageLayoutType : uint32_t
    {
        Undefined            = 0,
        General              = 1,
        Color                = 2,
        DepthStencil         = 3,
        DepthStencilReadOnly = 4,
        ShaderReadOnly       = 5,
        TransferSrc          = 6,
        TransferDst          = 7,
        Preinitialized       = 8,
        Present              = 9
    };

    // Based on: https://docs.vulkan.org/refpages/latest/refpages/source/VkAccessFlagBits2.html
    enum class ImageAccessFlagBits : uint64_t
    {
        None                = 0,
        IndirectCommandRead = 1 << 0,
        IndexRead           = 1 << 1,
        VertexAttributeRead = 1 << 2,
        UniformRead         = 1 << 3,
        InputAttachmentRead = 1 << 4,
        ShaderRead          = 1 << 5,
        ShaderWrite         = 1 << 6,
        ColorRead           = 1 << 7,
        ColorWrite          = 1 << 8,
        DepthStencilRead    = 1 << 9,
        DepthStencilWrite   = 1 << 10,
        TransferRead        = 1 << 11,
        TransferWrite       = 1 << 12,
        HostRead            = 1 << 13,
        HostWrite           = 1 << 14,
        MemoryRead          = 1 << 15,
        MemoryWrite         = 1 << 16,
        ShaderSampledRead   = 1 << 17,
        ShaderStorageRead   = 1 << 18,
        ShaderStorageWrite  = 1 << 19
    };

    /// <summary>
    /// Composed of ImageAccessFlagBits
    /// </summary>
    using ImageAccessFlag = uint64_t;

    enum class BufferTypeFlagBits : uint64_t
    {
        None                = 0ull,
        VertexBuffer        = 1ull << 0,       // Used as a vertex buffer.
        IndexBuffer         = 1ull << 1,       // Used as an index buffer.
        UniformBuffer       = 1ull << 2,       // Referenced through an UBO descriptor.
        StorageBuffer       = 1ull << 3,       // Referenced through a SSBO descriptor.
        TransferSource      = 1ull << 4,       // Used as the source of a copy buffer command.
        TransferDest        = 1ull << 5,       // Used as the destination of a copy buffer command.
        ShaderDeviceAddress = 1ull << 6,       // Can be accessed via a 64-bit point in shaders (Buffer Device Address (BDA))
    };

    LITL_ENABLE_BITMASK(BufferTypeFlagBits);

    /// <summary>
    /// Composed of BufferTypeFlagBits
    /// </summary>
    using BufferTypeFlag = BufferTypeFlagBits;

    enum class BufferMemoryType : uint32_t
    {
        Unknown = 0,
        Auto = 1,               // Recommended for most common use cases. Picks memory usage based on the declared buffer usage target.
        PreferGpu = 2,                // Selects best memory type automatically with preference for GPU (device) memory. Staging buffer required for updates.
        PreferCpu = 3                 // Selects best memory type automatically with preference for CPU (host) memory. Slower GPU access but does not require a staging buffer for updates.
    };

    // Based on https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineStageFlagBits2.html
    enum class PipelineStageFlagBits : uint64_t
    {
        None                         = 0,
        TopOfPipe                    = 1 << 0,
        DrawIndirect                 = 1 << 1,
        VertexInput                  = 1 << 2,
        VertexShader                 = 1 << 3,
        TessellationControlShader    = 1 << 4,
        TessellationEvaluationShader = 1 << 5,
        GeometryShader               = 1 << 6,
        FragmentShader               = 1 << 7,
        EarlyFragmentTests           = 1 << 8,
        LateFragmentTests            = 1 << 9,
        ColorAttachmentOutput        = 1 << 10,
        ComputeShader                = 1 << 11,
        Transfer                     = 1 << 12,
        BottomOfPipe                 = 1 << 13,
        Host                         = 1 << 14,
        AllGraphics                  = 1 << 15,
        AllCommands                  = 1 << 16,
        Copy                         = 1 << 17,
        Resolve                      = 1 << 18,
        Blit                         = 1 << 19,
        Clear                        = 1 << 20,
        IndexInput                   = 1 << 21,
        VertexAttributeInput         = 1 << 22,
        PreRasterizationShaders      = 1 << 23
    };

    LITL_ENABLE_BITMASK(PipelineStageFlagBits);

    /// <summary>
    /// Composed of PipelineStageFlagBits
    /// </summary>
    using PipelineStageFlag = PipelineStageFlagBits;

    enum class ColorComponentFlagBits : uint32_t
    {
        None = 0b0000,
        R    = 0b0001,
        G    = 0b0010,
        B    = 0b0100,
        A    = 0b1000,
        RGB  = 0b0111,
        RGBA = 0b1111
    };

    /// <summary>
    /// Composed of ColorComponentFlagBits
    /// </summary>
    using ColorComponentFlag = uint32_t;

    /// <summary>
    /// Based on: https://docs.vulkan.org/refpages/latest/refpages/source/VkAttachmentLoadOp.html
    /// </summary>
    enum class LoadOperationType : uint32_t
    {
        None     = 0,
        Load     = 1,
        Clear    = 2,
        DontCare = 3
    };

    /// <summary>
    /// Based on: https://docs.vulkan.org/refpages/latest/refpages/source/VkAttachmentStoreOp.html
    /// </summary>
    enum class StoreOperationType : uint32_t
    {
        None     = 0,
        Store    = 1,
        DontCare = 2
    };

    enum class DataFormat : uint32_t
    {
        Undefined = 0,

        // Color LDR
        RGBA8_UNorm,            // 32-bit RGBA color where each channel gets a 8-bit linear floating point value on the range [0,1].
        RGBA8_SRGB,             // 32-bit RGBA color where each channel gets a 8-bit non-linear (gamma corrected) float point values on the range [0,1].
        BGRA8_Unorm,            // 32-bit BGRA color where each channel gets a 8-bit linear floating point value on the range [0,1].
        BGRA8_SRGB,             // 32-bit BGRA color where each channel gets a 8-bit non-linear (gamma corrected) float point values on the range [0,1].
        ABGR10_UNorm_Pack32,    // 10-bit color, sometimes preferred for HDR-ish output or higher-precision G-buffers.

        // HDR
        RGBA16_SFloat,          // 64-bit RGBA color where each channel gets a 16-bit floating point value.
        RGB32_SFloat,           // 96-bit RGB color where each channel gets a 32-bit floating point value.
        RGBA32_SFloat,          // 128-bit RGBA color where each channel gets a 32-bit floating point value.
        R11G11B10_UFloat,       // 32-bit RGB color where R and G gets a 11-bit and B gets a 10-bit unsigned floating point value.

        // Single Channel
        R8_UNorm,               // Single 8-bit linear red channel on the range [0,1].
        R16_SFloat,             // Single 16-bit floating point value in the red channel.
        R32_SFloat,             // Single 32-bit floating point value in the red channel.

        // Dual Channel
        RG8_UNorm,              // Double channel 8-bit red and green on the range [0,1].
        RG16_SFloat,            // Double channel 16-bit red and green floating point values.

        // Depth
        D32_SFloat,             // 32-bit depth floating point value.
        D24_UNorm_S8_UInt,      // 32-bit format with a 24-bit linear depth value on the range [0,1] and 8-bit unsigned integer stencil.
        D24_SFloat_S8_Uint,     // 32-bit format with a 24-bit floating point depth value and 8-bit unsigned integer stencil.

        // Compressed
        BC7_UNorm,              // A four-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data.
        BC7_SRGB,               // A four-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RGBA texel data with sRGB nonlinear encoding applied to the RGB components.
        BC4_UNorm,              // A one-component, block-compressed format where each 64-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized red texel data.
        BC5_UNorm,              // A two-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned normalized RG texel data with the first 64 bits encoding red values followed by 64 bits encoding green values.
        BC6H_UFloat             // A  three-component, block-compressed format where each 128-bit compressed texel block encodes a 4×4 rectangle of unsigned floating-point RGB texel data.
    };

    enum class PrimitiveTopology : uint32_t
    {
        PointList = 0,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan
    };

    enum class PolygonMode : uint32_t
    {
        Fill = 0,
        Line,
        Point
    };

    enum class CullMode : uint32_t
    {
        None = 0,               // No triangles discarded
        Front,                  // Front-facing triangles discarded
        Back,                   // Back-facing triangles discarded
        Both                    // All triangles discarded
    };
    
    enum class FrontFace : uint32_t
    {
        CounterClockwise = 0,
        Clockwise = 1
    };

    enum class MultisampleCount : uint32_t
    {
        Count1 = 0,             // No multisampling
        Count2,
        Count4,
        Count8,
        Count16,
        Count32,
        Count64
    };

    enum class CompareOperationType
    {
        Never = 0,              // Comparison always evaluates false.
        Less,                   // Comparison evaluates reference < test.
        Equal,                  // Comparison evaluates reference = test.
        NotEqual,               // Comparison evaluates reference ≠ test.
        LessOrEqual,            // Comparison evaluates reference ≤ test.
        Greater,                // Comparison evaluates reference > test.
        GreaterOrEqual,         // Comparison evaluates reference ≥ test.
        Always                  // Comparison always evaluates true.
    };

    enum class StencilOperationType
    {
        Keep = 0,               // Keeps the current value.
        Zero,                   // Sets the value to 0.
        Replace,                // Sets the value to reference.
        IncrementAndClamp,      // Increments the current value and clamps to the maximum representable unsigned value.
        DecrementAndClamp,      // Decrements the current value and clamps to 0.
        Invert,                 // Bitwise-inverts the current value.
        IncrementAndWrap,       // Increments the current value and wraps to 0 when the maximum value would have been exceeded.
        DecrementAndWrap        // Decrements the current value and wraps to the maximum possible value when the value would go below 0.
    };

    enum class LogicOperationType
    {
        Clear = 0,
        And,
        AndReverse,
        Copy,
        AndInverted,
        NoOp,
        Xor,
        Or,
        Nor,
        Equivalent,
        Invert,
        OrReverse,
        CopyInverted,
        OrInverted,
        Nand,
        Set
    };

    enum class BlendOperationType
    {
        Add = 0,
        Subtract,
        ReverseSubtract,
        Min,
        Max

        // further blend ops available via VK_EXT_blend_operation_advanced
    };

    enum class BlendFactor
    {
        Zero = 0,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate
    };

    enum class DynamicState : uint32_t
    {
        None = 0,
        LineWidth,
        DepthBias,
        BlendConstants,
        StencilRef,
        CullMode

        // More available, see: https://docs.vulkan.org/refpages/latest/refpages/source/VkDynamicState.html
    };

    /// <summary>
    /// Is the vertex binding stepped at a per-vertex or per-instance rate?
    /// </summary>
    enum class VertexInputRate : uint32_t
    {
        PerVertex = 0,
        PerInstance = 1
    };

    /// <summary>
    /// Buffer and image sharing modes.
    /// </summary>
    enum class SharingMode : uint32_t
    {
        Exclusive = 0,          // Access to any range or image subresource of the object will be exclusive to a single queue family at a time.
        Concurrent = 1          // Concurrent access to any range or image subresource of the object from multiple queue families is supported.
    };

    enum class BufferMemoryUsage : uint32_t
    {
        GpuOnly = 0,            // A buffer that is used to write/read exclusively on the GPU
        Staging = 1,            // A staging buffer used to transfer CPU provided data to a GPU buffer
        ReadBack = 2,           // A buffer that is written to by the GPU and read back on the CPU
        PersistentMap = 3,      // A buffer that is both frequently written on the CPU and read on the GPU
    };

    enum class IndexType : uint32_t
    {
        Uint32 = 0,
        Uint16 = 1,
        Uint8 = 2
    };

    enum class SampleCount : uint32_t
    {
        Sample1  = 0,
        Sample2  = 1,
        Sample4  = 2,
        Sample8  = 3,
        Sample16 = 4,
        Sample32 = 5,
        Sample64 = 6
    };

    enum class TextureDimensions : uint32_t
    {
        Texture1D = 0,
        Texture2D = 1,
        Texture3D = 3
    };

    enum class SamplerFilter : uint32_t
    {
        Nearest = 0,
        Linear = 1
    };

    enum class SamplerMipFilter : uint32_t
    {
        None = 0,
        Nearest = 1,
        Linear = 2
    };

    enum class SamplerAddressMode : uint32_t
    {
        Repeat = 0,
        MirrorRepeat = 1,
        ClampEdge = 2,
        ClampBorder = 3,
        MirrorClampEdge = 4
    };

    enum class SamplerBorderColor : uint32_t
    {
        TransparentBlack = 0,
        OpaqueBlack = 1,
        OpaqueWhite = 2
    };

    enum class SamplerAnisotropy : uint32_t
    {
        Off = 0,
        X2 = 1,
        X4 = 2,
        X8 = 3,
        X16 = 4
    };
}

#endif