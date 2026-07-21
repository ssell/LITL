#ifndef LITL_ENGINE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_PASS_H__

#include <vector>
#include "litl-engine/render/renderableEntity.hpp"

namespace litl
{
    class Camera;
    class Renderer;
    class ObjectPool;

    class RenderPass
    {
    public:

        void render(Renderer const& renderer, ObjectPool& objectPool, Camera* camera, std::vector<RenderableEntity> const& entities) const noexcept;
    };
}

#endif