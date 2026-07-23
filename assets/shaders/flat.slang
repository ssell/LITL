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

// Maps to RenderPushConstants in litl-engine/render/renderStructs.hpp
struct PushConstants
{
    PerFrameData* frameData;
    PerPassData *passData;
    RenderInstanceData* instanceData;
    float4x4* worldMatrices;
};

[[vk::push_constant]] PushConstants _pc;

// -----------------------------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------------------------

// Maps to Vertex in litl-engine/types/vertex.hpp
struct VertexInput
{
    float3 position : POSITION;
    float3 color    : COLOR;
    float2 uv       : UV;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color    : COLOR;
    float2 uv       : UV;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output = (VertexOutput)0;

     float4x4 mvpMatrix = mul(_pc->passData->viewProjection, _pc->worldMatrices[0]);

    output.position = mul(mvpMatrix, float4(input.position.xy, 0.0, 1.0));
    output.color = input.color;
    output.uv = input.uv;
    
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

    float transition = 1.0f;//(sin(_pc->frameData->elapsedTime) + 1.0f) * 0.5f;
    output.color = float4(input.color.rgb * transition, 1.0f);
    //output.color.r = 1.0f;

    return output;
}
