#ifndef LITL_RENDERER_ENUMS_H__
#define LITL_RENDERER_ENUMS_H__

#include <cstdint>

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

    /// <summary>
    /// Composed of PipelineStageFlagBits
    /// </summary>
    using PipelineStageFlag = uint64_t;

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

    enum class ImageFormat : uint32_t
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
}

#endif