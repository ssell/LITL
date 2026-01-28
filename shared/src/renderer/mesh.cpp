#include <array>
#include "renderer/common.inl"
#include "renderer/mesh.hpp"

namespace LITL::Renderer
{
    Mesh::Mesh(RenderContext const* const pRenderContext)
        : m_pContext(new MeshContext{})
    {
        m_pContext->device = pRenderContext->device;
    }

    Mesh::~Mesh()
    {
        if (m_pContext->vertexBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(m_pContext->device, m_pContext->vertexBuffer, nullptr);
            m_pContext->vertexBuffer = VK_NULL_HANDLE;
        }

        delete m_pContext;
    }

    bool Mesh::createVertexBuffer() const
    {
        const auto bufferInfo = VkBufferCreateInfo {
            .size = sizeof(Math::Vertex) * m_pContext->vertexCount,
            .usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE
        };

        const auto result = vkCreateBuffer(m_pContext->device, &bufferInfo, nullptr, &m_pContext->vertexBuffer);

        return (result == VK_SUCCESS);
    }

    // -------------------------------------------------------------------------------------
    // Common Descriptors
    // -------------------------------------------------------------------------------------

    // https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/00_Vertex_input_description.html
    static VkVertexInputBindingDescription getCommonVertexBindingDescription()
    {
        return { 0, sizeof(Math::Vertex), VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX };
    }

    static std::array<VkVertexInputAttributeDescription, 2> getCommonVertexAttributeDescriptions()
    {
        return
        {
            VkVertexInputAttributeDescription(0, 0, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offsetof(Math::Vertex, position)),
            VkVertexInputAttributeDescription(1, 0, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offsetof(Math::Vertex, color))
        };
    }
}