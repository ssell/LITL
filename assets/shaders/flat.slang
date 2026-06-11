struct VertexInput
{
    float3 position : POSITION;
    float3 color : COLOR;
}

struct VertexOutput 
{
    float4 position: SV_Position;
    float3 color : COLOR;
};

struct FragmentOutput
{
    float4 color : SV_Target;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output = (VertexOutput)0;
    output.position = float4(input.position.xy, 0.0, 1.0);
    output.color = input.color;

    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output = (FragmentOutput)0;
    output.color = float4(1.0, 1.0, 1.0, 1.0);

    return output;
}
