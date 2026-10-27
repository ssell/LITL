#ifndef LITL_ENGINE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_PASS_H__

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "litl-engine/render/renderableEntity.hpp"
#include "litl-engine/render/passes/renderPassItem.hpp"

namespace litl
{
    class Renderer;
    class ObjectPool;
    struct RenderPassContext;

    class RenderPass
    {
    public:

        virtual ~RenderPass() = default;
        virtual void setup(Renderer& renderer, ObjectPool& objectPool) noexcept;
        virtual void render(RenderPassContext& context) noexcept = 0;

    protected:

        [[nodiscard]] bool compileRenderList(std::span<RenderableEntity const> entities) noexcept;
        [[nodiscard]] bool createRenderPassItems(RenderableEntity entity, uint32_t instanceOffset, std::vector<RenderPassItem>& m_renderPassItems, std::optional<MeshHandle>& currMeshHandle, std::optional<MaterialHandle>& currMaterialHandle) noexcept;

        static constexpr uint32_t MaxRenderWaitTimeMs = 1000u;

        Renderer* m_pRenderer{ nullptr };
        ObjectPool* m_pObjectPool{ nullptr };
        std::vector<RenderPassItem> m_renderPassItems;

    private:

    };
}

#endif