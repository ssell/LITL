#ifndef LITL_QUEUE_FAMILY_H__
#define LITL_QUEUE_FAMILY_H__

#include <cstdint>

namespace LITL::Renderer
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