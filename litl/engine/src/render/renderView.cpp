#include "litl-engine/render/renderView.hpp"
#include "litl-engine/objects/camera.hpp"

namespace litl
{
    ResolvedView ResolvedView::fromCamera(Camera const& camera) noexcept
    {
        return ResolvedView{
            .viewMat = camera.getViewMatrix(),
            .projMat = camera.getProjectionMatrix(),
            .viewProjMat = camera.getViewProjectionMatrix(),
            .position = camera.getWorldPosition(),
            .viewport = rect2D{ .offset = camera.getDescriptor().viewport.offset, .extents = camera.getDescriptor().viewport.extents },
            .frustum = camera.getFrustum(),
            .colorEnabled = camera.getColorEnabled(),
            .depthEnabled = camera.getDepthEnabled(),
            .clearColor = camera.getClearColor(),
            .near = camera.getNear(),
            .far = camera.getFar()
        };
    }

    ResolvedView ResolvedView::fromShadowView(ShadowView const& view) noexcept
    {
        return ResolvedView{
            .viewMat = view.viewMatrix,
            .projMat = view.projMatrix,
            .viewProjMat = view.viewProjMatrix,
            .position = view.eyePosition,
            .frustum = view.cullFrustum,
            .colorEnabled = false,
            .depthEnabled = true,
            .clearColor = colors::Black,
        };
    }
}