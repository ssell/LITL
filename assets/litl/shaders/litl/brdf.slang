// -----------------------------------------------------------------------------------------
// [brdf.slang]
// The Bidrectional Reflectance Distribution Function (BRDF) used by the LITL PBR lighting model.
// -----------------------------------------------------------------------------------------

// Throughout:
//     V = normalized directional vector pointing from the fragment surface to the camera.
//     L = normalized directional vector pointing from the fragment surface to the light source.
// The shorthand letter names are kept to maintain consistence with exteranal sources and references.

module brdf;
import common;

public struct SurfaceData
{
    public float3 normal = float3(0.0f, 0.0f, 0.0f);
    public float3 albedo = float3(1.0f, 1.0f, 1.0f);
    public float occlusion = 1.0f;
    public float roughness = 1.0f;
    public float metallic = 0.0f;
};

public float3 evaluateBRDF(SurfaceData surface, float3 V, float3 L)
{
    return surface.albedo * (1.0f / PI);
};