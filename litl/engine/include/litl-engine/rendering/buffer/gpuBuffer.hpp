#ifndef LITL_ENGINE_RENDERING_GPU_BUFFER_H__
#define LITL_ENGINE_RENDERING_GPU_BUFFER_H__

#include <vector>
#include "litl-renderer/resources/buffer.hpp"

namespace litl
{
    template<typename T>
    class GpuBuffer
    {
    public:

        void setData(std::span<T const> source) noexcept
        {
            m_data.clear();
            m_data.assign(source.begin(), source.end());
        }

        [[nodiscard]] std::span<T const> getData() const noexcept
        {
            return m_data;
        }

    private:

        BufferHandle m_handle{};
        std::vector<T> m_data;
    };
}

#endif