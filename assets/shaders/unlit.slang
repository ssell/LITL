// -----------------------------------------------------------------------------------------
// Input
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
}

// Maps to RenderDataMap in litl-engine/render/renderStructs.hpp
struct RenderInstanceData
{
    uint gpuIndex;
    uint materialSlot;
}

struct MaterialProperties
{
    float3 tint;
}

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
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint globalInstanceId: SV_VulkanInstanceID, uint localInstanceId: SV_InstanceID)
{
    VertexOutput output = (VertexOutput)0;

    const uint gpuIndex = _pc->instanceData[globalInstanceId].gpuIndex;
    const uint matIndex = _pc->instanceData[globalInstanceId].materialSlot;

    const float4x4 worldMatrix = _pc->worldMatrices[gpuIndex];
    const float4x4 mvpMatrix = mul(_pc->passData->viewProjection, worldMatrix);

    output.position = mul(mvpMatrix, float4(input.position.xyz, 1.0f));
    output.normal = normalize(mul(float3x3(worldMatrix), input.normal));
    output.uv = input.uv;
    output.color = float3(1.0f, 1.0f, 1.0f);

    return output;
}

// -----------------------------------------------------------------------------------------
// Fragment Shader
// -----------------------------------------------------------------------------------------

// !TEMPORARY BINDING! will move to bindless shortly ...
//#define BindSetPerObject 3
//#define Bind(_b, _s) [[vk::binding(binding, _s)]]
//#define BindPerObject(_b) [[vk::binding(_b, BindSetPerObject)]]
//BindPerObject(0) Texture2D<float4> _texture;
//BindPerObject(1) SamplerState _texture_sampler;

struct FragmentOutput
{
    float4 color : SV_Target;
};

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output = (FragmentOutput)0;

    const float3 tint = input.color;
    const float3 texColor = float3(1.0f, 1.0f, 1.0f);// _texture.Sample(_texture_sampler, input.uv).rgb;
    const float3 color = (tint * texColor * input.normal);

    output.color = float4(color, 1.0f);

    return output;
}