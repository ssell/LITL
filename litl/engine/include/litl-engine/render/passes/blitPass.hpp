#ifndef LITL_ENGINE_RENDER_BLIT_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_BLIT_RENDER_PASS_H__

#include <vector>

#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/render/tonemapping.hpp"
#include "litl-renderer/resources/handles.hpp"
#include "litl-renderer/resources/shaderModuleTypes.hpp"
namespace litl
{
    class AssetManager;

    /// <summary>
    /// The final pass that blits the ongoing render texture to the swapchain.
    /// </summary>
    class BlitPass : public RenderPass
    {
    public:

        void setup(ServiceProvider& services, RenderManager& renderManager, ObjectPool& objectPool) noexcept override;
        void render(RenderPassContext const& context) noexcept override;
        void setToneMapper(ToneMapper tonemapper) noexcept;
        void setToneMapExposure(float exposure) noexcept;
        [[nodiscard]] ToneMapper getToneMapper() const noexcept;
        [[nodiscard]] float getToneMapExposure() const noexcept;

    private:

        [[nodiscard]] bool barrierAndBeginRender(CommandBufferHandle commandBufferHandle) noexcept;
        [[nodiscard]] bool ensurePipelineExists() noexcept;

        AssetManager* m_pAssetManager{ nullptr };
        ShaderAssetHandle m_shaderAssetHandle{};
        GraphicsPipelineHandle m_graphicsPipelineHandle{};
        ToneMapper m_tonemapper{ ToneMapper::None };
        float m_tonemapExposure{ 1.0f };
        bool m_requireManualSRGB{ false };
    };
}

#endif