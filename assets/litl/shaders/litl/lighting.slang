// -----------------------------------------------------------------------------------------
// [lighting.slang]
// Lighting specific module. Containts light structures, functions, etc.
// -----------------------------------------------------------------------------------------

module lighting;
import common;
import textures;

__exported import brdf;

// -----------------------------------------------------------------------------------------
// Shadows
// -----------------------------------------------------------------------------------------

// Must match constants::MaxShadowCascades
public static const uint MAX_SHADOW_CASCADES = 4;

// Must match CascadeBlendFraction in ShadowManager
static const float CASCADE_BLEND_FRACTION = 0.1f;       // 10% of the cascade's depth range

// Must match ShadowRenderViewData.hpp
public struct ShadowRenderViewData
{
    public float4x4 cascadeViewProj[MAX_SHADOW_CASCADES];
    public float4 cascadeSplits;
    public float4 cascadeNormalOffset;
    public float resolution;
    public float resolutionInverse;
    public uint shadowArrayIndex;
    public uint firstLayer;
    public uint cascadeCount;
    public uint lightIndex;
}

public struct ShadowInputs
{
    public ShadowRenderViewData* data;
    public float3 positionWS;
    public float3 geometricNormal;
    public float viewDepth;
};

public float calcDirectionalLightShadowCascadeIndex(in ShadowInputs shadow)
{
    uint cidx = 0;

    // Select the first cadcade whose far split is beyond this fragment
    for (; cidx < shadow.data->cascadeCount; ++cidx)
    {
        if (shadow.viewDepth < shadow.data->cascadeSplits[cidx])
        {
            break;
        }
    }

    return cidx;
}

float sampleCascade(in ShadowRenderViewData* data, in ShadowInputs shadow, float3 L, uint cidx)
{
    // Normal offset. Push the lookup position along the geometric normal before projecting.
    // This removes most acne without inflating the depth bias. Scaling by (1 - NoL) keeps lit-facing surfaces tight
    // Note: the C++ side pre-scales the cascadeNormalOffset by world texel size.
    const float NoLg = saturate(dot(shadow.geometricNormal, L));
    const float3 posOffset = shadow.positionWS + shadow.geometricNormal * (shadow.data->cascadeNormalOffset[cidx] * (1.0f - NoLg));

    // Project into the light. The shadow view for directional is orthographic, so w == 1 and no divide is needed.
    // Additionally, the orthographic projection already negates [1][1] for our Vulkan backend, so NDC +y points down and lines up with texture v, so no flip needed.
    const float4 clip = mul(shadow.data->cascadeViewProj[cidx], float4(posOffset, 1.0f));
    const float2 uv = clip.xy * 0.5f + 0.5f;

    // reverse-Z: 1 = at the light's near plane, 0 = far. Outside of [0,1] needs no code: ClampBorder + OpaqueBlack returns "lit"
    const float ref = saturate(clip.z);
    const uint layer = shadow.data->firstLayer + cidx;

    return sampleGlobalShadowTextureArray5x5(
        shadow.data->shadowArrayIndex,
        uv,
        layer,
        ref,
        float2(shadow.data->resolution),
        float2(shadow.data->resolutionInverse));
}

float calcDirectionalLightShadow(in float3 L, in ShadowInputs shadow, uint lightIndex)
{
    if ((shadow.data == nullptr) || 
        (shadow.data->shadowArrayIndex == UINT32_NULL_INDEX) || 
        (shadow.data->lightIndex != lightIndex) || 
        (shadow.data->cascadeCount == 0))
    {
        return 1.0f;
    }

    ShadowRenderViewData* data = shadow.data;
    const uint count = data->cascadeCount;
    const uint cidx = calcDirectionalLightShadowCascadeIndex(shadow);

    // If the furthest cascade no blending can/needs to be done.
    if (cidx == count)
    {
        return 1.0f;
    }

    // Blend weight: 0 for most of the cascade, ramping to 1 across the last 10% before the next split.
    const float cascadeStart = (cidx == 0) ? 0.0f : data->cascadeSplits[cidx - 1];
    const float cascadeEnd = data->cascadeSplits[cidx];
    const float band = (cascadeEnd - cascadeStart) * CASCADE_BLEND_FRACTION;
    const float frac = saturate((shadow.viewDepth) - (cascadeEnd - band)) / band;

    const float cascadeSampleA = sampleCascade(data, shadow, L, cidx);

    if (frac <= 0.0f)
    {
        return cascadeSampleA;
    }

    const float cascadeSampleB = ((cidx + 1) < count) ? sampleCascade(data, shadow, L, cidx + 1) : 1.0f;

    return lerp(cascadeSampleA, cascadeSampleB, frac);
}

// -----------------------------------------------------------------------------------------
// Lights
// -----------------------------------------------------------------------------------------

public struct DirectionalLight
{
    public float4 colorIntensity;                   // (color.r, color.g, color.b, intensity)
    public float3 toLight;                          // Normalized vector to the directional light.
    public uint shadowIndex;                        // Index to the shadowmap that corresponds to this directional light.
};

public struct LightData
{
    public float4 ambient;                          // (color.r, color.g, color.b, unused)
    public DirectionalLight* directionalLights;     // Pointer to the directional light buffer.
    public uint directionalLightCount;              // Number of directional lights;
}

float3 shadeDirectionalLights(LightData *lightData, in SurfaceData surface, in BrdfParams brdf, in ShadowInputs shadow)
{
    float3 total = 0;

    for (uint i = 0; i < lightData->directionalLightCount; ++i)
    {
        const DirectionalLight light = lightData->directionalLights[i];
        const float3 L = light.toLight;
        const float NdotL = saturate(dot(surface.normal, L));
        const float3 radiance = light.colorIntensity.rgb * light.colorIntensity.a;

        float3 lighting = evaluateBRDF(surface, brdf, L, radiance);
        float shadow = calcDirectionalLightShadow(L, shadow, i);
        // ... todo multiply by per-light shadow/visibility factor ...

        total += lighting * shadow;
    }

    return total;
}

public float3 shadeLights(LightData *lightData, in SurfaceData surface, in ShadowInputs shadow)
{
    const BrdfParams brdf = prepareBRDF(surface);

    const float3 directionalLighting = shadeDirectionalLights(lightData, surface, brdf, shadow);
    // ... todo other light types ...
    // float3 lighting = directionalLighting + ...;

    return directionalLighting;
}