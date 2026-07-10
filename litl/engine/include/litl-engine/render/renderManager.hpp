#ifndef LITL_ENGINE_RENDER_MANAGER_H__
#define LITL_ENGINE_RENDER_MANAGER_H__

#include "litl-core/authority.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"

namespace litl
{
    class Engine;
    class EngineCallbacks;
    class Window;
    struct CullingBucket;

    struct RendererConfiguration;

    class RenderManager
    {
    public:

        RenderManager();
        ~RenderManager();

        void setup(Authority<Engine> authority, ServiceProvider& services) noexcept;
        void onRender(Authority<EngineCallbacks> authority) noexcept;
        [[nodiscard]] Renderer const* getRenderer() const noexcept;

    private:

        void sortVisibleEntities(CullingBucket& cullingBucket) noexcept;
        void createRenderer(Window* window, RendererConfiguration const& rendererDescriptor) noexcept;

        Renderer* m_pRenderer{ nullptr };
        RenderPass m_renderPass{};
    };
}

#endif