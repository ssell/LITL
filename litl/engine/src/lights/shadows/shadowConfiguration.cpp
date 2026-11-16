#include "litl-core/math/common.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/lights/shadows/shadowConfiguration.hpp"

namespace litl
{
    void ShadowConfiguration::sanitize()
    {
        if (!between(resolution, MinResolution, MaxResolution))
        {
            logWarning("Shadow settings 'resolution' is outside of acceptable range of [", MinResolution, ", ", MaxResolution, "]. Clamping.");
            resolution = clamp(resolution, MinResolution, MaxResolution);
        }

        if (!between(directionalLayers, MinDirectionalLayers, MaxDirectionalLayers))
        {
            logWarning("Shadow settings 'resolution' is outside of acceptable range of [", MinDirectionalLayers, ", ", MaxDirectionalLayers, "]. Clamping.");
            resolution = clamp(directionalLayers, MinDirectionalLayers, MaxDirectionalLayers);
        }

        if (!between(directionalShadowDistance, MinDirectionalShadowDistance, MaxDirectionalShadowDistance))
        {
            logWarning("Shadow settings 'directionalShadowDistance' is outside of acceptable range of [", MinDirectionalShadowDistance, ", ", MaxDirectionalShadowDistance, "]. Clamping.");
            resolution = clamp(directionalShadowDistance, MinDirectionalShadowDistance, MaxDirectionalShadowDistance);
        }

        if (!between(directionalCasterPullback, MinDirectionalCasterPullback, MaxDirectionalCasterPullback))
        {
            logWarning("Shadow settings 'directionalCasterPullback' is outside of acceptable range of [", MinDirectionalCasterPullback, ", ", MaxDirectionalCasterPullback, "]. Clamping.");
            resolution = clamp(directionalCasterPullback, MinDirectionalCasterPullback, MaxDirectionalCasterPullback);
        }

        if (!between(cascadeSplitLambda, MinCascadeSplitLambda, MaxCascadeSplitLambsa))
        {
            logWarning("Shadow settings 'cascadeSplitLambda' is outside of acceptable range of [", MinCascadeSplitLambda, ", ", MaxCascadeSplitLambsa, "]. Clamping.");
            resolution = clamp(cascadeSplitLambda, MinCascadeSplitLambda, MaxCascadeSplitLambsa);
        }
    }
}