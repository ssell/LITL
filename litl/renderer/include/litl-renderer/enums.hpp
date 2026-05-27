#ifndef LITL_RENDERER_ENUMS_H__
#define LITL_RENDERER_ENUMS_H__

#include <cstdint>

namespace litl
{
    // Based on: https://docs.vulkan.org/refpages/latest/refpages/source/VkImageLayout.html
    enum class ImageLayoutType : uint32_t
    {
        Undefined = 0,
        General = 1,
        Color = 2,
        DepthStencil = 3,
        DepthStencilReadOnly = 4,
        ShaderReadOnly = 5,
        TransferSrc = 6,
        TransferDst = 7,
        Preinitialized = 8,
        Present = 1000001002
    };

    // Based on: https://docs.vulkan.org/refpages/latest/refpages/source/VkAccessFlagBits2.html
    enum class ImageAccessFlagBits : uint64_t
    {
        None                = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000,
        IndirectCommandRead = 0b0000'0000'0000'0000'0000'0000'0000'0000'0001,
        IndexRead           = 0b0000'0000'0000'0000'0000'0000'0000'0000'0010,
        VertexAttributeRead = 0b0000'0000'0000'0000'0000'0000'0000'0000'0100,
        UniformRead         = 0b0000'0000'0000'0000'0000'0000'0000'0000'1000,
        InputAttachmentRead = 0b0000'0000'0000'0000'0000'0000'0000'0001'0000,
        ShaderRead          = 0b0000'0000'0000'0000'0000'0000'0000'0010'0000,
        ShaderWrite         = 0b0000'0000'0000'0000'0000'0000'0000'0100'0000,
        ColorRead           = 0b0000'0000'0000'0000'0000'0000'0000'1000'0000,
        ColorWrite          = 0b0000'0000'0000'0000'0000'0000'0001'0000'0000,
        DepthStencilRead    = 0b0000'0000'0000'0000'0000'0000'0010'0000'0000,
        DepthStencilWrite   = 0b0000'0000'0000'0000'0000'0000'0100'0000'0000,
        TransferRead        = 0b0000'0000'0000'0000'0000'0000'1000'0000'0000,
        TransferWrite       = 0b0000'0000'0000'0000'0000'0001'0000'0000'0000,
        HostRead            = 0b0000'0000'0000'0000'0000'0010'0000'0000'0000,
        HostWrite           = 0b0000'0000'0000'0000'0000'0100'0000'0000'0000,
        MemoryRead          = 0b0000'0000'0000'0000'0000'1000'0000'0000'0000,
        MemoryWrite         = 0b0000'0000'0000'0000'0001'0000'0000'0000'0000,
        ShaderSampledRead   = 0b0001'0000'0000'0000'0000'0000'0000'0000'0000,
        ShaderStorageRead   = 0b0010'0000'0000'0000'0000'0000'0000'0000'0000,
        ShaderStorageWrite  = 0b0100'0000'0000'0000'0000'0000'0000'0000'0000
    };

    // Based on https://docs.vulkan.org/refpages/latest/refpages/source/VkPipelineStageFlagBits2.html
    enum class PipelineStageFlagBits : uint64_t
    {
        None                         = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000,
        TopOfPipe                    = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0001,
        DrawIndirect                 = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0010,
        VertexInput                  = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0100,
        VertexShader                 = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'1000,
        TessellationControlShader    = 0b0000'0000'0000'0000'0000'0000'0000'0000'0001'0000,
        TessellationEvaluationShader = 0b0000'0000'0000'0000'0000'0000'0000'0000'0010'0000,
        GeometryShader               = 0b0000'0000'0000'0000'0000'0000'0000'0000'0100'0000,
        FragmentShader               = 0b0000'0000'0000'0000'0000'0000'0000'0000'1000'0000,
        EarlyFragmentTests           = 0b0000'0000'0000'0000'0000'0000'0000'0001'0000'0000,
        LateFragmentTests            = 0b0000'0000'0000'0000'0000'0000'0000'0010'0000'0000,
        ColorAttachmentOutput        = 0b0000'0000'0000'0000'0000'0000'0000'0100'0000'0000,
        ComputeShader                = 0b0000'0000'0000'0000'0000'0000'0000'1000'0000'0000,
        Transfer                     = 0b0000'0000'0000'0000'0000'0000'0001'0000'0000'0000,
        BottomOfPipe                 = 0b0000'0000'0000'0000'0000'0000'0010'0000'0000'0000,
        Host                         = 0b0000'0000'0000'0000'0000'0000'0100'0000'0000'0000,
        AllGraphics                  = 0b0000'0000'0000'0000'0000'0000'1000'0000'0000'0000,
        AllCommands                  = 0b0000'0000'0000'0000'0000'0001'0000'0000'0000'0000,
        Copy                         = 0b0000'0001'0000'0000'0000'0000'0000'0000'0000'0000,
        Resolve                      = 0b0000'0010'0000'0000'0000'0000'0000'0000'0000'0000,
        Blit                         = 0b0000'0100'0000'0000'0000'0000'0000'0000'0000'0000,
        Clear                        = 0b0000'1000'0000'0000'0000'0000'0000'0000'0000'0000,
        IndexInput                   = 0b0001'0000'0000'0000'0000'0000'0000'0000'0000'0000,
        VertexAttributeInput         = 0b0010'0000'0000'0000'0000'0000'0000'0000'0000'0000,
        PreRasterizationShaders      = 0b0100'0000'0000'0000'0000'0000'0000'0000'0000'0000,
    };

    /// <summary>
    /// Based on: https://docs.vulkan.org/refpages/latest/refpages/source/VkAttachmentLoadOp.html
    /// </summary>
    enum class LoadOperationType : uint32_t
    {
        Load = 0,
        Clear = 1,
        DontCare = 2,
        None = 1000400000
    };

    /// <summary>
    /// Based on: https://docs.vulkan.org/refpages/latest/refpages/source/VkAttachmentStoreOp.html
    /// </summary>
    enum class StoreOperationType : uint32_t
    {
        Store = 0,
        DontCare = 1,
        None = 1000301000
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