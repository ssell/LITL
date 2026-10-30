#include "litl-engine/render/passes/depthOnlyPass.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/objects/material/material.hpp"

namespace litl
{
    void DepthOnlyPass::render(RenderPassContext const& context) noexcept
    {
        for (const RenderView& renderView : context.renderViews)
        {
            if (renderView.type == RenderViewType::Shadow)
            {
                // ... todo ...
            }
        }
    }

    GraphicsPipelineHandle DepthOnlyPass::resolveGraphicsPipelineHandle(Material* material) const
    {
        return material->getGraphicsPipelineHandle(MaterialVariantType::DepthOnly, m_pRenderManager->getSceneRenderFormats(), true);
    }
}