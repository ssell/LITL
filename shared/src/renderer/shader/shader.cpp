#include <fstream>
#include <vector>

#include "renderer/common.inl"
#include "renderer/shader/shader.hpp"

namespace LITL::Renderer
{
    Shader::Shader(RenderContext const* const pContext, ShaderStage shaderStage)
        : m_pContext(new ShaderContext{}), m_ShaderStage(shaderStage)
    {
        m_pContext->device = pContext->device;
    }

    Shader::~Shader()
    {
        if (m_pContext->shaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(m_pContext->device, m_pContext->shaderModule, nullptr);
            m_pContext->shaderModule = VK_NULL_HANDLE;
        }

        delete m_pContext;
    }

    bool Shader::load(const char* path, const char* entry)
    {
        if (m_pContext->shaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(m_pContext->device, m_pContext->shaderModule, nullptr);
            m_pContext->shaderModule = VK_NULL_HANDLE;
        }

        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            // todo log out error
            return false;
        }

        auto fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> fileBuffer(fileSize);

        file.seekg(0);
        file.read(fileBuffer.data(), fileSize);
        file.close();

        const auto createShaderInfo = VkShaderModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = fileSize,
            .pCode = reinterpret_cast<const uint32_t*>(fileBuffer.data())
        };

        VkResult result = vkCreateShaderModule(m_pContext->device, &createShaderInfo, nullptr, &m_pContext->shaderModule);

        if (result != VK_SUCCESS)
        {
            // todo log out error
            return false;
        }

        static const VkShaderStageFlagBits ShaderStageFlags[6]{
            VK_SHADER_STAGE_VERTEX_BIT,
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            VK_SHADER_STAGE_GEOMETRY_BIT,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            VK_SHADER_STAGE_COMPUTE_BIT
        };

        m_pContext->shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        m_pContext->shaderStageInfo.stage = ShaderStageFlags[static_cast<uint32_t>(m_ShaderStage)];
        m_pContext->shaderStageInfo.module = m_pContext->shaderModule;
        m_pContext->shaderStageInfo.pName = entry;

        return true;
    }

    ShaderContext const* Shader::getContext() const noexcept
    {
        return m_pContext;
    }
}