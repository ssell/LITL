// -----------------------------------------------------------------------------------------
// [brdf.slang]
// The Bidrectional Reflectance Distribution Function (BRDF) used by the LITL PBR lighting model.
// -----------------------------------------------------------------------------------------

/**
 * The following notation is used throughout.
 * This is consistent with the various references used (such as glTF 2.0 specs).
 *
 *     V = normalized vector from the shading location to the eye.
 *     L = normalized vector from the shading location to the light.
 *     N = surface normal in the same space as the above values.
 *     H = half vector, where H = normalize(L + V)
 *     AoB = A dot B
 */

module brdf;
import common;

static const float MIN_ALPHA = 0.002f;      // Minimum α value. ~= perceptual roughness 0.045; avoids D -> inf in fp16
static const float MIN_NOV = 1e-4;          // Minimum (normal dot toView) value.

// Resolved from material textures and properties. All vectors are expected to be normalized and in world-space.
public struct SurfaceData
{
    public float3 normal;               // Shading normal, make sure to flip for back faces if applicable.
    public float3 toView;               // Direction from the surface to the camera/view.
    public float3 baseColor;            // Linear base color of the surface.
    public float occlusion;             // 
    public float roughness;             // Roughness value on the range [0, 1]
    public float metallic;              // Metallic value on the range [0, 1]
};

// BRDF parameters derived once per fragment and reusedfor every light.
public struct BrdfParams
{
    public float3 diffuseColor;         // Interpolated surface base color based on the metallic factor. Metals have no diffuse color.
    public float3 f0;                   // Input to the fresnel term. Surface reflectance at normal incidence, aka 0 degrees aka looking straight at the normal.
    public float alpha;                 // α = clamped(roughness * roughness). Most visible roughness changes happen near 0, so artist authored roughness on [0, 1] is perceptual and needs to be scaled down.
    public float NoV;                   // On the range [0, 1] where 0 = looking at the surfacee edge-on at a grazing angle, and 1 = looking straight down at the surface.
};

/**
 * Builds up the BrdfParams from the surface data.
 * Call once per-fragment, not once per-light.
 */
public BrdfParams prepareBRDF(in SurfaceData surface)
{
    const float roughness = saturate(surface.roughness);

    BrdfParams brdf;
    brdf.diffuseColor = surface.baseColor * (1.0f - surface.metallic);
    brdf.f0    = lerp(float3(0.04f), surface.baseColor, surface.metallic);         // 0.04 = dielectric incidence of radiance (IOR) 1.5
    brdf.alpha = max(roughness * roughness, MIN_ALPHA);
    brdf.NoV   = max(abs(dot(surface.normal, surface.toView)), MIN_NOV);

    return brdf;
}

/**
 * GGX/Trowbridge-Reitz NDF
 *
 * Calculates the specular highlighting from surface roughness.
 * Roughness lies on the range [0.0, 1.0], with lower values producing a smoother, "glossier", surface.
 * Higher values produce a rougher surface with the specular lighting distributed over a larger surface area.
 */
float calculateNDF(float NoH, float alpha)
{
    float a2 = alpha * alpha;
    float f = 1.0f + NoH * ((NoH * a2) - NoH);

    return a2 / (PI * f * f);
}

/**
 * Height-correlated Smith-GGX visibility (Heitz 2014).
 * V = G / (4 NoL NoV); The microfacet denominator is folded in.
 */
float calculateSmithGGXCorrelatedVisibility(float NoV, float NoL, float alpha)
{
    float a2   = alpha * alpha;
    float ggxV = NoL * sqrt(NoV * NoV * (1.0f - a2) + a2);
    float ggxL = NoV * sqrt(NoL * NoL * (1.0f - a2) + a2);

    return 0.5f / (ggxV + ggxL);
}

/**
 * Fast approximation of the height-correlated Smith-GGX visibility (Hammon 2017).
 * Removes both sqrt calls present in the more accurate, but slower, version.
 */
float calculateSmithGGXCorrelatedVisibilityFast(float NoV, float NoL, float alpha)
{
    float ggxV = NoL * (NoV * (1.0f - alpha) + alpha);
    float ggxL = NoV * (NoL * (1.0f - alpha) + alpha);

    return 0.5f / (ggxV + ggxL);
}

/**
 * Calculates the Fresnel reflectivity.
 * The surface metallic parameters controls the initial incidence value (f0).
 */
float3 calculateSclickFresnel(float VoH, float3 f0)
{
    float m  = 1.0f - VoH;
    float m2 = m * m;
    float m5 = m2 * m2 * m;

    return f0 + m5 * (1.0f - f0);
}

/**
 * Outgoing radiance from one light, prior to shadowing.
 * L = normalized surface to light vector, radiance = light color * intensity.
 */
public float3 evaluateBRDF(in SurfaceData surface, in BrdfParams brdf, float3 L, float3 radiance)
{
    float NoL = saturate(dot(surface.normal, L));

    if (NoL <= 0.0f)
    {
        return float3(0.0f);
    }

    float3 H  = halfVector(surface.toView, L);
    float NoH = saturate(dot(surface.normal, H));
    float VoH = saturate(dot(surface.toView, H));

    float D  = calculateNDF(NoH, brdf.alpha);
    float V  = calculateSmithGGXCorrelatedVisibility(brdf.NoV, NoL, brdf.alpha);
    float3 F = calculateSclickFresnel(VoH, brdf.f0);

    float3 specular = D * V * F;
    float3 diffuse = (1.0f - F) * brdf.diffuseColor * ONE_OVER_PI;          // Match glTF spec mix

    return (diffuse + specular) * radiance * NoL;
};