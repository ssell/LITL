#ifndef LITL_ENGINE_RENDERING_MESH_H__
#define LITL_ENGINE_RENDERING_MESH_H__

#include "litl-engine/rendering/buffer/vertexBuffer.hpp"
#include "litl-engine/rendering/buffer/indexBuffer.hpp"

namespace litl
{
    class Mesh
    {
    public:

    private:

        VertexBuffer m_vertexBuffer;
        IndexBuffer m_indexBuffer;
    };
}

#endif