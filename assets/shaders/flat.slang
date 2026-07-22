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
struct DataMap
{
    PerFrameData* frameData;
    PerPassData* passData;
}

// Maps to RenderPushConstants in litl-engine/render/renderStructs.hpp
struct PushConstants
{
    DataMap* dataMap;
};

[[vk::push_constant]] PushConstants _pushConstants;

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
    output.position = mul(_pushConstants->dataMap->passData->viewProjection, float4(input.position.xy, 0.0, 1.0));
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

    // float transition = (sin(_pushConstants->dataMap->frameData->elapsedTime) + 1.0f) * 0.5f;
    output.color = float4(input.color.rgb, 1.0f);

    return output;
}
