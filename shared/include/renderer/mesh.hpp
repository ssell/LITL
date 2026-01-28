#ifndef LITL_RENDERER_MESH_H__
#define LITL_RENDERER_MESH_H__

#include "renderer/renderer.hpp"
#include "math/vertex.hpp"

namespace LITL::Renderer
{
    class Mesh
    {
    public:

        Mesh(RenderContext const* const pRenderContext);
        ~Mesh();

        Mesh(Mesh const& other) = delete;
        Mesh& operator=(Mesh other) = delete;

    protected:

    private:

        bool createVertexBuffer() const;

        MeshContext* m_pContext;
    };
}

#endif 