// -----------------------------------------------------------------------------------------
// unlit.slang
// Basic shader that applies tint and textures without any lighting or other effects.
// -----------------------------------------------------------------------------------------

import litl.core;

struct MaterialProperties
{
    float3 tint;
    uint baseColor;
};

[[vk::push_constant]] ConstantBuffer<PushConstants<MaterialProperties>> _pc;

// -----------------------------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------------------------

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : UV;
    float3 color    : COLOR;
    nointerpolation uint materialSlot;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint globalInstanceId: SV_VulkanInstanceID, uint localInstanceId: SV_InstanceID)
{
    VertexOutput output = (VertexOutput)0;

    const float4x4 worldMatrix = _pc.getWorldMatrix(globalInstanceId);
    const float4x4 mvpMatrix = mul(_pc.getViewProjectionMatrix(), worldMatrix);

    output.position     = mul(mvpMatrix, float4(input.position.xyz, 1.0f));
    output.normal       = normalize(mul(float3x3(worldMatrix), input.normal));
    output.uv           = input.uv;
    output.color        = float3(1.0f, 1.0f, 1.0f);
    output.materialSlot = _pc.getMaterialSlot(globalInstanceId);

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

    const MaterialProperties matProps = _pc.getMaterial(input.materialSlot);
    const float3 tint     = matProps.tint;
    const float3 texColor = sampleGlobalTexture(matProps.baseColor, input.uv).rgb;
    
    output.color = float4(texColor * tint, 1.0f);

    return output;
}