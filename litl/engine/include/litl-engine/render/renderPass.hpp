#ifndef LITL_ENGINE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_PASS_H__

#include <vector>
#include "litl-engine/render/renderableEntity.hpp"
#include "litl-renderer/resources/commandBuffer.hpp"

namespace litl
{
    class Camera;
    class Renderer;
    class ObjectPool;

    class RenderPass
    {
    public:

        void render(Renderer const& renderer, CommandBufferHandle frameCommandBuffer, ObjectPool& objectPool, Camera* camera, std::vector<RenderableEntity> const& entities) const noexcept;
    };
}

#endif