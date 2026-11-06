// -----------------------------------------------------------------------------------------
// [lighting.slang]
// Lighting specific module. Containts light structures, functions, etc.
// -----------------------------------------------------------------------------------------

module lighting;
__exported import brdf;

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

public float3 shadeDirectionalLights(LightData* lightData, SurfaceData surface, float3 toViewDir)
{
    float3 total = 0;

    for (uint i = 0; i < lightData->directionalLightCount; ++i)
    {
        const DirectionalLight light = lightData->directionalLights[i];
        const float3 L = light.toLight;
        const float NdotL = saturate(dot(surface.normal, L));
        const float3 radiance = light.colorIntensity.rgb * light.colorIntensity.a;

        // ... todo multiply by per-light shadow/visibility factor ...

        total += evaluateBRDF(surface, toViewDir, L) * radiance * NdotL;
    }

    return total;
}