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

        if (any(stages & ShaderStage::Vertex)) { flags |= VK_SHADER_STAGE_VERTEX_BIT; }
        if (any(stages & ShaderStage::Fragment)) { flags |= VK_SHADER_STAGE_FRAGMENT_BIT; }
        if (any(stages & ShaderStage::Geometry)) { flags |= VK_SHADER_STAGE_GEOMETRY_BIT; }
        if (any(stages & ShaderStage::TessellationControl)) { flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; }
        if (any(stages & ShaderStage::TessellationEvaluation)) { flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; }
        if (any(stages & ShaderStage::Compute)) { flags |= VK_SHADER_STAGE_COMPUTE_BIT; }
        if (any(stages & ShaderStage::Mesh)) { flags |= VK_SHADER_STAGE_MESH_BIT_EXT; }
        if (any(stages & ShaderStage::Task)) { flags |= VK_SHADER_STAGE_TASK_BIT_EXT; }

        return flags;
    }

    ShaderStage fromVkShaderStageFlags(VkShaderStageFlags flags) noexcept
    {
        ShaderStage stages = ShaderStage::None;

        if (flags & VK_SHADER_STAGE_VERTEX_BIT) { stages = stages | ShaderStage::Vertex; }
        if (flags & VK_SHADER_STAGE_FRAGMENT_BIT) { stages = stages | ShaderStage::Fragment; }
        if (flags & VK_SHADER_STAGE_GEOMETRY_BIT) { stages = stages | ShaderStage::Geometry; }
        if (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) { stages = stages | ShaderStage::TessellationControl; }
        if (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) { stages = stages | ShaderStage::TessellationEvaluation; }
        if (flags & VK_SHADER_STAGE_COMPUTE_BIT) { stages = stages | ShaderStage::Compute; }
        if (flags & VK_SHADER_STAGE_MESH_BIT_EXT) { stages = stages | ShaderStage::Mesh; }
        if (flags & VK_SHADER_STAGE_TASK_BIT_EXT) { stages = stages | ShaderStage::Task; }

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
            return VK_DESCRIPTOR_TYPE_SAMPLER;

        case ShaderResourceType::UniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        case ShaderResourceType::StorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        case ShaderResourceType::ImageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

        case ShaderResourceType::SampledImage:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

        case ShaderResourceType::StorageImage:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        case ShaderResourceType::InputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

        case ShaderResourceType::AccelerationStructure:
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

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
        case VK_DESCRIPTOR_TYPE_SAMPLER:
            return ShaderResourceType::Sampler;

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            return ShaderResourceType::UniformBuffer;

        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            return ShaderResourceType::StorageBuffer;

        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return ShaderResourceType::ImageBuffer;

        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return ShaderResourceType::SampledImage;

        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return ShaderResourceType::StorageImage;

        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return ShaderResourceType::InputAttachment;

        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return ShaderResourceType::AccelerationStructure;

        default:
            return ShaderResourceType::Unknown;
        }
    }

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

    }
}