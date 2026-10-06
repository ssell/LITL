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
// (these will eventually be moved into their own file)
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

void UnpackPackedTextureSlotSamplerIndex(in uint packedTextureSlotSamplerIndex, out uint textureIndex, out uint samplerIndex)
{
    // The texture slot occupies the lower 24-bits and the sampler index is in the upper 8-bits.
    // This must be kept in sync with packing/unpacking functions in litl-renderer/utility.hpp
    textureIndex = packedTextureSlotSamplerIndex & 0x00FFFFFF;
    samplerIndex = (packedTextureSlotSamplerIndex >> 24u);
}

float4 SampleGlobalTexture(uint packedTextureSlotSamplerIndex, float2 uv)
{
    uint textureIndex = 0;
    uint samplerIndex = 0;

    UnpackPackedTextureSlotSamplerIndex(packedTextureSlotSamplerIndex, /* out */ textureIndex, /* out */ samplerIndex);

    return g_Textures[NonUniformResourceIndex(textureIndex)].Sample(g_Samplers[NonUniformResourceIndex(samplerIndex)], uv);
}

float4 SampleGlobalTexture(uint packedTextureSlotSamplerIndex, in SamplerState samplerOverride, float2 uv)
{
    uint textureIndex = 0;
    uint samplerIndex = 0;

    UnpackPackedTextureSlotSamplerIndex(packedTextureSlotSamplerIndex, /* out */ textureIndex, /* out */ samplerIndex);

    return g_Textures[NonUniformResourceIndex(textureIndex)].Sample(samplerOverride, uv);
}

// -----------------------------------------------------------------------------------------
// Input
// (these will eventually be moved into their own file)
// -----------------------------------------------------------------------------------------

// Maps to RenderPerFrameData in litl-engine/render/renderStructs.hpp
struct PerFrameData
{
    uint frame;
    uint frameIndex;
    float elapsedTime;
    float deltaTime;
};

// Maps to RenderPerPassData in litl-engine/render/renderStructs.hpp
struct PerPassData
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
};

// Maps to RenderDataMap in litl-engine/render/renderStructs.hpp
struct RenderInstanceData
{
    uint gpuIndex;
    uint materialSlot;
};

struct MaterialProperties
{
    float3 tint;
    uint albedo;
};

// Maps to RenderPushConstants in litl-engine/render/renderStructs.hpp
struct PushConstants
{
    PerFrameData* frameData;
    PerPassData *passData;
    RenderInstanceData* instanceData;
    float4x4* worldMatrices;
    MaterialProperties* materialProperties;
};

[[vk::push_constant]] PushConstants _pc;

MaterialProperties FetchMaterialProperties(in uint globalInstanceId)
{
    return _pc->materialProperties[_pc->instanceData[globalInstanceId].materialSlot];
}

// -----------------------------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------------------------

// Maps to Vertex in litl-core/math/geometry/vertex.hpp
struct VertexInput
{
    float3 position;
    float2 uv;
    float3 normal;
    float4 tangent;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : UV;
    float3 color    : COLOR;
    nointerpolation uint materialIndex;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint globalInstanceId: SV_VulkanInstanceID, uint localInstanceId: SV_InstanceID)
{
    VertexOutput output = (VertexOutput)0;

    const uint gpuIndex = _pc->instanceData[globalInstanceId].gpuIndex;

    const float4x4 worldMatrix = _pc->worldMatrices[gpuIndex];
    const float4x4 mvpMatrix = mul(_pc->passData->viewProjection, worldMatrix);

    output.position = mul(mvpMatrix, float4(input.position.xyz, 1.0f));
    output.normal = normalize(mul(float3x3(worldMatrix), input.normal));
    output.uv = input.uv;
    output.color = float3(1.0f, 1.0f, 1.0f);
    output.materialIndex = _pc->instanceData[globalInstanceId].materialSlot;

    return output;
}

// -----------------------------------------------------------------------------------------
// Fragment Shader
// -----------------------------------------------------------------------------------------

struct FragmentOutput
{
    float4 color : SV_Target;
};

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output = (FragmentOutput)0;
    MaterialProperties matProps = FetchMaterialProperties(input.materialIndex);

    const float3 tint = matProps.tint;
    const float3 texColor = SampleGlobalTexture(matProps.albedo, input.uv).rgb;
    
    output.color = float4(texColor * tint, 1.0f);

    return output;
}