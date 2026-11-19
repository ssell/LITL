#ifndef LITL_ENGINE_LIGHTS_SHADOW_CONFIGURATION_H__
#define LITL_ENGINE_LIGHTS_SHADOW_CONFIGURATION_H__

#include <cstdint>

#include "litl-engine/constants.hpp"
#include "litl-engine/lights/shadows/shadowBiasSettings.hpp"

namespace litl
{
    struct ShadowConfiguration
    {
        static constexpr uint32_t MinResolution = 32u;
        static constexpr uint32_t MaxResolution = 4096u;
        static constexpr uint32_t MinDirectionalLayers = 1u;
        static constexpr uint32_t MaxDirectionalLayers = 1024u;
        static constexpr float MinDirectionalShadowDistance = 1.0f;
        static constexpr float MaxDirectionalShadowDistance = 131072.0f;
        static constexpr float MinDirectionalCasterPullback = 0.0f;
        static constexpr float MaxDirectionalCasterPullback = 131072.0f;
        static constexpr float MinCascadeSplitLambda = 0.0f;
        static constexpr float MaxCascadeSplitLambsa = 1.0f;

        /// <summary>
        /// Shadow map resolution.
        /// As maps are stored in a texture array this resolution is shared by all maps.
        /// </summary>
        uint32_t resolution{ 2048u };

        /// <summary>
        /// The array size of the directional light shadow map.
        /// This corresponds to the maximimum direction light shadow rendering cameras * maximum cascades.
        /// Default of 2 cameras * max cascades.
        /// </summary>
        uint32_t directionalLayers{ 2u * constants::MaxShadowCascades };

        /// <summary>
        /// Max view distance that receives directional shadows.
        /// </summary>
        float directionalShadowDistance{ 100.0f };

        /// <summary>
        /// How far behind the fitted view box casters are still culled in.
        /// </summary>
        float directionalCasterPullback{ 50.0f };

        /// <summary>
        /// 0 = uniform splits, 1 = logarithmic.
        /// </summary>
        float cascadeSplitLambda{ 0.75f };

        /// <summary>
        /// Default depth bias for directional light shadows.
        /// </summary>
        ShadowBiasSettings defaultDirectionalBias{};

        /// <summary>
        /// Default depth bias for spot light shadows.
        /// </summary>
        ShadowBiasSettings defaultSpotBias{};

        /// <summary>
        /// Default depth bias for point light shadows.
        /// </summary>
        ShadowBiasSettings defaultPointBias{};
        
        void sanitize() noexcept;
    };
}

#endif