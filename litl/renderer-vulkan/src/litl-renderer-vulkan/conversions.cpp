#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/conversions.hpp"

namespace litl::vulkan
{
    // -------------------------------------------------------------------------------------
    // ShaderStage <-> VkShaderStageFlags
    // -------------------------------------------------------------------------------------

    static_assert(static_cast<uint32_t>(ShaderStage::Vertex) == 0b00000001, "ShaderStage::Vertex has changed! Update Vulkan conversions!");
    static_assert(static_cast<uint32_t>(ShaderStage::Fragment) == 0b00000010, "ShaderStage::Fragment has changed! Update Vulkan conversions!");
    static_assert(static_cast<uint32_t>(ShaderStage::Geometry) == 0b00000100, "ShaderStage::Geometry has changed! Update Vulkan conversions!");
    static_assert(static_cast<uint32_t>(ShaderStage::TessellationControl) == 0b00001000, "ShaderStage::TessellationControl has changed! Update Vulkan conversions!");
    static_assert(static_cast<uint32_t>(ShaderStage::TessellationEvaluation) == 0b00010000, "ShaderStage::TessellationEvaluation has changed! Update Vulkan conversions!");
    static_assert(static_cast<uint32_t>(ShaderStage::Compute) == 0b00100000, "ShaderStage::Compute has changed! Update Vulkan conversions!");
    static_assert(static_cast<uint32_t>(ShaderStage::Mesh) == 0b01000000, "ShaderStage::Mesh has changed! Update Vulkan conversions!");
    static_assert(static_cast<uint32_t>(ShaderStage::Task) == 0b10000000, "ShaderStage::Task has changed! Update Vulkan conversions!");

    VkShaderStageFlags toVkShaderStageFlags(ShaderStage stages) noexcept
    {
        VkShaderStageFlags flags = 0;

        if (any(stages & ShaderStage::Vertex)) { flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT; }
        if (any(stages & ShaderStage::Fragment)) { flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT; }
        if (any(stages & ShaderStage::Geometry)) { flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT; }
        if (any(stages & ShaderStage::TessellationControl)) { flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; }
        if (any(stages & ShaderStage::TessellationEvaluation)) { flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; }
        if (any(stages & ShaderStage::Compute)) { flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT; }
        if (any(stages & ShaderStage::Mesh)) { flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT; }
        if (any(stages & ShaderStage::Task)) { flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT; }

        return flags;
    }

