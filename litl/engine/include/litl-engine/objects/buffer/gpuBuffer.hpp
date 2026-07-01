#ifndef LITL_ENGINE_OBJECTS_GPU_BUFFER_H__
#define LITL_ENGINE_OBJECTS_GPU_BUFFER_H__

#include <cstdint>
#include <span>
#include <vector>

#include "litl-engine/objects/buffer/gpuBufferDescriptor.hpp"
#include "litl-renderer/resources/buffer.hpp"

namespace litl
{
    class GpuBuffer
    {
    public:

        void setData(std::span<std::byte const> data) noexcept
        {
            m_data.clear();
            m_data.assign(data.begin(), data.end());
        }

        [[nodiscard]] std::span<std::byte const> getData() const noexcept
        {
            return m_data;
        }

    private:

        BufferHandle m_handle{};
        std::vector<std::byte> m_data;
    };
}

#endif