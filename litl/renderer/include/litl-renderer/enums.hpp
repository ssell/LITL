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
}

#endif