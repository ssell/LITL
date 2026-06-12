#include "litl-core/logging/logging.hpp"
#include "litl-core/enumBitFlags.hpp"
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

        default:
            logError("Unsupported ShaderResourceType '", static_cast<uint32_t>(type), "' defaulting to VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER");
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER;  // = 0
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
            logError("Unsupported VkDescriptorType '", type, "' defaulting to ShaderResourceType::Unknown");
            return ShaderResourceType::Unknown;
        }
    }

    // -------------------------------------------------------------------------------------
    // ImageFormat <-> VkFormat
    // -------------------------------------------------------------------------------------

    VkFormat toVkFormat(DataFormat format) noexcept
    {
        switch (format)
        {
            // Color
        case DataFormat::RGBA8_UNorm:
            return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;

        case DataFormat::RGBA8_SRGB:
            return VkFormat::VK_FORMAT_R8G8B8A8_SRGB;

        case DataFormat::BGRA8_Unorm:
            return VkFormat::VK_FORMAT_B8G8R8A8_UNORM;

        case DataFormat::BGRA8_SRGB:
            return VkFormat::VK_FORMAT_B8G8R8A8_SRGB;

        case DataFormat::ABGR10_UNorm_Pack32:
            return VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32;

            // HDR
        case DataFormat::RGBA16_SFloat:
            return VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT;

        case DataFormat::RGB32_SFloat:
            return VkFormat::VK_FORMAT_R32G32B32_SFLOAT;

        case DataFormat::RGBA32_SFloat:
            return VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;

        case DataFormat::R11G11B10_UFloat:
            return VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32;

            // Single Channel
        case DataFormat::R8_UNorm:
            return VkFormat::VK_FORMAT_R8_UNORM;

        case DataFormat::R16_SFloat:
            return VkFormat::VK_FORMAT_R16_SFLOAT;

        case DataFormat::R32_SFloat:
            return VkFormat::VK_FORMAT_R32_SFLOAT;

            // Dual Channel
        case DataFormat::RG8_UNorm:
            return VkFormat::VK_FORMAT_R8G8_UNORM;

        case DataFormat::RG16_SFloat:
            return VkFormat::VK_FORMAT_R16G16_SFLOAT;

            // Depth
        case DataFormat::D32_SFloat:
            return VkFormat::VK_FORMAT_R32G32_SFLOAT;

        case DataFormat::D24_UNorm_S8_UInt:
            return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;

        case DataFormat::D24_SFloat_S8_Uint:
            return VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;

            // Compressed
        case DataFormat::BC7_UNorm:
            return VkFormat::VK_FORMAT_BC7_UNORM_BLOCK;

        case DataFormat::BC7_SRGB:
            return VkFormat::VK_FORMAT_BC7_SRGB_BLOCK;

        case DataFormat::BC4_UNorm:
            return VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;

        case DataFormat::BC5_UNorm:
            return VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;

        case DataFormat::BC6H_UFloat:
            return VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK;

            // Fall-through
        case DataFormat::Undefined:
        default:
            logError("Unsupported ShaderResourceType '", static_cast<uint32_t>(format), "' defaulting to VkFormat::VK_FORMAT_UNDEFINED");
            return VkFormat::VK_FORMAT_UNDEFINED;
        }
    }

    DataFormat fromVkFormat(VkFormat format) noexcept
    {
        switch (format)
        {
            // Color
        case VkFormat::VK_FORMAT_R8G8B8A8_UNORM:
            return DataFormat::RGBA8_UNorm;

        case VkFormat::VK_FORMAT_R8G8B8A8_SRGB:
            return DataFormat::RGBA8_SRGB;

        case VkFormat::VK_FORMAT_B8G8R8A8_UNORM:
            return DataFormat::BGRA8_Unorm;

        case VkFormat::VK_FORMAT_B8G8R8A8_SRGB:
            return DataFormat::BGRA8_SRGB;

        case VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return DataFormat::ABGR10_UNorm_Pack32;

            // HDR
        case VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT:
            return DataFormat::RGBA16_SFloat;

        case VkFormat::VK_FORMAT_R32G32B32_SFLOAT:
            return DataFormat::RGB32_SFloat;

        case VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT:
            return DataFormat::RGBA32_SFloat;

        case VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            return DataFormat::R11G11B10_UFloat;

            // Single Channel
        case VkFormat::VK_FORMAT_R8_UNORM:
            return DataFormat::R8_UNorm;

        case VkFormat::VK_FORMAT_R16_SFLOAT:
            return DataFormat::R16_SFloat;

        case VkFormat::VK_FORMAT_R32_SFLOAT:
            return DataFormat::R32_SFloat;

            // Dual Channel
        case VkFormat::VK_FORMAT_R8G8_UNORM:
            return DataFormat::RG8_UNorm;

        case VkFormat::VK_FORMAT_R16G16_SFLOAT:
            return DataFormat::RG16_SFloat;

            // Depth
        case VkFormat::VK_FORMAT_R32G32_SFLOAT:
            return DataFormat::D32_SFloat;

        case VkFormat::VK_FORMAT_D24_UNORM_S8_UINT:
            return DataFormat::D24_UNorm_S8_UInt;

        case VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT:
            return DataFormat::D24_SFloat_S8_Uint;

            // Compressed
        case VkFormat::VK_FORMAT_BC7_UNORM_BLOCK:
            return DataFormat::BC7_UNorm;

        case VkFormat::VK_FORMAT_BC7_SRGB_BLOCK:
            return DataFormat::BC7_SRGB;

        case VkFormat::VK_FORMAT_BC4_UNORM_BLOCK:
            return DataFormat::BC4_UNorm;

        case VkFormat::VK_FORMAT_BC5_UNORM_BLOCK:
            return DataFormat::BC5_UNorm;

        case VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK:
            return DataFormat::BC6H_UFloat;

        case VkFormat::VK_FORMAT_UNDEFINED:
        default:
            logError("Unsupported VkFormat '", format, "' defaulting to DataFormat::Undefined");
            return DataFormat::Undefined;
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
            logError("Unsupported ImageLayoutType '", static_cast<uint32_t>(layout), "' defaulting to VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED");
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
            logError("Unsupported VkImageLayout '", layout, "' defaulting to ImageLayoutType::Undefined");
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

        if (has_any(flag, PipelineStageFlagBits::TopOfPipe)) { vkFlag |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT; }
        if (has_any(flag, PipelineStageFlagBits::DrawIndirect)) { vkFlag |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT; }
        if (has_any(flag, PipelineStageFlagBits::VertexInput)) { vkFlag |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT; }
        if (has_any(flag, PipelineStageFlagBits::VertexShader)) { vkFlag |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT; }
        if (has_any(flag, PipelineStageFlagBits::TessellationControlShader)) { vkFlag |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT; }
        if (has_any(flag, PipelineStageFlagBits::TessellationEvaluationShader)) { vkFlag |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT; }
        if (has_any(flag, PipelineStageFlagBits::GeometryShader)) { vkFlag |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT; }
        if (has_any(flag, PipelineStageFlagBits::FragmentShader)) { vkFlag |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT; }
        if (has_any(flag, PipelineStageFlagBits::EarlyFragmentTests)) { vkFlag |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT; }
        if (has_any(flag, PipelineStageFlagBits::LateFragmentTests)) { vkFlag |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT; }
        if (has_any(flag, PipelineStageFlagBits::ColorAttachmentOutput)) { vkFlag |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT; }
        if (has_any(flag, PipelineStageFlagBits::ComputeShader)) { vkFlag |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT; }
        if (has_any(flag, PipelineStageFlagBits::Transfer)) { vkFlag |= VK_PIPELINE_STAGE_2_TRANSFER_BIT; }
        if (has_any(flag, PipelineStageFlagBits::BottomOfPipe)) { vkFlag |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT; }
        if (has_any(flag, PipelineStageFlagBits::Host)) { vkFlag |= VK_PIPELINE_STAGE_2_HOST_BIT; }
        if (has_any(flag, PipelineStageFlagBits::AllGraphics)) { vkFlag |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; }
        if (has_any(flag, PipelineStageFlagBits::AllCommands)) { vkFlag |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; }
        if (has_any(flag, PipelineStageFlagBits::Copy)) { vkFlag |= VK_PIPELINE_STAGE_2_COPY_BIT; }
        if (has_any(flag, PipelineStageFlagBits::Resolve)) { vkFlag |= VK_PIPELINE_STAGE_2_RESOLVE_BIT; }
        if (has_any(flag, PipelineStageFlagBits::Blit)) { vkFlag |= VK_PIPELINE_STAGE_2_BLIT_BIT; }
        if (has_any(flag, PipelineStageFlagBits::Clear)) { vkFlag |= VK_PIPELINE_STAGE_2_CLEAR_BIT; }
        if (has_any(flag, PipelineStageFlagBits::IndexInput)) { vkFlag |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT; }
        if (has_any(flag, PipelineStageFlagBits::VertexAttributeInput)) { vkFlag |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT; }
        if (has_any(flag, PipelineStageFlagBits::PreRasterizationShaders)) { vkFlag |= VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT; }

        return vkFlag;
    }

    PipelineStageFlag fromVkPipelineStageFlag(VkPipelineStageFlags2 flag) noexcept
    {
        PipelineStageFlag litlFlag = PipelineStageFlagBits::None;

        if ((flag & VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT) != 0) { litlFlag |= PipelineStageFlagBits::TopOfPipe; }
        if ((flag & VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT) != 0) { litlFlag |= PipelineStageFlagBits::DrawIndirect; }
        if ((flag & VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT) != 0) { litlFlag |= PipelineStageFlagBits::VertexInput; }
        if ((flag & VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT) != 0) { litlFlag |= PipelineStageFlagBits::VertexShader; }
        if ((flag & VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT) != 0) { litlFlag |= PipelineStageFlagBits::TessellationControlShader; }
        if ((flag & VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT) != 0) { litlFlag |= PipelineStageFlagBits::TessellationEvaluationShader; }
        if ((flag & VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT) != 0) { litlFlag |= PipelineStageFlagBits::GeometryShader; }
        if ((flag & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) != 0) { litlFlag |= PipelineStageFlagBits::FragmentShader; }
        if ((flag & VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT) != 0) { litlFlag |= PipelineStageFlagBits::EarlyFragmentTests; }
        if ((flag & VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT) != 0) { litlFlag |= PipelineStageFlagBits::LateFragmentTests; }
        if ((flag & VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT) != 0) { litlFlag |= PipelineStageFlagBits::ColorAttachmentOutput; }
        if ((flag & VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT) != 0) { litlFlag |= PipelineStageFlagBits::ComputeShader; }
        if ((flag & VK_PIPELINE_STAGE_2_TRANSFER_BIT) != 0) { litlFlag |= PipelineStageFlagBits::Transfer; }
        if ((flag & VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT) != 0) { litlFlag |= PipelineStageFlagBits::BottomOfPipe; }
        if ((flag & VK_PIPELINE_STAGE_2_HOST_BIT) != 0) { litlFlag |= PipelineStageFlagBits::Host; }
        if ((flag & VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT) != 0) { litlFlag |= PipelineStageFlagBits::AllGraphics; }
        if ((flag & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT) != 0) { litlFlag |= PipelineStageFlagBits::AllCommands; }
        if ((flag & VK_PIPELINE_STAGE_2_COPY_BIT) != 0) { litlFlag |= PipelineStageFlagBits::Copy; }
        if ((flag & VK_PIPELINE_STAGE_2_RESOLVE_BIT) != 0) { litlFlag |= PipelineStageFlagBits::Resolve; }
        if ((flag & VK_PIPELINE_STAGE_2_BLIT_BIT) != 0) { litlFlag |= PipelineStageFlagBits::Blit; }
        if ((flag & VK_PIPELINE_STAGE_2_CLEAR_BIT) != 0) { litlFlag |= PipelineStageFlagBits::Clear; }
        if ((flag & VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT) != 0) { litlFlag |= PipelineStageFlagBits::IndexInput; }
        if ((flag & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT) != 0) { litlFlag |= PipelineStageFlagBits::VertexAttributeInput; }
        if ((flag & VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT) != 0) { litlFlag |= PipelineStageFlagBits::PreRasterizationShaders; }

        return litlFlag;
    }

    // -------------------------------------------------------------------------------------
    // LoadOperationType <-> VkAttachmentLoadOp
    // -------------------------------------------------------------------------------------

    VkAttachmentLoadOp toVkAttachmentLoadOp(LoadOperationType op) noexcept
    {
        switch (op)
        {
        case LoadOperationType::None:
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_NONE;

        case LoadOperationType::Load:
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;

        case LoadOperationType::Clear:
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;

        case LoadOperationType::DontCare:
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;

        default:
            logError("Unsupported LoadOperationType '", static_cast<uint32_t>(op), "' defaulting to  VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_NONE");
            return VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_NONE;
        }
    }

    LoadOperationType fromVkAttachmentLoadOp(VkAttachmentLoadOp op) noexcept
    {
        switch (op)
        {
        case VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_NONE:
            return LoadOperationType::None;

        case VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD:
            return LoadOperationType::Load;

        case VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR:
            return LoadOperationType::Clear;

        case VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE:
            return LoadOperationType::DontCare;

        default:
            logError("Unsupported VkAttachmentLoadOp '", op, "' defaulting to LoadOperationType::None");
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
        case StoreOperationType::None:
            return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE;

        case StoreOperationType::Store:
            return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;

        case StoreOperationType::DontCare:
            return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;

        default:
            logError("Unsupported StoreOperationType '", static_cast<uint32_t>(op), "' defaulting to  VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE");
            return VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE;
        }
    }

    StoreOperationType fromVkAttachmentStoreOp(VkAttachmentStoreOp op) noexcept
    {
        switch (op)
        {
        case VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_NONE:
            return StoreOperationType::None;

        case VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE:
            return StoreOperationType::Store;

        case VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE:
            return StoreOperationType::DontCare;

        default:
            logError("Unsupported VkAttachmentStoreOp '", op, "' defaulting to StoreOperationType::None");
            return StoreOperationType::None;
        }
    }

    // -------------------------------------------------------------------------------------
    // PrimitiveTopology <-> VkPrimitiveTopology
    // -------------------------------------------------------------------------------------

    VkPrimitiveTopology toVkPrimitiveTopology(PrimitiveTopology topology) noexcept
    {
        switch (topology)
        {
        case PrimitiveTopology::PointList:
            return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

        case PrimitiveTopology::LineList:
            return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

        case PrimitiveTopology::LineStrip:
            return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

        case PrimitiveTopology::TriangleList:
            return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        case PrimitiveTopology::TriangleStrip:
            return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        case PrimitiveTopology::TriangleFan:
            return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

        default:
            logError("Unsupported PrimitiveTopology ", static_cast<uint32_t>(topology), " provided to toVkPrimitiveTopology");
            return static_cast<VkPrimitiveTopology>(0);
        }
    }

    PrimitiveTopology fromVkPrimitiveTopology(VkPrimitiveTopology topology) noexcept
    {
        switch (topology)
        {
        case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
            return PrimitiveTopology::PointList;

        case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
            return PrimitiveTopology::LineList;

        case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
            return PrimitiveTopology::LineStrip;

        case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
            return PrimitiveTopology::TriangleList;

        case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
            return PrimitiveTopology::TriangleStrip;

        case VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
            return PrimitiveTopology::TriangleFan;

        default:
            logError("Unsupported VkPrimitiveTopology ", static_cast<uint32_t>(topology), " provided to fromVkPrimitiveTopology");
            return static_cast<PrimitiveTopology>(0);
        }
    }

    // -------------------------------------------------------------------------------------
    // PolygonMode <-> VkPolygonMode
    // -------------------------------------------------------------------------------------

    VkPolygonMode toVkPolygonMode(PolygonMode mode) noexcept
    {
        switch (mode)
        {
        case PolygonMode::Fill:
            return VkPolygonMode::VK_POLYGON_MODE_FILL;

        case PolygonMode::Line:
            return VkPolygonMode::VK_POLYGON_MODE_LINE;

        case PolygonMode::Point:
            return VkPolygonMode::VK_POLYGON_MODE_POINT;

        default:
            logError("Unsupported PolygonMode ", static_cast<uint32_t>(mode), " provided to toVkPolygonMode");
            return static_cast<VkPolygonMode>(0);
        }
    }

    PolygonMode fromVkPolygonMode(VkPolygonMode mode) noexcept
    {
        switch (mode)
        {
        case VkPolygonMode::VK_POLYGON_MODE_FILL:
            return PolygonMode::Fill;

        case VkPolygonMode::VK_POLYGON_MODE_LINE:
            return PolygonMode::Line;

        case VkPolygonMode::VK_POLYGON_MODE_POINT:
            return PolygonMode::Point;

        default:
            logError("Unsupported VkPolygonMode ", static_cast<uint32_t>(mode), " provided to fromVkPolygonMode");
            return static_cast<PolygonMode>(0);
        }
    }

    // -------------------------------------------------------------------------------------
    // CullMode <-> VkCullModeFlags
    // -------------------------------------------------------------------------------------

    VkCullModeFlags toVkCullModeFlag(CullMode mode) noexcept
    {
        switch (mode)
        {
        case CullMode::None:
            return VK_CULL_MODE_NONE;

        case CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;

        case CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;

        case CullMode::Both:
            return VK_CULL_MODE_FRONT_AND_BACK;

        default:
            logError("Unsupported CullMode ", static_cast<uint32_t>(mode), " provided to toVkCullModeFlag");
            return static_cast<VkCullModeFlags>(0);
        }
    }

    CullMode fromVkCullModeFlag(VkCullModeFlags mode) noexcept
    {
        switch (mode)
        {
        case VK_CULL_MODE_NONE:
            return CullMode::None;

        case VK_CULL_MODE_FRONT_BIT:
            return CullMode::Front;

        case VK_CULL_MODE_BACK_BIT:
            return CullMode::Back;

        case VK_CULL_MODE_FRONT_AND_BACK:
            return CullMode::Both;

        default:
            logError("Unsupported VkCullModeFlags ", static_cast<uint32_t>(mode), " provided to fromVkCullModeFlag");
            return static_cast<CullMode>(0);
        }
    }

    // -------------------------------------------------------------------------------------
    // FrontFace <-> VkFrontFace
    // -------------------------------------------------------------------------------------

    VkFrontFace toVkFrontFace(FrontFace face) noexcept
    {
        if (face == FrontFace::Clockwise)
        {
            return VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
        }
        else
        {
            return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
        }
    }

    FrontFace fromVkFrontFace(VkFrontFace face) noexcept
    {
        if (face == VkFrontFace::VK_FRONT_FACE_CLOCKWISE)
        {
            return FrontFace::Clockwise;
        }
        else
        {
            return FrontFace::CounterClockwise;
        }
    }

    // -------------------------------------------------------------------------------------
    // MultisampleCount <-> VkSampleCountFlags
    // -------------------------------------------------------------------------------------

    VkSampleCountFlags toVkSampleCountFlag(MultisampleCount count) noexcept
    {
        switch (count)
        {
        case MultisampleCount::Count1:
            return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

        case MultisampleCount::Count2:
            return VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT;

        case MultisampleCount::Count4:
            return VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT;

        case MultisampleCount::Count8:
            return VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT;

        case MultisampleCount::Count16:
            return VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT;

        case MultisampleCount::Count32:
            return VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT;

        case MultisampleCount::Count64:
            return VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT;

        default:
            logError("Unsupported MultisampleCount '", static_cast<uint32_t>(count), "' defaulting to VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT");
            return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
        }
    }

    MultisampleCount fromVkSampleCountFlag(VkSampleCountFlags count) noexcept
    {
        switch (count)
        {
        case VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT:
            return MultisampleCount::Count1;

        case VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT:
            return MultisampleCount::Count2;

        case VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT:
            return MultisampleCount::Count4;

        case VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT:
            return MultisampleCount::Count8;

        case VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT:
            return MultisampleCount::Count16;

        case VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT:
            return MultisampleCount::Count32;

        case VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT:
            return MultisampleCount::Count64;

        default:
            logError("Unsupported VkSampleCountFlags '", count, "' defaulting to MultisampleCount::Count1");
            return MultisampleCount::Count1;
        }
    }

    // -------------------------------------------------------------------------------------
    // CompareOperationType <-> VkCompareOp
    // -------------------------------------------------------------------------------------

    VkCompareOp toVkCompareOp(CompareOperationType op) noexcept
    {
        switch (op)
        {
        case CompareOperationType::Greater:
            return VkCompareOp::VK_COMPARE_OP_GREATER;

        case CompareOperationType::Never:
            return VkCompareOp::VK_COMPARE_OP_NEVER;

        case CompareOperationType::Less:
            return VkCompareOp::VK_COMPARE_OP_LESS;

        case CompareOperationType::Equal:
            return VkCompareOp::VK_COMPARE_OP_EQUAL;

        case CompareOperationType::LessOrEqual:
            return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;

        case CompareOperationType::GreaterOrEqual:
            return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;

        case CompareOperationType::Always:
            return VkCompareOp::VK_COMPARE_OP_ALWAYS;

        default:
            logError("Unsupported CompareOperationType '", static_cast<uint32_t>(op), "' defaulting to VkCompareOp::VK_COMPARE_OP_GREATER");
            return VkCompareOp::VK_COMPARE_OP_GREATER;
        }
    }

    CompareOperationType fromVkCompareOp(VkCompareOp op) noexcept
    {
        switch (op)
        {
        case VkCompareOp::VK_COMPARE_OP_GREATER:
            return CompareOperationType::Greater;

        case VkCompareOp::VK_COMPARE_OP_NEVER:
            return CompareOperationType::Never;

        case VkCompareOp::VK_COMPARE_OP_LESS:
            return CompareOperationType::Less;

        case VkCompareOp::VK_COMPARE_OP_EQUAL:
            return CompareOperationType::Equal;

        case VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL:
            return CompareOperationType::LessOrEqual;

        case VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL:
            return CompareOperationType::GreaterOrEqual;

        case VkCompareOp::VK_COMPARE_OP_ALWAYS:
            return CompareOperationType::Always;

        default:
            logError("Unsupported VkCompareOp '", op, "' defaulting to CompareOperationType::Greater");
            return CompareOperationType::Greater;
        }
    }

    // -------------------------------------------------------------------------------------
    // StencilOperationType <-> VkStencilOp
    // -------------------------------------------------------------------------------------

    VkStencilOp toVkStencilOp(StencilOperationType op) noexcept
    {
        switch (op)
        {
        case StencilOperationType::Keep:
            return VkStencilOp::VK_STENCIL_OP_KEEP;

        case StencilOperationType::Zero:
            return VkStencilOp::VK_STENCIL_OP_ZERO;

        case StencilOperationType::Replace:
            return VkStencilOp::VK_STENCIL_OP_REPLACE;

        case StencilOperationType::IncrementAndClamp:
            return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP;

        case StencilOperationType::DecrementAndClamp:
            return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP;

        case StencilOperationType::Invert:
            return VkStencilOp::VK_STENCIL_OP_INVERT;

        case StencilOperationType::IncrementAndWrap:
            return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP;

        case StencilOperationType::DecrementAndWrap:
            return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP;

        default:
            logError("Unsupported StencilOperationType '", static_cast<uint32_t>(op), "' defaulting to VkStencilOp::VK_STENCIL_OP_KEEP");
            return VkStencilOp::VK_STENCIL_OP_KEEP;
        }
    }

    StencilOperationType fromVkStencilOp(VkStencilOp op) noexcept
    {
        switch (op)
        {
        case VkStencilOp::VK_STENCIL_OP_KEEP:
            return StencilOperationType::Keep;

        case VkStencilOp::VK_STENCIL_OP_ZERO:
            return StencilOperationType::Zero;

        case VkStencilOp::VK_STENCIL_OP_REPLACE:
            return StencilOperationType::Replace;

        case VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP:
            return StencilOperationType::IncrementAndClamp;

        case VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP:
            return StencilOperationType::DecrementAndClamp;

        case VkStencilOp::VK_STENCIL_OP_INVERT:
            return StencilOperationType::Invert;

        case VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP:
            return StencilOperationType::IncrementAndWrap;

        case VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP:
            return StencilOperationType::DecrementAndWrap;

        default:
            logError("Unsupported VkStencilOp '", op, "' defaulting to StencilOperationType::Keep");
            return StencilOperationType::Keep;
        }
    }

    // -------------------------------------------------------------------------------------
    // LogicOperationType <-> VkLogicOp
    // -------------------------------------------------------------------------------------

    VkLogicOp toVkLogicOp(LogicOperationType op) noexcept
    {
        switch (op)
        {
        case LogicOperationType::Clear:
            return VkLogicOp::VK_LOGIC_OP_CLEAR;

        case LogicOperationType::And:
            return VkLogicOp::VK_LOGIC_OP_AND;

        case LogicOperationType::AndReverse:
            return VkLogicOp::VK_LOGIC_OP_AND_REVERSE;

        case LogicOperationType::Copy:
            return VkLogicOp::VK_LOGIC_OP_COPY;

        case LogicOperationType::AndInverted:
            return VkLogicOp::VK_LOGIC_OP_AND_INVERTED;

        case LogicOperationType::NoOp:
            return VkLogicOp::VK_LOGIC_OP_NO_OP;

        case LogicOperationType::Xor:
            return VkLogicOp::VK_LOGIC_OP_XOR;

        case LogicOperationType::Or:
            return VkLogicOp::VK_LOGIC_OP_OR;

        case LogicOperationType::Nor:
            return VkLogicOp::VK_LOGIC_OP_NOR;

        case LogicOperationType::Equivalent:
            return VkLogicOp::VK_LOGIC_OP_EQUIVALENT;

        case LogicOperationType::Invert:
            return VkLogicOp::VK_LOGIC_OP_INVERT;

        case LogicOperationType::OrReverse:
            return VkLogicOp::VK_LOGIC_OP_OR_REVERSE;

        case LogicOperationType::CopyInverted:
            return VkLogicOp::VK_LOGIC_OP_COPY_INVERTED;

        case LogicOperationType::OrInverted:
            return VkLogicOp::VK_LOGIC_OP_OR_INVERTED;

        case LogicOperationType::Nand:
            return VkLogicOp::VK_LOGIC_OP_NAND;

        case LogicOperationType::Set:
            return VkLogicOp::VK_LOGIC_OP_SET;

        default:
            logError("Unsupported LogicOperationType '", static_cast<uint32_t>(op), "' defaulting to VkLogicOp::VK_LOGIC_OP_CLEAR");
            return VkLogicOp::VK_LOGIC_OP_CLEAR;
        }
    }

    LogicOperationType fromVkLogicOp(VkLogicOp op) noexcept
    {
        switch (op)
        {
        case VkLogicOp::VK_LOGIC_OP_CLEAR:
            return LogicOperationType::Clear;

        case VkLogicOp::VK_LOGIC_OP_AND:
            return LogicOperationType::And;

        case VkLogicOp::VK_LOGIC_OP_AND_REVERSE:
            return LogicOperationType::AndReverse;

        case VkLogicOp::VK_LOGIC_OP_COPY:
            return LogicOperationType::Copy;

        case VkLogicOp::VK_LOGIC_OP_AND_INVERTED:
            return LogicOperationType::AndInverted;

        case VkLogicOp::VK_LOGIC_OP_NO_OP:
            return LogicOperationType::NoOp;

        case VkLogicOp::VK_LOGIC_OP_XOR:
            return LogicOperationType::Xor;

        case VkLogicOp::VK_LOGIC_OP_OR:
            return LogicOperationType::Or;

        case VkLogicOp::VK_LOGIC_OP_NOR:
            return LogicOperationType::Nor;

        case VkLogicOp::VK_LOGIC_OP_EQUIVALENT:
            return LogicOperationType::Equivalent;

        case VkLogicOp::VK_LOGIC_OP_INVERT:
            return LogicOperationType::Invert;

        case VkLogicOp::VK_LOGIC_OP_OR_REVERSE:
            return LogicOperationType::OrReverse;

        case VkLogicOp::VK_LOGIC_OP_COPY_INVERTED:
            return LogicOperationType::CopyInverted;

        case VkLogicOp::VK_LOGIC_OP_OR_INVERTED:
            return LogicOperationType::OrInverted;

        case VkLogicOp::VK_LOGIC_OP_NAND:
            return LogicOperationType::Nand;

        case VkLogicOp::VK_LOGIC_OP_SET:
            return LogicOperationType::Set;

        default:
            logError("Unsupported VkLogicOp '", op, "' defaulting to LogicOperationType::Clear");
            return LogicOperationType::Clear;
        }
    }

    // -------------------------------------------------------------------------------------
    // BlendOperationType <-> VkBlendOp
    // -------------------------------------------------------------------------------------

    VkBlendOp toVkBlendOp(BlendOperationType op) noexcept
    {
        switch (op)
        {
        case BlendOperationType::Add:
            return VkBlendOp::VK_BLEND_OP_ADD;

        case BlendOperationType::Subtract:
            return VkBlendOp::VK_BLEND_OP_SUBTRACT;

        case BlendOperationType::ReverseSubtract:
            return VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT;

        case BlendOperationType::Min:
            return VkBlendOp::VK_BLEND_OP_MIN;

        case BlendOperationType::Max:
            return VkBlendOp::VK_BLEND_OP_MAX;

        default:
            logError("Unsupported BlendOperationType '", static_cast<uint32_t>(op), "' defaulting to VK_BLEND_OP_ADD");
            return VkBlendOp::VK_BLEND_OP_ADD;
        }
    }

    BlendOperationType fromVkBlendOp(VkBlendOp op) noexcept
    {
        switch (op)
        {
        case VkBlendOp::VK_BLEND_OP_ADD:
            return BlendOperationType::Add;

        case VkBlendOp::VK_BLEND_OP_SUBTRACT:
            return BlendOperationType::Subtract;

        case VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT:
            return BlendOperationType::ReverseSubtract;

        case VkBlendOp::VK_BLEND_OP_MIN:
            return BlendOperationType::Min;

        case VkBlendOp::VK_BLEND_OP_MAX:
            return BlendOperationType::Max;

        default:
            logError("Unsupported VkBlendOp '", op, "' defaulting to BlendOperationType::Add");
            return BlendOperationType::Add;
        }
    }

    // -------------------------------------------------------------------------------------
    // BlendFactor <-> VkBlendFactor
    // -------------------------------------------------------------------------------------

    VkBlendFactor toVkBlendFactor(BlendFactor factor) noexcept
    {
        switch (factor)
        {
        case BlendFactor::Zero:
            return VkBlendFactor::VK_BLEND_FACTOR_ZERO;

        case BlendFactor::One:
            return VkBlendFactor::VK_BLEND_FACTOR_ONE;

        case BlendFactor::SrcColor:
            return VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR;

        case BlendFactor::OneMinusSrcColor:
            return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;

        case BlendFactor::DstColor:
            return VkBlendFactor::VK_BLEND_FACTOR_DST_COLOR;

        case BlendFactor::OneMinusDstColor:
            return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;

        case BlendFactor::SrcAlpha:
            return VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA;

        case BlendFactor::OneMinusSrcAlpha:
            return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        case BlendFactor::DstAlpha:
            return VkBlendFactor::VK_BLEND_FACTOR_DST_ALPHA;

        case BlendFactor::OneMinusDstAlpha:
            return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;

        case BlendFactor::ConstantColor:
            return VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR;

        case BlendFactor::OneMinusConstantColor:
            return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;

        case BlendFactor::ConstantAlpha:
            return VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_ALPHA;

        case BlendFactor::OneMinusConstantAlpha:
            return VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;

        case BlendFactor::SrcAlphaSaturate:
            return VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;

        default:
            logError("Unsupported BlendFactor '", static_cast<uint32_t>(factor), "' defaulting to VK_BLEND_FACTOR_ZERO");
            return VkBlendFactor::VK_BLEND_FACTOR_ZERO;
        }
    }

    BlendFactor fromVkBlendFactor(VkBlendFactor factor) noexcept
    {
        switch (factor)
        {
        case VkBlendFactor::VK_BLEND_FACTOR_ZERO:
            return BlendFactor::Zero;

        case VkBlendFactor::VK_BLEND_FACTOR_ONE:
            return BlendFactor::One;

        case VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR:
            return BlendFactor::SrcColor;

        case VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
            return BlendFactor::OneMinusSrcColor;

        case VkBlendFactor::VK_BLEND_FACTOR_DST_COLOR:
            return BlendFactor::DstColor;

        case VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
            return BlendFactor::OneMinusDstColor;

        case VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA:
            return BlendFactor::SrcAlpha;

        case VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
            return BlendFactor::OneMinusSrcAlpha;

        case VkBlendFactor::VK_BLEND_FACTOR_DST_ALPHA:
            return BlendFactor::DstAlpha;

        case VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
            return BlendFactor::OneMinusDstAlpha;

        case VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR:
            return BlendFactor::ConstantColor;

        case VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
            return BlendFactor::OneMinusConstantColor;

        case VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_ALPHA:
            return BlendFactor::ConstantAlpha;

        case VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
            return BlendFactor::OneMinusConstantAlpha;

        case VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
            return BlendFactor::SrcAlphaSaturate;

        default:
            logError("Unsupported VkBlendFactor '", factor, "' defaulting to BlendFactor::Zero");
            return BlendFactor::Zero;
        }
    }

    // -------------------------------------------------------------------------------------
    // ColorComponentFlag <-> VkColorComponentFlags
    // -------------------------------------------------------------------------------------

    VkColorComponentFlags toVkColorComponentFlag(ColorComponentFlag flag) noexcept
    {
        VkColorComponentFlags vkFlag = 0;

        if ((flag & static_cast<ColorComponentFlag>(ColorComponentFlagBits::R)) != 0) { vkFlag |= VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT; }
        if ((flag & static_cast<ColorComponentFlag>(ColorComponentFlagBits::G)) != 0) { vkFlag |= VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT; }
        if ((flag & static_cast<ColorComponentFlag>(ColorComponentFlagBits::B)) != 0) { vkFlag |= VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT; }
        if ((flag & static_cast<ColorComponentFlag>(ColorComponentFlagBits::A)) != 0) { vkFlag |= VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT; }

        return vkFlag;
    }

    ColorComponentFlag fromVkColorComponentFlag(VkColorComponentFlags flag) noexcept
    {
        ColorComponentFlag litlFlag = 0;

        if ((flag & VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT) != 0) { litlFlag |= static_cast<ColorComponentFlag>(ColorComponentFlagBits::R); }
        if ((flag & VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT) != 0) { litlFlag |= static_cast<ColorComponentFlag>(ColorComponentFlagBits::G); }
        if ((flag & VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT) != 0) { litlFlag |= static_cast<ColorComponentFlag>(ColorComponentFlagBits::B); }
        if ((flag & VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT) != 0) { litlFlag |= static_cast<ColorComponentFlag>(ColorComponentFlagBits::A); }

        return litlFlag;
    }

    // -------------------------------------------------------------------------------------
    // DynamicStateFlag <-> VkDynamicState
    // -------------------------------------------------------------------------------------

    VkDynamicState toVkDynamicState(DynamicState state) noexcept
    {
        switch (state)
        {
        case DynamicState::LineWidth:
            return VkDynamicState::VK_DYNAMIC_STATE_LINE_WIDTH;

        case DynamicState::DepthBias:
            return VkDynamicState::VK_DYNAMIC_STATE_DEPTH_BIAS;

        case DynamicState::BlendConstants:
            return VkDynamicState::VK_DYNAMIC_STATE_BLEND_CONSTANTS;

        case DynamicState::StencilRef:
            return VkDynamicState::VK_DYNAMIC_STATE_STENCIL_REFERENCE;

        case DynamicState::CullMode:
            return VkDynamicState::VK_DYNAMIC_STATE_CULL_MODE;

        default:
            logError("Unsupported DynamicState of '", static_cast<uint32_t>(state), "' defaulting to 0");
            return static_cast<VkDynamicState>(0);
        }
    }

    DynamicState fromVkDynamicState(VkDynamicState state) noexcept
    {
        switch (state)
        {
        case VkDynamicState::VK_DYNAMIC_STATE_LINE_WIDTH:
            return DynamicState::LineWidth;

        case VkDynamicState::VK_DYNAMIC_STATE_DEPTH_BIAS:
            return DynamicState::DepthBias;

        case VkDynamicState::VK_DYNAMIC_STATE_BLEND_CONSTANTS:
            return DynamicState::BlendConstants;

        case VkDynamicState::VK_DYNAMIC_STATE_STENCIL_REFERENCE:
            return DynamicState::StencilRef;

        case VkDynamicState::VK_DYNAMIC_STATE_CULL_MODE:
            return DynamicState::CullMode;

        default:
            logError("Unsupported VkDynamicState of '", static_cast<uint32_t>(state), "' defaulting to DynamicState::None");
            return DynamicState::None;
        }
    }

    // -------------------------------------------------------------------------------------
    // DynamicStateFlag <-> VkDynamicState
    // -------------------------------------------------------------------------------------

    VkVertexInputRate toVkVertexInputRate(VertexInputRate rate) noexcept
    {
        switch (rate)
        {
        case VertexInputRate::PerVertex:
            return VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

        case VertexInputRate::PerInstance:
            return VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;

        default:
            logError("Unsupported VertexInputRate of '", static_cast<uint32_t>(rate), "' defaulting to 0");
            return static_cast<VkVertexInputRate>(0);
        }
    }

    VertexInputRate fromVkVertexInputRate(VkVertexInputRate rate) noexcept
    {
        switch (rate)
        {
        case VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX:
            return VertexInputRate::PerVertex;

        case VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE:
            return VertexInputRate::PerInstance;

        default:
            logError("Unsupported VkVertexInputRate of '", static_cast<uint32_t>(rate), "' defaulting to 0");
            return static_cast<VertexInputRate>(0);
        }
    }

    // -------------------------------------------------------------------------------------
    // BufferTypeFlag <-> VkBufferUsageFlags2
    // -------------------------------------------------------------------------------------

    VkBufferUsageFlags2 toVkBufferUsageFlag(BufferTypeFlag flag) noexcept
    {
        VkBufferUsageFlags2 vkFlag = 0;

        if (has_any(flag, BufferTypeFlagBits::VertexBuffer)) { vkFlag |= VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT; }
        if (has_any(flag, BufferTypeFlagBits::IndexBuffer)) { vkFlag |= VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT; }
        if (has_any(flag, BufferTypeFlagBits::UniformBuffer)) { vkFlag |= VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT; }
        if (has_any(flag, BufferTypeFlagBits::StorageBuffer)) { vkFlag |= VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT; }
        if (has_any(flag, BufferTypeFlagBits::TransferSource)) { vkFlag |= VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT; }
        if (has_any(flag, BufferTypeFlagBits::TransferDest)) { vkFlag |= VK_BUFFER_USAGE_2_TRANSFER_DST_BIT; }
        if (has_any(flag, BufferTypeFlagBits::ShaderDeviceAddress)) { vkFlag |= VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT; }

        return vkFlag;
    }

    BufferTypeFlag fromVkBufferUsageFlag(VkBufferUsageFlags2 flag) noexcept
    {
        BufferTypeFlag litlFlag = BufferTypeFlag::None;

        if ((flag & VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT) != 0) { litlFlag |= BufferTypeFlagBits::VertexBuffer; }
        if ((flag & VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT) != 0) { litlFlag |= BufferTypeFlagBits::IndexBuffer; }
        if ((flag & VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT) != 0) { litlFlag |= BufferTypeFlagBits::UniformBuffer; }
        if ((flag & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) != 0) { litlFlag |= BufferTypeFlagBits::StorageBuffer; }
        if ((flag & VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT) != 0) { litlFlag |= BufferTypeFlagBits::TransferSource; }
        if ((flag & VK_BUFFER_USAGE_2_TRANSFER_DST_BIT) != 0) { litlFlag |= BufferTypeFlagBits::TransferDest; }
        if ((flag & VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT) != 0) { litlFlag |= BufferTypeFlagBits::ShaderDeviceAddress; }

        return litlFlag;
    }

    // -------------------------------------------------------------------------------------
    // BufferMemoryType <-> VmaMemoryUsage
    // -------------------------------------------------------------------------------------

    [[nodiscard]] VmaMemoryUsage toVmaMemoryUsage(BufferMemoryType usage) noexcept
    {
        switch (usage)
        {
        case BufferMemoryType::Auto:
            return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;

        case BufferMemoryType::PreferGpu:
            return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        case BufferMemoryType::PreferCpu:
            return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

        case BufferMemoryType::Unknown:
        default:
            return VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN;
        }
    }

    [[nodiscard]] BufferMemoryType fromVmaMemoryUsage(VmaMemoryUsage usage) noexcept
    {
        switch (usage)
        {
        case VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO:
            return BufferMemoryType::Auto;

        case VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE:
            return BufferMemoryType::PreferGpu;

        case VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST:
            return BufferMemoryType::PreferCpu;

        case VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN:
        default:
            return BufferMemoryType::Unknown;
        }
    }

    VmaAllocationCreateFlags toVmaAllocationCreateFlag(BufferMemoryUsage usage) noexcept
    {
        // See: https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html
        switch (usage)
        {
        case BufferMemoryUsage::GpuOnly:
            return VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;                      // own dedicated gpu memory block

        case BufferMemoryUsage::Staging:
            return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |         // allow to map the allocation using vmaMapMemory and it will only be written to sequentially (memcpy) or a loop writing number-by-number, never random access
                   VMA_ALLOCATION_CREATE_MAPPED_BIT;                                // memory will be persistently mapped and to retrieve a pointer to it
        
        case BufferMemoryUsage::ReadBack:
            return VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |                   // memory can be read, written, and accessed in random order
                   VMA_ALLOCATION_CREATE_MAPPED_BIT;                                // memory will be persistently mapped and to retrieve a pointer to it
        
        case BufferMemoryUsage::PersistentMap:
            return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |         // allow to map the allocation using vmaMapMemory and it will only be written to sequentially (memcpy) or a loop writing number-by-number, never random access
                   VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |   // will need to check where the allocation ended up and act accordingly
                   VMA_ALLOCATION_CREATE_MAPPED_BIT;                                // memory will be persistently mapped and to retrieve a pointer to it

        default:
            logError("Unsupported VmaAllocationCreateFlags of '", static_cast<uint32_t>(usage), "' defaulting to 0");
            return static_cast<VmaAllocationCreateFlags>(0);
        }
    }

    // -------------------------------------------------------------------------------------
    // SharingMode <-> VkSharingMode
    // -------------------------------------------------------------------------------------

    [[nodiscard]] VkSharingMode toVkSharingMode(SharingMode mode) noexcept
    {
        switch (mode)
        {
        case SharingMode::Exclusive:
            return VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

        case SharingMode::Concurrent:
            return VkSharingMode::VK_SHARING_MODE_CONCURRENT;

        default:
            logError("Unsupported SharingMode of '", static_cast<uint32_t>(mode), "' defaulting to 0");
            return static_cast<VkSharingMode>(0);
        }
    }

    [[nodiscard]] SharingMode fromVkSharingMode(VkSharingMode mode) noexcept
    {
        switch (mode)
        {
        case VkSharingMode::VK_SHARING_MODE_EXCLUSIVE:
            return SharingMode::Exclusive;

        case VkSharingMode::VK_SHARING_MODE_CONCURRENT:
            return SharingMode::Concurrent;

        default:
            logError("Unsupported VkSharingMode of '", static_cast<uint32_t>(mode), "' defaulting to 0");
            return static_cast<SharingMode>(0);
        }
    }

    // -------------------------------------------------------------------------------------
    // .dstAccess and .dstStage from Pipeline Usage Flag
    // -------------------------------------------------------------------------------------


    VkPipelineStageFlags2 deriveDstStageFromBufferType(BufferTypeFlag flag) noexcept
    {
        return deriveDstStageFromUsageFlag(toVkBufferUsageFlag(flag));
    }

    VkAccessFlags2 deriveDstAccessFromBufferType(BufferTypeFlag flag) noexcept
    {
        return deriveDstAccessFromUsageFlag(toVkBufferUsageFlag(flag));
    }

    VkPipelineStageFlags2 deriveDstStageFromUsageFlag(VkBufferUsageFlags2 usage) noexcept
    {
        VkPipelineStageFlags2 dstStage = 0;

        if (usage & VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT) { dstStage |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT; }
        if (usage & VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT) { dstStage |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT; }
        if (usage & VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT) { dstStage |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT; }
        if (usage & VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT) { dstStage |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; }        // can't know which shader stage
        if (usage & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) { dstStage |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT; }        // compute or graphics

        return dstStage;
    }

    VkAccessFlags2 deriveDstAccessFromUsageFlag(VkBufferUsageFlags2 usage) noexcept
    {
        VkAccessFlags2 dstAccess = 0;

        if (usage & VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT) { dstAccess |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT; }
        if (usage & VK_BUFFER_USAGE_2_INDEX_BUFFER_BIT) { dstAccess |= VK_ACCESS_2_INDEX_READ_BIT; }
        if (usage & VK_BUFFER_USAGE_2_INDIRECT_BUFFER_BIT) { dstAccess |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT; }
        if (usage & VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT) { dstAccess |= VK_ACCESS_2_UNIFORM_READ_BIT; }               // can't know which shader stage
        if (usage & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT) { dstAccess |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT; }        // compute or graphics

        return dstAccess;
    }

    // -------------------------------------------------------------------------------------
    // color -> VkClearColorValue
    // -------------------------------------------------------------------------------------

    VkClearColorValue toVkClearColorValue(color color) noexcept
    {
        return VkClearColorValue{ { color.r(), color.g(), color.b(), color.a() } };
    }
}