struct PerFrameData
{
    float elapsedTime;
    float deltaTime;
    uint frameCount;
};

struct PerCameraData
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
}

struct PushConstants
{
    PerFrameData* frame;
};

struct VertexInput
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct VertexOutput 
{
    float4 position: SV_Position;
    float3 color : COLOR;
};

struct FragmentOutput
{
    float4 color : SV_Target;
};

[vk::push_constant] PushConstants _pc;
[vk::binding(0, 0)] ConstantBuffer<PerCameraData> _cameraData;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output = (VertexOutput)0;
    output.position = mul(_cameraData.viewProjection, float4(input.position.xy, 0.0, 1.0));
    output.color = input.color;

    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output = (FragmentOutput)0;
    output.color = float4(input.color * sin(_pc.frame->elapsedTime), 1.0);

    return output;
}
