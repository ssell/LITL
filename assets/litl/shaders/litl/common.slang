// -----------------------------------------------------------------------------------------
// [common.slang]
// Core shader functionality and utility for both internal and external LITL shaders.
// -----------------------------------------------------------------------------------------

module common;

public static const uint BindSetPerFrame = 0;
public static const uint BindSetPerPass = 1;
public static const uint BindSetPerMaterial = 2;
public static const uint BindSetPerObject = 3;

public static const float PI          = 3.1415926f;
public static const float PI_OVER_TWO = 1.5707963f;
public static const float ONE_OVER_PI = 0.3183098f;
public static const float EPSILON     = 0.000001f;

// -----------------------------------------------------------------------------------------
// Utility Functions
// -----------------------------------------------------------------------------------------

// Cofactor of M: proportional to the inverse-transpose of M but without an inverse needed. Row i of cofactor(M) = cross(row[i + 1], row[i + 2])
public float3x3 cofactor(float3x3 m)
{
    return float3x3(
        cross(m[1], m[2]),
        cross(m[2], m[0]),
        cross(m[0], m[1]));
}