// -----------------------------------------------------------------------------------------
// [common.slang]
// Core shader functionality and utility for both internal and external LITL shaders.
// -----------------------------------------------------------------------------------------

module common;

public static const uint BindSetPerFrame = 0;
public static const uint BindSetPerPass = 1;
public static const uint BindSetPerMaterial = 2;
public static const uint BindSetPerObject = 3;

public static const float PI          = 3.14159265359f;
public static const float PI_OVER_TWO = 1.57079632679f;
public static const float ONE_OVER_PI = 0.31830988618f;
public static const float EPSILON     = 1e-6;

// -----------------------------------------------------------------------------------------
// Utility Functions
// -----------------------------------------------------------------------------------------

/**
 * Calculates the vector (h) half-way inbetween the light (l) and view (v).
 *
 *       v\   |h  /l
 *         \  |  /
 *          \ | /
 *           \|/
 * -------------------------
 */
public float3 halfVector(float3 V, float3 L)
{
    return normalize(V + L);
}

// Cofactor of M: proportional to the inverse-transpose of M but without an inverse needed. Row i of cofactor(M) = cross(row[i + 1], row[i + 2])
public float3x3 cofactor(float3x3 m)
{
    return float3x3(
        cross(m[1], m[2]),
        cross(m[2], m[0]),
        cross(m[0], m[1]));
}