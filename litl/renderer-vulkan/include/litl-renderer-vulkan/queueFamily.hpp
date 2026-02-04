#ifndef LITL_VULKAN_RENDERER_QUEUE_FAMILIY_H__
#define LITL_VULKAN_RENDERER_QUEUE_FAMILIY_H__

#include <cstdint>

namespace LITL::Vulkan::Renderer
{
    class QueueFamilyIndices
    {
    public:

        void setGraphicsIndex(uint32_t index) noexcept;
        uint32_t getGraphicsIndex() const noexcept;
        bool hasGraphicsIndex() const noexcept;

        void setPresentIndex(uint32_t index) noexcept;
        uint32_t getPresentIndex() const noexcept;
        bool hasPresentIndex() const noexcept;

        bool hasAll() noexcept;

    private:

        uint32_t m_graphicsIndex;
        uint32_t m_presentIndex;

        bool m_hasGraphicsIndex;
        bool m_hasPresentIndex;
    };
}

#endif