 // todo: these will be moved into some sort of common file
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
    float fade;
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
    float2 uv       : UV;
    float3 color    : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(
    VertexInput input, 
    uint globalInstanceId : SV_VulkanInstanceID, 
    uint localInstanceId : SV_InstanceID)
{
    VertexOutput output = (VertexOutput)0;

    uint gpuIndex = _pc->instanceData[globalInstanceId].gpuIndex;
    uint matIndex = _pc->instanceData[globalInstanceId].materialSlot;
    
    float4x4 worldMatrix = _pc->worldMatrices[gpuIndex];
    float4x4 mvpMatrix = mul(_pc->passData->viewProjection, worldMatrix);
    MaterialProperties matProps = _pc->materialProperties[matIndex];

    output.position = mul(mvpMatrix, float4(input.position.xyz, 1.0));
    output.uv = input.uv;
    output.color = matProps.fade * (input.normal + 1.0) * 0.5;      // [-1, 1] to [0, 1]

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
    output.color = float4(input.color.yyy, 1.0f);
    return output;
}
