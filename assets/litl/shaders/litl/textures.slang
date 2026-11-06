// -----------------------------------------------------------------------------------------
// [textures.slang]
// Common logic for texture-related functions and the global texture table.
// -----------------------------------------------------------------------------------------

module textures;
import common;

// -----------------------------------------------------------------------------------------
// Global Texture Utilities

[[vk::binding(0, BindSetPerFrame)]] Texture2D<float4> g_Textures[];
[[vk::binding(1, BindSetPerFrame)]] SamplerState g_Samplers[16];

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

void unpackPackedTextureSlotSamplerIndex(in uint packedTextureSlotSamplerIndex, out uint textureIndex, out uint samplerIndex)
{
    // The texture slot occupies the lower 24-bits and the sampler index is in the upper 8-bits.
    // This must be kept in sync with packing/unpacking functions in litl-renderer/utility.hpp
    textureIndex = packedTextureSlotSamplerIndex & 0x00FFFFFF;
    samplerIndex = (packedTextureSlotSamplerIndex >> 24u);
}

public float4 sampleGlobalTexture(uint packedTextureSlotSamplerIndex, float2 uv)
{
    uint textureIndex = 0;
    uint samplerIndex = 0;

    unpackPackedTextureSlotSamplerIndex(packedTextureSlotSamplerIndex, /* out */ textureIndex, /* out */ samplerIndex);

    return g_Textures[NonUniformResourceIndex(textureIndex)].Sample(g_Samplers[NonUniformResourceIndex(samplerIndex)], uv);
}

public float4 sampleGlobalTexture(uint packedTextureSlotSamplerIndex, in SamplerState samplerOverride, float2 uv)
{
    uint textureIndex = 0;
    uint samplerIndex = 0;

    unpackPackedTextureSlotSamplerIndex(packedTextureSlotSamplerIndex, /* out */ textureIndex, /* out */ samplerIndex);

    return g_Textures[NonUniformResourceIndex(textureIndex)].Sample(samplerOverride, uv);
}
