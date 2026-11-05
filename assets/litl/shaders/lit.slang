// -----------------------------------------------------------------------------------------
// lit.slang
// Applies tint, textures, and lighting. Note: there currently is no lighting in LITL so it
// simply modifes the final color value by the model normal.y to give the illusion.
// -----------------------------------------------------------------------------------------

import litl.core;

struct MaterialProperties
{
    float4 tint;
    uint baseColor;
    float alphaCutoff;
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
    const float4 texColor = sampleGlobalTexture(matProps.baseColor, input.uv);
    const float4 baseColor = texColor * matProps.tint;

    // todo need specialization constants so this is not present among all opaques
    if (baseColor.a < matProps.alphaCutoff)
    {
        discard;
    }

    // TODO lighting
    const float3 ambient = _pc.lightData->ambient.rgb;
    const float3 shading = max(ambient, ((normalize(input.normal).y + 1.0f) * 0.5f).rrr);
    const float3 diffuse = float3(1.0f, 1.0f, 1.0f);
    const float3 specular = float3(0.0f, 0.0f, 0.0f);
    const float3 emissive = float3(0.0f, 0.0f, 0.0f);
    const float3 color = (baseColor.rgb * diffuse * shading * baseColor.a) + specular + emissive;           // * baseColor.a for premultiplied transparency

    output.color = float4(color.rgb, baseColor.a);

    return output;
}