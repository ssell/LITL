module core;

#define BindSetPerFrame    0
#define BindSetPerPass     1
#define BindSetPerMaterial 2
#define BindSetPerObject   3

#define Bind(_b, _s)        [[vk::binding(binding, _s)]]
#define BindPerFrame(_b)    [[vk::binding(_b, BindSetPerFrame)]]
#define BindPerPass(_b)     [[vk::binding(_b, BindSetPerPass)]]
#define BindPerMaterial(_b) [[vk::binding(_b, BindSetPerMaterial)]]
#define BindPerObject(_b)   [[vk::binding(_b, BindSetPerObject)]]

// -----------------------------------------------------------------------------------------
// Global Texture Utilities
// -----------------------------------------------------------------------------------------

BindPerFrame(0) Texture2D<float4> g_Textures[];
BindPerFrame(1) SamplerState g_Samplers[16];

// These correlate to TextureTableReservedIndices in litl-renderer
#define DefaultTextureIndex_Pink   0
#define DefaultTextureIndex_White  1
#define DefaultTextureIndex_Black  2
#define DefaultTextureIndex_Normal 3

// These correlate to SamplerPredefines in litl-renderer
#define DefaultSamplerIndex_LinearRepeat           0
#define DefaultSamplerIndex_LinearClamp            1
#define DefaultSamplerIndex_LinearRepeatAniso      2
#define DefaultSamplerIndex_LinearClampAniso       3
#define DefaultSamplerIndex_NearestRepeat          4
#define DefaultSamplerIndex_NearestClamp           5
#define DefaultSamplerIndex_NearestClampNoMip      6
#define DefaultSamplerIndex_LinearClampNoMip       7
#define DefaultSamplerIndex_ShadowCompare          8
#define DefaultSamplerIndex_LinearClampBorderWhite 9
#define DefaultSamplerIndex_LinearClampBorderBlack 10
#define DefaultSamplerIndex_LinearMirrorRepeat     11
#define DefaultSamplerIndex_Reserved12             12
#define DefaultSamplerIndex_Reserved13             13
#define DefaultSamplerIndex_Reserved14             14
#define DefaultSamplerIndex_Reserved15             15

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

// -----------------------------------------------------------------------------------------
// Properties Input
// -----------------------------------------------------------------------------------------

// Maps to RenderPerFrameData in litl-engine/render/renderStructs.hpp
public struct PerFrameData
{
    public uint frame;
    public uint frameIndex;
    public float elapsedTime;
    public float deltaTime;
};

// Maps to RenderPerPassData in litl-engine/render/renderStructs.hpp
public struct PerPassData
{
    public float4x4 view;
    public float4x4 projection;
    public float4x4 viewProjection;
};

// Maps to RenderDataMap in litl-engine/render/renderStructs.hpp
public struct RenderInstanceData
{
    public uint gpuIndex;
    public uint materialSlot;
};

// Maps to RenderPushConstants in litl-engine/render/renderStructs.hpp
public struct PushConstants<MatProps>
{
    public PerFrameData *frameData;
    public PerPassData *passData;
    public RenderInstanceData *instanceData;
    public float4x4 *worldMatrices;
    public MatProps *materialProperties;

    public uint getGpuIndex(in uint globalInstanceId)
    {
        return instanceData[globalInstanceId].gpuIndex;
    }

    public float4x4 getWorldMatrix(in uint globalInstanceId)
    {
        return worldMatrices[getGpuIndex(globalInstanceId)];
    }

    public float4x4 getViewMatrix()
    {
        return passData->view;
    }

    public float4x4 getProjectionMatrix()
    {
        return passData->projection;
    }

    public float4x4 getViewProjectionMatrix()
    {
        return passData->viewProjection;
    }

    public uint getMaterialSlot(in uint globalInstanceId)
    {
        return instanceData[getGpuIndex(globalInstanceId)].materialSlot;
    }

    public MatProps getMaterial(in uint slot)
    {
        return materialProperties[slot];
    }
};

// -----------------------------------------------------------------------------------------
// Vertex Input
// -----------------------------------------------------------------------------------------

// Maps to Vertex in litl-core/math/geometry/vertex.hpp
public struct VertexInput
{
    public float3 position;
    public float2 uv;
    public float3 normal;
    public float4 tangent;
};
