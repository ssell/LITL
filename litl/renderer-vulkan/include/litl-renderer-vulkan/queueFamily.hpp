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

        void setTransferIndex(uint32_t index) noexcept
        {
            m_transferIndex = index;
            m_hasTransferIndex = true;
        }

        [[nodiscard]] uint32_t getTransferIndex() const noexcept
        {
            return m_transferIndex;
        }

        [[nodiscard]] bool hasTransferIndex() const noexcept
        {
            return m_hasTransferIndex;
        }

        [[nodiscard]] bool hasAll() noexcept
        {
            return m_hasGraphicsIndex && m_hasPresentIndex && m_hasTransferIndex;
        }

    private:

        uint32_t m_graphicsIndex = 0u;
        uint32_t m_presentIndex = 0u;
        uint32_t m_transferIndex = 0u;

        bool m_hasGraphicsIndex = false;
        bool m_hasPresentIndex = false;
        bool m_hasTransferIndex = false;
    };
}

#endif