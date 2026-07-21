#ifndef LITL_ENGINE_RENDER_MANAGER_H__
#define LITL_ENGINE_RENDER_MANAGER_H__

#include <memory>
#include <queue>

#include "litl-core/authority.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class Engine;
    class EngineCallbacks;
    class ObjectPool;
    class Window;
    class GpuBuffer;

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
        void trackDirtyBuffer(Authority<GpuBuffer> auth, GpuBufferHandle handle) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif