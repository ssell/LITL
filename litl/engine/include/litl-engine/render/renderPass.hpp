#ifndef LITL_ENGINE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_PASS_H__

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "litl-engine/render/renderableEntity.hpp"
#include "litl-engine/render/renderPassContext.hpp"
#include "litl-engine/render/passes/renderPassItem.hpp"
#include "litl-renderer/commands/beginRenderCommand.hpp"
#include "litl-renderer/commands/setViewportAndScissorCommand.hpp"
#include "litl-renderer/commands/pipelineBarrierCommand.hpp"

namespace litl
{
    class ServiceProvider;
    class RenderManager;
    class Renderer;
    class ObjectPool;
    class Camera;
    class Texture;

    class RenderPass
    {
    public:

        RenderPass(RenderPass const&) = delete;
        RenderPass& operator=(RenderPass const&) = delete;

        RenderPass() = default;
        virtual ~RenderPass() = default;

        virtual void setup(ServiceProvider& services, RenderManager& renderManager, ObjectPool& objectPool) noexcept;
        virtual void destroy() noexcept;
        virtual void render(RenderPassContext const& context) noexcept = 0;

    protected:

        /// <summary>
        /// The underlying color and depth textures for a RenderView's camera render texture.
        /// </summary>
        struct RenderViewSources
        {
            Texture* color{ nullptr };
            Texture* depth{ nullptr };
        };

        /// <summary>
        /// The resolved attachments for one RenderView's rendering scope, plus the barriers that must be in place.
        /// </summary>
        struct RenderTargetSetup
        {
            static constexpr uint32_t MaxBarriers{ 2u };        // color + depth for now
            BeginRenderCommand beginRenderCmd{};
            std::array<PipelineBarrierCommand, MaxBarriers> pipelineBarrierCmds{};
            uint32_t barrierCount{ 0u };                        // number of barriers between [0, MaxBarriers]
            bool targetsSwapchain{ false };
            bool valid{ false };

            [[nodiscard]] std::span<PipelineBarrierCommand const> getPipelineBarrierCmds() const noexcept;
        };

        struct RenderTargetOptions
        {
            LoadOperationType loadOp{ LoadOperationType::Clear };
            StoreOperationType colorStoreOp{ StoreOperationType::Store };
            StoreOperationType depthStoreOp{ StoreOperationType::DontCare };
            bool hasColorAttachment{ true };
            bool hasDepthAttachment{ true };
        };

        [[nodiscard]] bool compileRenderList(std::span<RenderableEntity const> entities, bool depthOnly) noexcept;
        [[nodiscard]] bool createRenderPassItems(RenderableEntity entity, uint32_t instanceOffset, std::optional<MeshHandle>& currMeshHandle, std::optional<MaterialHandle>& currMaterialHandle, bool depthOnly) noexcept;
        [[nodiscard]] virtual GraphicsPipelineHandle resolveGraphicsPipelineHandle(Material* material, bool depthOnly) const noexcept;
        [[nodiscard]] SetViewportAndScissorCommand createSetViewportAndScissorCommand(Camera const* camera, bool applyCameraViewport) const noexcept;
        void resolveSourceTextures(Camera const* camera, Texture** colorSource, Texture** depthSource) const noexcept;
        void gatherSourceTexturesIntoScratch(RenderPassContext const& context) noexcept;
        [[nodiscard]] RenderTargetSetup resolveRenderTarget(Camera const* camera, RenderTargetOptions const& options) const noexcept;

        const RenderManager* m_pRenderManager{ nullptr };
        const Renderer* m_pRenderer{ nullptr };
        ObjectPool* m_pObjectPool{ nullptr };
        std::vector<RenderPassItem> m_renderPassItems;
        std::vector<RenderViewSources> m_renderViewSourcesScratch;

    private:

    };
}

#endif