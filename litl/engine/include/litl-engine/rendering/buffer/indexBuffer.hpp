#ifndef LITL_ENGINE_RENDERING_INDEX_BUFFER_H__
#define LITL_ENGINE_RENDERING_INDEX_BUFFER_H__

#include <cstdint>
#include "litl-engine/rendering/buffer/gpuBuffer.hpp"

namespace litl
{
    class IndexBuffer
    {
    public:

    private:

        GpuBuffer<uint32_t> m_buffer;
    };
}

#endif