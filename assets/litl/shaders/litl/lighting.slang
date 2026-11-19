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

// Must match ShadowRenderViewData.hpp
public struct ShadowRenderViewData
{
    public float4x4 cascadeViewProj[MAX_SHADOW_CASCADES];
    public float4 cascadeSplits;
    public float4 cascadeNormalOffset;
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

float calcDirectionalLightShadow(in float3 L, in ShadowInputs shadow, uint lightIndex)
{
    if ((shadow.data == nullptr) || 
        (shadow.data->shadowArrayIndex == UINT32_NULL_INDEX) || 
        (shadow.data->lightIndex != lightIndex) || 
        (shadow.data->cascadeCount == 0))
    {
        return 1.0f;
    }

    for (uint cidx = 0; cidx < MAX_SHADOW_CASCADES; ++cidx)
    {
        if (shadow.viewDepth < shadow.data->cascadeSplits[cidx])
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

            // TODO pass these in
            const float2 resolution = float2(2048.0f);
            const float2 invResolution = float2(0.00048828125f); // 1 / 2048
            const uint layer = shadow.data->firstLayer + cidx;

            return sampleGlobalShadowTextureArray5x5(shadow.data->shadowArrayIndex, uv, layer, ref, resolution, invResolution);
        }
    }

    return 1.0f;
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