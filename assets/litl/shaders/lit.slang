// -----------------------------------------------------------------------------------------
// lit.slang
// Applies tint, textures, and lighting. Note: there currently is no lighting in LITL so it
// simply modifes the final color value by the model normal.y to give the illusion.
// -----------------------------------------------------------------------------------------

import litl.library;

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
    float3 positionWS;
    nointerpolation uint materialSlot;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint globalInstanceId: SV_VulkanInstanceID, uint localInstanceId: SV_InstanceID)
{
    VertexOutput output = (VertexOutput)0;

    const float4x4 worldMatrix = _pc.getWorldMatrix(globalInstanceId);
    const float4x4 mvpMatrix = mul(_pc.getViewProjectionMatrix(), worldMatrix);

    output.position     = mul(mvpMatrix, float4(input.position.xyz, 1.0f));
    output.positionWS   = input.position.xyz;
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

    const float3 toViewDir = normalize(_pc.passData.cameraPosition.xyz - input.positionWS);
    const MaterialProperties matProps = _pc.getMaterial(input.materialSlot);
    const float4 texColor = sampleGlobalTexture(matProps.baseColor, input.uv);
    const float4 baseColor = texColor * matProps.tint;

    // todo need specialization constants so this is not present among all opaques
    if (baseColor.a < matProps.alphaCutoff)
    {
        discard;
    }

    SurfaceData surface;
    surface.normal    = normalize(input.normal);        // Normalize again since its been interpolated across the surface
    surface.albedo    = baseColor.rgb;
    surface.roughness = 1.0f;                                   // todo
    surface.metallic  = 0.0f;                                   // todo

    const float3 ambient = _pc.lightData->ambient.rgb * surface.albedo;
    const float3 directional = shadeDirectionalLights(_pc.lightData, surface, toViewDir);
    const float3 diffuse = directional + ambient;
    const float3 specular = float3(0.0f, 0.0f, 0.0f);
    const float3 emissive = float3(0.0f, 0.0f, 0.0f);
    const float3 color = diffuse + specular + emissive;

    output.color = float4(color.rgb, baseColor.a);

    // todo if transparency is premultiplied. again, need a specialization constant.
    //output.color.rgb *= baseColor.a;

    return output;
}