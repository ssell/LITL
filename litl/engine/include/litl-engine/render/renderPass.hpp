#ifndef LITL_ENGINE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_PASS_H__

#include <memory>
#include <vector>

#include "litl-engine/render/renderableEntity.hpp"
#include "litl-renderer/resources/commandBuffer.hpp"

namespace litl
{
    class Camera;
    class Renderer;
    class ObjectPool;
    struct RenderPushConstants;

    class RenderPass
    {
    public:

        RenderPass();
        ~RenderPass();

        void setup(Renderer& renderer, ObjectPool& objectPool) noexcept;
        void render(CommandBufferHandle frameCommandBuffer, RenderPushConstants const& pushConstants, Camera& camera, std::vector<RenderableEntity> const& entities) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif