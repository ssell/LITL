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

    RenderViewType RenderView::resolveType(Camera* camera) noexcept
    {
        if (camera != nullptr)
        {
            switch (camera->getCameraType())
            {
            case CameraType::DepthOnly:
                return RenderViewType::DepthOnly;

            case CameraType::Standard:
            default:
                return RenderViewType::Camera;
            }
        }
        else
        {
            return RenderViewType::Shadow;
        }
    }
}