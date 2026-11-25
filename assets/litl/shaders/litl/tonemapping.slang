// -----------------------------------------------------------------------------------------
// [tonemapping.slang]
// Tone mapping implementations.
// -----------------------------------------------------------------------------------------

module tonemapping;

// Must match ToneMapper in engine/lights/tonemapping.hpp
public static const uint ToneMapNone = 0;
public static const uint ToneMapACES = 1;
public static const uint ToneMapKhronosNeutralPBR = 2;

// -----------------------------------------------------------------------------------------
// ACES
// -----------------------------------------------------------------------------------------

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat = float3x3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat = float3x3(
    1.60475, -0.53108, -0.07367,
    -0.10208, 1.10813, -0.00605,
    -0.00327, -0.07276, 1.07602);

float3 RRTAndODTFit(float3 v)
{
    const float3 a = v * (v + 0.0245786) - 0.000090537;
    const float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

float3 tonemapACESFitted(float3 color)
{
    color = mul(ACESInputMat, color);
    color = RRTAndODTFit(color);
    color = mul(ACESOutputMat, color);
    return saturate(color);
}

// -----------------------------------------------------------------------------------------
// Khronos Neutral PBR
// -----------------------------------------------------------------------------------------

float3 tonemapKhronosNeutralPBR(float3 color)
{
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

    const float x = min(color.r, min(color.g, color.b));
    const float offset = (x < 0.08) ? (x - 6.25 * x * x) : 0.04;
    color -= offset;

    const float peak = max(color.r, max(color.g, color.b));

    if (peak < startCompression)
    {
        return color;
    }

    const float d = 1.0 - startCompression;
    const float newPeak = 1.0 - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    const float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
    return lerp(color, float3(newPeak), g);
}

// -----------------------------------------------------------------------------------------
// Apply
// -----------------------------------------------------------------------------------------

public float3 applyTonemap(float3 hdr, uint tonemapper)
{
    switch (tonemapper)
    {
    case ToneMapACES:
        return tonemapACESFitted(hdr);

    case ToneMapKhronosNeutralPBR:
        return tonemapKhronosNeutralPBR(hdr);

    case ToneMapNone:
    default:
        return saturate(hdr);
    }
}