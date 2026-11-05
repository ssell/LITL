// -----------------------------------------------------------------------------------------
// [lighting.slang]
// Lighting specific module. Containts light structures, functions, etc.
// -----------------------------------------------------------------------------------------

module lighting;

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