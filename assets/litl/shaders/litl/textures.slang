// -----------------------------------------------------------------------------------------
// [textures.slang]
// Common logic for texture-related functions and the global texture table.
// -----------------------------------------------------------------------------------------

module textures;
import common;

// -----------------------------------------------------------------------------------------
// Global Texture Utilities

// These bindings are defined in TextureTable::buildDescriptorSetLayout
[[vk::binding(0, BindSetPerFrame)]] Texture2D<float4> g_Textures[];                     // Most 2D textures end up here
[[vk::binding(1, BindSetPerFrame)]] Texture2DArray<float4> g_TextureArrays[];           // Texture arrays such as cascaded shadow maps
[[vk::binding(1, BindSetPerFrame)]] Texture2DArray<float> g_ShadowTextureArrays[];      // Aliased view of g_TextureArrays for depth compares.
[[vk::binding(2, BindSetPerFrame)]] TextureCubeArray<float4> g_TextureCubeArrays[];     // Cube arrays such as point light shadows
[[vk::binding(3, BindSetPerFrame)]] SamplerState g_Samplers[16];
[[vk::binding(3, BindSetPerFrame)]] SamplerComparisonState g_ComparisonSamplers[16];    // Aliased view of g_Samplers for comparison samplers.

// These correlate to TextureTableReservedIndices in litl-renderer
public static const uint DefaultTextureIndex_Pink = 0;
public static const uint DefaultTextureIndex_White = 1;
public static const uint DefaultTextureIndex_Black = 2;
public static const uint DefaultTextureIndex_Normal = 3;

// These correlate to SamplerPredefines in litl-renderer
public static const uint DefaultSamplerIndex_LinearRepeat = 0;
public static const uint DefaultSamplerIndex_LinearClamp = 1;
public static const uint DefaultSamplerIndex_LinearRepeatAniso = 2;
public static const uint DefaultSamplerIndex_LinearClampAniso = 3;
public static const uint DefaultSamplerIndex_NearestRepeat = 4;
public static const uint DefaultSamplerIndex_NearestClamp = 5;
public static const uint DefaultSamplerIndex_NearestClampNoMip = 6;
public static const uint DefaultSamplerIndex_LinearClampNoMip = 7;
public static const uint DefaultSamplerIndex_ShadowCompare = 8;
public static const uint DefaultSamplerIndex_LinearClampBorderWhite = 9;
public static const uint DefaultSamplerIndex_LinearClampBorderBlack = 10;
public static const uint DefaultSamplerIndex_LinearMirrorRepeat = 11;
public static const uint DefaultSamplerIndex_Reserved12 = 12;
public static const uint DefaultSamplerIndex_Reserved13 = 13;
public static const uint DefaultSamplerIndex_Reserved14 = 14;
public static const uint DefaultSamplerIndex_Reserved15 = 15;

public SamplerState getGlobalSampler(uint samplerIndex)
{
    return g_Samplers[samplerIndex];
}

uint unpackTextureIndex(uint packedTextureSlotSamplerIndex)
{
    // The texture slot occupies the lower 24-bits and the sampler index is in the upper 8-bits.
    // This must be kept in sync with packing/unpacking functions in litl-renderer/utility.hpp
    return packedTextureSlotSamplerIndex & 0x00FFFFFF;
}

uint unpackSamplerIndex(uint packedTextureSlotSamplerIndex)
{
    // The texture slot occupies the lower 24-bits and the sampler index is in the upper 8-bits.
    // This must be kept in sync with packing/unpacking functions in litl-renderer/utility.hpp
    return (packedTextureSlotSamplerIndex >> 24u);
}

/**
 * Samples from the global 2D texture table using the packed sampler.
 */
public float4 sampleGlobalTexture(uint packedTextureSlotSamplerIndex, float2 uv)
{
    const uint textureIndex = unpackTextureIndex(packedTextureSlotSamplerIndex);
    const uint samplerIndex = unpackSamplerIndex(packedTextureSlotSamplerIndex);
    return g_Textures[NonUniformResourceIndex(textureIndex)].Sample(g_Samplers[NonUniformResourceIndex(samplerIndex)], uv);
}

/**
 * Samples from the global 2D texture table using the specified sampler.
 */
public float4 sampleGlobalTexture(uint packedTextureSlotSamplerIndex, in SamplerState samplerOverride, float2 uv)
{
    const uint textureIndex = unpackTextureIndex(packedTextureSlotSamplerIndex);
    return g_Textures[NonUniformResourceIndex(textureIndex)].Sample(samplerOverride, uv);
}

/**
 * Samples from the global 2D texture array table using the packed sampler.
 */
public float4 sampleGlobalTextureArray(uint packedTextureSlotSamplerIndex, uint layer, float2 uv)
{
    const uint textureIndex = unpackTextureIndex(packedTextureSlotSamplerIndex);
    const uint samplerIndex = unpackSamplerIndex(packedTextureSlotSamplerIndex);
    const float3 uvw = float3(uv, float(layer));

    return g_TextureArrays[NonUniformResourceIndex(textureIndex)].Sample(g_Samplers[NonUniformResourceIndex(samplerIndex)], uvw);
}

/**
 * Samples from the global 2D texture array table using the specified sampler.
 */
public float4 sampleGlobalTextureArray(uint packedTextureSlotSamplerIndex, in SamplerState samplerOverride, uint layer, float2 uv)
{
    const uint textureIndex = unpackTextureIndex(packedTextureSlotSamplerIndex);
    const float3 uvw = float3(uv, float(layer));

    return g_TextureArrays[NonUniformResourceIndex(textureIndex)].Sample(samplerOverride, uvw);
}

public float sampleGlobalShadowTextureArray(uint index, float3 uvw, float ref)
{
    return g_ShadowTextureArrays[index].SampleCmpLevelZero(g_ComparisonSamplers[DefaultSamplerIndex_ShadowCompare], uvw, ref);
}
