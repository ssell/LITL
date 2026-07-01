#ifndef LITL_ENGINE_RENDERING_VERTEX_BUFFER_H__
#define LITL_ENGINE_RENDERING_VERTEX_BUFFER_H__

#include "litl-engine/rendering/buffer/gpuBuffer.hpp"
#include "litl-engine/rendering/types/vertex.hpp"

namespace litl
{
    class VertexBuffer
    {
    public:

    private:

        GpuBuffer<Vertex> m_buffer;
    };
}

#endif