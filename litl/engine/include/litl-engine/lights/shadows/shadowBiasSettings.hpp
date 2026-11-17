#ifndef LITL_ENGINE_LIGHTS_SHADOW_BIAS_SETTINGS_H__
#define LITL_ENGINE_LIGHTS_SHADOW_BIAS_SETTINGS_H__

namespace litl
{
    struct ShadowBiasSettings
    {
        /// <summary>
        /// Scalar factor controlling the constant depth value added to each fragment.
        /// Typically on the range of [-1.0, -4.0]. Negative values as we use a reversed-z.
        /// </summary>
        float depthBiasConstantFactor{ -1.0f };

        /// <summary>
        /// Scalar factor applied to a fragment’s slope in depth bias calculations.
        /// Typically on the range [-1.5, -2.5]. Negative values as we use a reversed-z.
        /// </summary>
        float depthBiasSlopeFactor{ -1.5f };

        /// <summary>
        /// The maximum (or minimum) depth bias of a fragment.
        /// 0 = no clamp.
        /// </summary>
        float depthBiasClamp{ 0.0f };

        /// <summary>
        /// 
        /// </summary>
        float normalOffsetTexels{ 1.0f };
    };
}

#endif