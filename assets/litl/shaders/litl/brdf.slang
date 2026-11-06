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
    public float3 normal;
    public float3 albedo;
    public float roughness;
    public float metallic;
};

public float3 evaluateBRDF(SurfaceData surface, float3 V, float3 L)
{
    return surface.albedo * (1.0f / PI);
};