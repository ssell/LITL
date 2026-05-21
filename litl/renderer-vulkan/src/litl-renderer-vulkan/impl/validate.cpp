#include "litl-renderer/enums.hpp"
#include "litl-renderer-vulkan/renderer.hpp"

// validate our renderer enums against the vulkan enums/bits

namespace litl
{
    // ImageLayoutType
    static_assert(static_cast<uint32_t>(ImageLayoutType::Undefined) == VK_IMAGE_LAYOUT_UNDEFINED);
    static_assert(static_cast<uint32_t>(ImageLayoutType::General) == VK_IMAGE_LAYOUT_GENERAL);
    static_assert(static_cast<uint32_t>(ImageLayoutType::Color) == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    static_assert(static_cast<uint32_t>(ImageLayoutType::DepthStencil) == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    static_assert(static_cast<uint32_t>(ImageLayoutType::DepthStencilReadOnly) == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
    static_assert(static_cast<uint32_t>(ImageLayoutType::ShaderReadOnly) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    static_assert(static_cast<uint32_t>(ImageLayoutType::TransferSrc) == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    static_assert(static_cast<uint32_t>(ImageLayoutType::TransferDst) == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    static_assert(static_cast<uint32_t>(ImageLayoutType::Preinitialized) == VK_IMAGE_LAYOUT_PREINITIALIZED);
    static_assert(static_cast<uint32_t>(ImageLayoutType::Present) == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // ImageAccessFlagBits
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::None) == VK_ACCESS_2_NONE);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::IndirectCommandRead) == VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::IndexRead) == VK_ACCESS_2_INDEX_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::VertexAttributeRead) == VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::UniformRead) == VK_ACCESS_2_UNIFORM_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::InputAttachmentRead) == VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::ShaderRead) == VK_ACCESS_2_SHADER_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::ShaderWrite) == VK_ACCESS_2_SHADER_WRITE_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::ColorRead) == VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::ColorWrite) == VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::DepthStencilRead) == VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::DepthStencilWrite) == VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::TransferRead) == VK_ACCESS_2_TRANSFER_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::TransferWrite) == VK_ACCESS_2_TRANSFER_WRITE_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::HostRead) == VK_ACCESS_2_HOST_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::HostWrite) == VK_ACCESS_2_HOST_WRITE_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::MemoryRead) == VK_ACCESS_2_MEMORY_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::MemoryWrite) == VK_ACCESS_2_MEMORY_WRITE_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::ShaderSampledRead) == VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::ShaderStorageRead) == VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
    static_assert(static_cast<uint64_t>(ImageAccessFlagBits::ShaderStorageWrite) == VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);

    // PipelineStageFlagBits
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::None) == VK_PIPELINE_STAGE_2_NONE);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::TopOfPipe) == VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::DrawIndirect) == VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::VertexInput) == VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::VertexShader) == VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::TessellationControlShader) == VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::TessellationEvaluationShader) == VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::GeometryShader) == VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::FragmentShader) == VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::EarlyFragmentTests) == VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::LateFragmentTests) == VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::ColorAttachmentOutput) == VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::ComputeShader) == VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::Transfer) == VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::BottomOfPipe) == VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::Host) == VK_PIPELINE_STAGE_2_HOST_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::AllGraphics) == VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::AllCommands) == VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::Copy) == VK_PIPELINE_STAGE_2_COPY_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::Resolve) == VK_PIPELINE_STAGE_2_RESOLVE_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::Blit) == VK_PIPELINE_STAGE_2_BLIT_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::Clear) == VK_PIPELINE_STAGE_2_CLEAR_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::IndexInput) == VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::VertexAttributeInput) == VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT);
    static_assert(static_cast<uint64_t>(PipelineStageFlagBits::PreRasterizationShaders) == VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT);

    // LoadOperationType
    static_assert(static_cast<uint32_t>(LoadOperationType::Load) == VK_ATTACHMENT_LOAD_OP_LOAD);
    static_assert(static_cast<uint32_t>(LoadOperationType::Clear) == VK_ATTACHMENT_LOAD_OP_CLEAR);
    static_assert(static_cast<uint32_t>(LoadOperationType::DontCare) == VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    static_assert(static_cast<uint32_t>(LoadOperationType::None) == VK_ATTACHMENT_LOAD_OP_NONE);

    // StoreOperationType
    static_assert(static_cast<uint32_t>(StoreOperationType::Store) == VK_ATTACHMENT_STORE_OP_STORE);
    static_assert(static_cast<uint32_t>(StoreOperationType::DontCare) == VK_ATTACHMENT_STORE_OP_DONT_CARE);
    static_assert(static_cast<uint32_t>(StoreOperationType::None) == VK_ATTACHMENT_STORE_OP_NONE);
}