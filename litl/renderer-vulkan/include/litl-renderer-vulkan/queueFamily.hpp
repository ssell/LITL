#ifndef LITL_VULKAN_RENDERER_QUEUE_FAMILIY_H__
#define LITL_VULKAN_RENDERER_QUEUE_FAMILIY_H__

#include <cstdint>

namespace litl::vulkan
{
    class QueueFamilyIndices
    {
    public:

        void setGraphicsIndex(uint32_t index) noexcept
        {
            m_graphicsIndex = index;
            m_hasGraphicsIndex = true;
        }

        [[nodiscard]] uint32_t getGraphicsIndex() const noexcept
        {
            return m_graphicsIndex;
        }

        [[nodiscard]] bool hasGraphicsIndex() const noexcept
        {
            return m_hasGraphicsIndex;
        }

        void setPresentIndex(uint32_t index) noexcept
        {
            m_presentIndex = index;
            m_hasPresentIndex = true;
        }

        [[nodiscard]] uint32_t getPresentIndex() const noexcept
        {
            return m_presentIndex;
        }

        [[nodiscard]] bool hasPresentIndex() const noexcept
        {
            return m_hasPresentIndex;
        }

        [[nodiscard]] bool hasAll() noexcept
        {
            return m_hasGraphicsIndex && m_hasPresentIndex;
        }

    private:

        uint32_t m_graphicsIndex;
        uint32_t m_presentIndex;

        bool m_hasGraphicsIndex;
        bool m_hasPresentIndex;
    };
}

#endif