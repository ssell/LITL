#include "renderer/queueFamily.hpp"

namespace LITL
{
    void QueueFamilyIndices::setGraphicsIndex(uint32_t index) noexcept
    {
        m_graphicsIndex = index;
        m_hasGraphicsIndex = true;
    }

    uint32_t QueueFamilyIndices::getGraphicsIndex() const noexcept
    {
        return m_graphicsIndex;
    }

    bool QueueFamilyIndices::hasGraphicsIndex() const noexcept
    {
        return m_hasGraphicsIndex;
    }

    void QueueFamilyIndices::setPresentIndex(uint32_t index) noexcept
    {
        m_presentIndex = index;
        m_hasPresentIndex = true;
    }

    uint32_t QueueFamilyIndices::getPresentIndex() const noexcept
    {
        return m_presentIndex;
    }

    bool QueueFamilyIndices::hasPresentIndex() const noexcept
    {
        return m_hasPresentIndex;
    }

    bool QueueFamilyIndices::hasAll() noexcept
    {
        return m_hasGraphicsIndex && m_hasPresentIndex;
    }
}