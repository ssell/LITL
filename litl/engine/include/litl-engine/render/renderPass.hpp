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

namespace litl
{
    class Renderer;
    class ObjectPool;
    class Camera;

    class RenderPass
    {
    public:

        RenderPass(RenderPass const&) = delete;
        RenderPass& operator=(RenderPass const&) = delete;

        RenderPass() = default;
        virtual ~RenderPass() = default;

        virtual void setup(Renderer& renderer, ObjectPool& objectPool) noexcept;
        virtual void render(RenderPassContext const& context) noexcept = 0;

    protected:

        [[nodiscard]] bool compileRenderList(std::span<RenderableEntity const> entities) noexcept;
        [[nodiscard]] bool createRenderPassItems(RenderableEntity entity, uint32_t instanceOffset, std::optional<MeshHandle>& currMeshHandle, std::optional<MaterialHandle>& currMaterialHandle) noexcept;
        [[nodiscard]] virtual GraphicsPipelineHandle resolveGraphicsPipelineHandle(Material* material) const noexcept = 0;
        [[nodiscard]] BeginRenderCommand createBeginRenderCommand(Camera const& camera) const noexcept;
        [[nodiscard]] SetViewportAndScissorCommand createSetViewportAndScissorCommand(Camera const& camera) const noexcept;

        Renderer* m_pRenderer{ nullptr };
        ObjectPool* m_pObjectPool{ nullptr };
        std::vector<RenderPassItem> m_renderPassItems;

    private:

    };
}

#endif