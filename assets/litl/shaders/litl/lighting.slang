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

float3 shadeDirectionalLights(LightData* lightData, in SurfaceData surface, in BrdfParams brdf)
{
    float3 total = 0;

    for (uint i = 0; i < lightData->directionalLightCount; ++i)
    {
        const DirectionalLight light = lightData->directionalLights[i];
        const float3 L = light.toLight;
        const float NdotL = saturate(dot(surface.normal, L));
        const float3 radiance = light.colorIntensity.rgb * light.colorIntensity.a;

        float3 lighting = evaluateBRDF(surface, brdf, L, radiance);

        // ... todo multiply by per-light shadow/visibility factor ...

        total += lighting;
    }

    return total;
}

public float3 shadeLights(LightData* lightData, in SurfaceData surface)
{
    const BrdfParams brdf = prepareBRDF(surface);

    const float3 directionalLighting = shadeDirectionalLights(lightData, surface, brdf);
    // ... todo other light types ...
    // float3 lighting = directionalLighting + ...;

    return directionalLighting;
}