    ShaderStage fromVkShaderStageFlags(VkShaderStageFlags flags) noexcept
    {
        ShaderStage stages = ShaderStage::None;

        if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT) { stages = stages | ShaderStage::Vertex; }
        if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT) { stages = stages | ShaderStage::Fragment; }
        if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT) { stages = stages | ShaderStage::Geometry; }
        if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) { stages = stages | ShaderStage::TessellationControl; }
        if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) { stages = stages | ShaderStage::TessellationEvaluation; }
        if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT) { stages = stages | ShaderStage::Compute; }
        if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT) { stages = stages | ShaderStage::Mesh; }
        if (flags & VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT) { stages = stages | ShaderStage::Task; }

        return stages;
    }

    // -------------------------------------------------------------------------------------
    // ShaderResourceType <-> VkDescriptorType
    // -------------------------------------------------------------------------------------

    VkDescriptorType toVkDescriptorType(ShaderResourceType type) noexcept
    {
        switch (type)
        {
        case ShaderResourceType::Sampler:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;

        case ShaderResourceType::UniformBuffer:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        case ShaderResourceType::StorageBuffer:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        case ShaderResourceType::ImageBuffer:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

        case ShaderResourceType::SampledImage:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

        case ShaderResourceType::StorageImage:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        case ShaderResourceType::InputAttachment:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

        case ShaderResourceType::AccelerationStructure:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

        case ShaderResourceType::Unknown:
        default:
            logError("Unknown/unhandled shader resource type of ", static_cast<uint32_t>(type), " provided to toVkDescriptorType");
            return static_cast<VkDescriptorType>(0);
        }
    }

    ShaderResourceType fromVkDescriptorType(VkDescriptorType type) noexcept
    {
        switch (type)
        {
        case VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER:
            return ShaderResourceType::Sampler;

        case VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return ShaderResourceType::UniformBuffer;

        case VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return ShaderResourceType::StorageBuffer;

        case VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return ShaderResourceType::ImageBuffer;

        case VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return ShaderResourceType::SampledImage;

        case VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return ShaderResourceType::StorageImage;

        case VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return ShaderResourceType::InputAttachment;

        case VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return ShaderResourceType::AccelerationStructure;

        default:
            return ShaderResourceType::Unknown;
        }
    }

    // -------------------------------------------------------------------------------------
    // ImageFormat <-> VkFormat
    // -------------------------------------------------------------------------------------

    VkFormat toVkFormat(ImageFormat format) noexcept
    {
        switch (format)
        {
            // Color
        case ImageFormat::RGBA8_UNorm:
            return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;

        case ImageFormat::RGBA8_SRGB:
            return VkFormat::VK_FORMAT_R8G8B8A8_SRGB;

        case ImageFormat::BGRA8_Unorm:
            return VkFormat::VK_FORMAT_B8G8R8A8_UNORM;

        case ImageFormat::BGRA8_SRGB:
            return VkFormat::VK_FORMAT_B8G8R8A8_SRGB;

        case ImageFormat::ABGR10_UNorm_Pack32:
            return VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32;

            // HDR
        case ImageFormat::RGBA16_SFloat:
            return VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;

        case ImageFormat::RGBA32_SFloat:
            return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;

        case ImageFormat::R11G11B10_UFloat:
            return VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32;

            // Single Channel
        case ImageFormat::R8_UNorm:
            return VkFormat::VK_FORMAT_R8_UNORM;

        case ImageFormat::R16_SFloat:
            return VkFormat::VK_FORMAT_R16_SFLOAT;

        case ImageFormat::R32_SFloat:
            return VkFormat::VK_FORMAT_R32_SFLOAT;

            // Dual Channel
        case ImageFormat::RG8_UNorm:
            return VkFormat::VK_FORMAT_R8G8_UNORM;

        case ImageFormat::RG16_SFloat:
            return VkFormat::VK_FORMAT_R16G16_SFLOAT;

            // Depth
        case ImageFormat::D32_SFloat:
            return VkFormat::VK_FORMAT_R32G32_SFLOAT;

        case ImageFormat::D24_UNorm_S8_UInt:
            return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;

        case ImageFormat::D24_SFloat_S8_Uint:
            return VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;

            // Compressed
        case ImageFormat::BC7_UNorm:
            return VkFormat::VK_FORMAT_BC7_UNORM_BLOCK;

        case ImageFormat::BC7_SRGB:
            return VkFormat::VK_FORMAT_BC7_SRGB_BLOCK;

        case ImageFormat::BC4_UNorm:
            return VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;

        case ImageFormat::BC5_UNorm:
            return VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;

        case ImageFormat::BC6H_UFloat:
            return VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK;

            // Fall-through
        case ImageFormat::Undefined:
        default:
            return VkFormat::VK_FORMAT_UNDEFINED;
        }
    }

    ImageFormat fromVkFormat(VkFormat format) noexcept
    {
        switch (format)
        {
            // Color
        case VkFormat::VK_FORMAT_R8G8B8A8_UNORM:
            return ImageFormat::RGBA8_UNorm;

        case VkFormat::VK_FORMAT_R8G8B8A8_SRGB:
            return ImageFormat::RGBA8_SRGB;

        case VkFormat::VK_FORMAT_B8G8R8A8_UNORM:
            return ImageFormat::BGRA8_Unorm;

        case VkFormat::VK_FORMAT_B8G8R8A8_SRGB:
            return ImageFormat::BGRA8_SRGB;

        case VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return ImageFormat::ABGR10_UNorm_Pack32;

            // HDR
        case VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT:
            return ImageFormat::RGBA16_SFloat;

        case VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT:
            return ImageFormat::RGBA32_SFloat;

        case VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            return ImageFormat::R11G11B10_UFloat;

            // Single Channel
        case VkFormat::VK_FORMAT_R8_UNORM:
            return ImageFormat::R8_UNorm;

        case VkFormat::VK_FORMAT_R16_SFLOAT:
            return ImageFormat::R16_SFloat;

        case VkFormat::VK_FORMAT_R32_SFLOAT:
            return ImageFormat::R32_SFloat;

            // Dual Channel
        case VkFormat::VK_FORMAT_R8G8_UNORM:
            return ImageFormat::RG8_UNorm;

        case VkFormat::VK_FORMAT_R16G16_SFLOAT:
            return ImageFormat::RG16_SFloat;

            // Depth
        case VkFormat::VK_FORMAT_R32G32_SFLOAT:
            return ImageFormat::D32_SFloat;

        case VkFormat::VK_FORMAT_D24_UNORM_S8_UINT:
            return ImageFormat::D24_UNorm_S8_UInt;

        case VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT:
            return ImageFormat::D24_SFloat_S8_Uint;

            // Compressed
        case VkFormat::VK_FORMAT_BC7_UNORM_BLOCK:
            return ImageFormat::BC7_UNorm;

        case VkFormat::VK_FORMAT_BC7_SRGB_BLOCK:
            return ImageFormat::BC7_SRGB;

        case VkFormat::VK_FORMAT_BC4_UNORM_BLOCK:
            return ImageFormat::BC4_UNorm;

        case VkFormat::VK_FORMAT_BC5_UNORM_BLOCK:
            return ImageFormat::BC5_UNorm;

        case VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK:
            return ImageFormat::BC6H_UFloat;

        case VkFormat::VK_FORMAT_UNDEFINED:
        default:
            return ImageFormat::Undefined;
        }
    }

    // -------------------------------------------------------------------------------------
    // ImageLayoutType <-> VkImageLayout
    // -------------------------------------------------------------------------------------

    VkImageLayout toVkImageLayout(ImageLayoutType layout) noexcept
    {
        switch (layout)
        {
        case ImageLayoutType::General:
            return VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;

        case ImageLayoutType::Color:
            return VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        case ImageLayoutType::DepthStencil:
            return VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        case ImageLayoutType::DepthStencilReadOnly:
            return VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        case ImageLayoutType::ShaderReadOnly:
            return VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        case ImageLayoutType::TransferSrc:
            return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        case ImageLayoutType::TransferDst:
            return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        case ImageLayoutType::Preinitialized:
            return VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED;

        case ImageLayoutType::Present:
            return VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        case ImageLayoutType::Undefined:
        default:
            return VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    ImageLayoutType fromVkImageLayout(VkImageLayout layout) noexcept
    {
        switch (layout)
        {
        case VkImageLayout::VK_IMAGE_LAYOUT_GENERAL:
            return ImageLayoutType::General;

        case VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return ImageLayoutType::Color;

        case VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return ImageLayoutType::DepthStencil;

        case VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return ImageLayoutType::DepthStencilReadOnly;

        case VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return ImageLayoutType::ShaderReadOnly;

        case VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return ImageLayoutType::TransferSrc;

        case VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return ImageLayoutType::TransferDst;

        case VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED:
            return ImageLayoutType::Preinitialized;

        case VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return ImageLayoutType::Present;

        case VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED:
        default:
            return ImageLayoutType::Undefined;
        }
    }

    // -------------------------------------------------------------------------------------
    // ImageAccessFlagBits <-> VkAccessFlagBits2
    // -------------------------------------------------------------------------------------

    VkAccessFlags2 toVkAccessFlag(ImageAccessFlag flag) noexcept
    {
        VkAccessFlags2 vkFlag = 0;

        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::IndirectCommandRead)) != 0) { vkFlag |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::IndexRead)) != 0) { vkFlag |= VK_ACCESS_2_INDEX_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::VertexAttributeRead)) != 0) { vkFlag |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::UniformRead)) != 0) { vkFlag |= VK_ACCESS_2_UNIFORM_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::InputAttachmentRead)) != 0) { vkFlag |= VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::ShaderRead)) != 0) { vkFlag |= VK_ACCESS_2_SHADER_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::ShaderWrite)) != 0) { vkFlag |= VK_ACCESS_2_SHADER_WRITE_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::ColorRead)) != 0) { vkFlag |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::ColorWrite)) != 0) { vkFlag |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::DepthStencilRead)) != 0) { vkFlag |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::DepthStencilWrite)) != 0) { vkFlag |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::TransferRead)) != 0) { vkFlag |= VK_ACCESS_2_TRANSFER_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::TransferWrite)) != 0) { vkFlag |= VK_ACCESS_2_TRANSFER_WRITE_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::HostRead)) != 0) { vkFlag |= VK_ACCESS_2_HOST_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::HostWrite)) != 0) { vkFlag |= VK_ACCESS_2_HOST_WRITE_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::MemoryRead)) != 0) { vkFlag |= VK_ACCESS_2_MEMORY_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::MemoryWrite)) != 0) { vkFlag |= VK_ACCESS_2_MEMORY_WRITE_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::ShaderSampledRead)) != 0) { vkFlag |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::ShaderStorageRead)) != 0) { vkFlag |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT; }
        if ((flag & static_cast<uint64_t>(ImageAccessFlagBits::ShaderStorageWrite)) != 0) { vkFlag |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT; }

        return vkFlag;
    }

    ImageAccessFlag fromVkAccessFlag(VkAccessFlags2 flag) noexcept
    {
        ImageAccessFlag litlFlag = 0;

        if ((flag & VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::IndirectCommandRead); }
        if ((flag & VK_ACCESS_2_INDEX_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::IndexRead); }
        if ((flag & VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::VertexAttributeRead); }
        if ((flag & VK_ACCESS_2_UNIFORM_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::UniformRead); }
        if ((flag & VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::InputAttachmentRead); }
        if ((flag & VK_ACCESS_2_SHADER_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::ShaderRead); }
        if ((flag & VK_ACCESS_2_SHADER_WRITE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::ShaderWrite); }
        if ((flag & VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::ColorRead); }
        if ((flag & VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::ColorWrite); }
        if ((flag & VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::DepthStencilRead); }
        if ((flag & VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::DepthStencilWrite); }
        if ((flag & VK_ACCESS_2_TRANSFER_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::TransferRead); }
        if ((flag & VK_ACCESS_2_TRANSFER_WRITE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::TransferWrite); }
        if ((flag & VK_ACCESS_2_HOST_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::HostRead); }
        if ((flag & VK_ACCESS_2_HOST_WRITE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::HostWrite); }
        if ((flag & VK_ACCESS_2_MEMORY_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::MemoryRead); }
        if ((flag & VK_ACCESS_2_MEMORY_WRITE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::MemoryWrite); }
        if ((flag & VK_ACCESS_2_SHADER_SAMPLED_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::ShaderSampledRead); }
        if ((flag & VK_ACCESS_2_SHADER_STORAGE_READ_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::ShaderStorageRead); }
        if ((flag & VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(ImageAccessFlagBits::ShaderStorageWrite); }
        
        return litlFlag;
    }

    // -------------------------------------------------------------------------------------
    // PipelineStageFlagBits <-> VkPipelineStageFlagBits2
    // -------------------------------------------------------------------------------------

    VkPipelineStageFlags2 toVkPipelineStageFlag(PipelineStageFlag flag) noexcept
    {
        VkPipelineStageFlags2 vkFlag = 0;

        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::TopOfPipe)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::DrawIndirect)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::VertexInput)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::VertexShader)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::TessellationControlShader)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::TessellationEvaluationShader)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::GeometryShader)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::FragmentShader)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::EarlyFragmentTests)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::LateFragmentTests)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::ColorAttachmentOutput)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::ComputeShader)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::Transfer)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_TRANSFER_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::BottomOfPipe)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::Host)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_HOST_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::AllGraphics)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::AllCommands)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::Copy)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_COPY_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::Resolve)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_RESOLVE_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::Blit)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_BLIT_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::Clear)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_CLEAR_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::IndexInput)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::VertexAttributeInput)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT; }
        if ((flag & static_cast<uint64_t>(PipelineStageFlagBits::PreRasterizationShaders)) != 0) { vkFlag |= VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT; }

        return vkFlag;
    }

    PipelineStageFlag fromVkPipelineStageFlag(VkPipelineStageFlags2 flag) noexcept
    {
        PipelineStageFlag litlFlag = 0;

        if ((flag & VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::TopOfPipe); }
        if ((flag & VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::DrawIndirect); }
        if ((flag & VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::VertexInput); }
        if ((flag & VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::VertexShader); }
        if ((flag & VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::TessellationControlShader); }
        if ((flag & VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::TessellationEvaluationShader); }
        if ((flag & VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::GeometryShader); }
        if ((flag & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::FragmentShader); }
        if ((flag & VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::EarlyFragmentTests); }
        if ((flag & VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::LateFragmentTests); }
        if ((flag & VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::ColorAttachmentOutput); }
        if ((flag & VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::ComputeShader); }
        if ((flag & VK_PIPELINE_STAGE_2_TRANSFER_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::Transfer); }
        if ((flag & VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::BottomOfPipe); }
        if ((flag & VK_PIPELINE_STAGE_2_HOST_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::Host); }
        if ((flag & VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::AllGraphics); }
        if ((flag & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::AllCommands); }
        if ((flag & VK_PIPELINE_STAGE_2_COPY_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::Copy); }
        if ((flag & VK_PIPELINE_STAGE_2_RESOLVE_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::Resolve); }
        if ((flag & VK_PIPELINE_STAGE_2_BLIT_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::Blit); }
        if ((flag & VK_PIPELINE_STAGE_2_CLEAR_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::Clear); }
        if ((flag & VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::IndexInput); }
        if ((flag & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::VertexAttributeInput); }
        if ((flag & VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT) != 0) { litlFlag |= static_cast<uint64_t>(PipelineStageFlagBits::PreRasterizationShaders); }

        return litlFlag;
    }

    // -------------------------------------------------------------------------------------
    // LoadOperationType <-> VkAttachmentLoadOp
    // -------------------------------------------------------------------------------------

    VkAttachmentLoadOp toVkAttachmentLoadOp(LoadOperationType op) noexcept
    {
        switch (op)
        {
        case LoadOperationType::Load:
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;

        case LoadOperationType::Clear:
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;

        case LoadOperationType::DontCare:
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

        case LoadOperationType::None:
        default:
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_NONE;
        }
    }

    LoadOperationType fromVkAttachmentLoadOp(VkAttachmentLoadOp op) noexcept
    {
        switch (op)
        {
        case VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD:
            return LoadOperationType::Load;

        case VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR:
            return LoadOperationType::Clear;

        case VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE:
            return LoadOperationType::DontCare;

        case VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_NONE:
        default:
            return LoadOperationType::None;
        }
    }

    // -------------------------------------------------------------------------------------
    // StoreOperationType <-> VkAttachmentStoreOp
    // -------------------------------------------------------------------------------------

    VkAttachmentStoreOp toVkAttachmentStoreOp(StoreOperationType op) noexcept
    {
        switch (op)
        {
        case StoreOperationType::Store:
            return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;

        case StoreOperationType::DontCare:
            return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;

        case StoreOperationType::None:
        default:
            return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE;
        }
    }

    StoreOperationType fromVkAttachmentStoreOp(VkAttachmentStoreOp op) noexcept
    {
        switch (op)
        {
        case VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE:
            return StoreOperationType::Store;

        case VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE:
            return StoreOperationType::DontCare;

        case VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE:
        default:
            return StoreOperationType::None;
        }
    }
}