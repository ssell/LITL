#ifndef LITL_ENGINE_RENDER_MANAGER_H__
#define LITL_ENGINE_RENDER_MANAGER_H__

#include <memory>
#include <queue>

#include "litl-core/authority.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderAttachmentFormats.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class Engine;
    class EngineCallbacks;
    class ObjectPool;
    class Window;
    class GpuBuffer;
    class Texture;

    struct CullingBucket;
    struct RendererConfiguration;

    class RenderManager
    {
    public:

        RenderManager();
        ~RenderManager();

        void setup(Authority<Engine> authority, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> authority) noexcept;
        void onRender(Authority<EngineCallbacks> authority, float dt) noexcept;
        [[nodiscard]] Renderer const* getRenderer() const noexcept;
        void trackDirtyBuffer(Authority<GpuBuffer> auth, GpuBufferHandle handle) noexcept;
        void trackDirtyTexture(Authority<Texture> auth, TextureHandle handle) noexcept;

        /// <summary>
        /// Blocks until the GPU is idle.
        /// </summary>
        void halt() noexcept;
        
        /// <summary>
        /// The data format used by the scene, which may differ from the format of the swapchain.
        /// For example, for HDR lighting the scene format may be RGBA16_SFloat while the swapchain is still RGBA8_UNorm.
        /// </summary>
        [[nodiscard]] RenderAttachmentFormats getSceneRenderFormats() const noexcept;

        /// <summary>
        /// The data formats used by the renderer swapchain.
        /// </summary>
        [[nodiscard]] RenderAttachmentFormats getSwapchainRenderFormats() const noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif