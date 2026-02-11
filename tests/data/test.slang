static float2 positions[3] = float2[](
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
);

static float3 colors[3] = float3[](
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
);

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
VertexOutput vertexMain(uint vertexId: SV_VertexID)
{
    VertexOutput output = (VertexOutput)0;
    output.position = float4(positions[vertexId], 0.0, 1.0);
    output.color = colors[vertexId];

    return output;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input)
{
    FragmentOutput output = (FragmentOutput)0;
    output.color = float4(input.color, 1.0);

    return output;
}
