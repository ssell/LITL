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
    uint normalMap;
    uint occlusionMap;
    uint roughnessMap;
    uint metallicMap;
    float normalFactor;
    float occlusionFactor;
    float roughnessFactor;
    float metallicFactor;
    float alphaCutoff;
};

[[vk::push_constant]] ConstantBuffer<PushConstants<MaterialProperties>> _pc;

// -----------------------------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------------------------

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal;
    float4 tangent;
    float2 uv;
    float3 color;
    float3 positionWS;
    nointerpolation uint materialSlot;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input, uint globalInstanceId: SV_VulkanInstanceID, uint localInstanceId: SV_InstanceID)
{
    VertexOutput output = (VertexOutput)0;

    const float4x4 worldMatrix = _pc.getWorldMatrix(globalInstanceId);
    const float4x4 mvpMatrix = mul(_pc.getViewProjectionMatrix(), worldMatrix);
    const float3x3 m3 = (float3x3)worldMatrix;
    const float detS = sign(determinant(m3));

    output.position     = mul(mvpMatrix, float4(input.position.xyz, 1.0f));
    output.positionWS   = mul(worldMatrix, float4(input.position.xyz, 1.0f)).xyz;
    output.normal       = mul(cofactor(m3), input.normal) * detS;       // Fine only for uniform scaled objects. Non-uniform scaled will need to transform by the inverse-transpose of the world matrix.
    output.tangent.xyz  = mul(m3, input.tangent.xyz);
    output.tangent.w    = input.tangent.w * detS;                       // Mirroring flips the handiness
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

float3 buildSurfaceNormal(in VertexOutput input, in MaterialProperties matProps, bool isFrontFace)
{
    // Rebuild the frame from the interpolated vectors
    float3 N = normalize(input.normal);

    if (dot(input.tangent.xyz, input.tangent.xyz) < EPSILON)
    {
        return (N * (isFrontFace ? 1.0f : -1.0f));
    }

    float3 T = normalize(input.tangent.xyz - N * dot(N, input.tangent.xyz));
    float3 B = cross(N, T) * (input.tangent.w < 0.0f ? -1.0f : 1.0f);

    // Decode the tangent-space normal
    float3 nTS = sampleGlobalTexture(matProps.normalMap, input.uv).xyz * 2.0f - 1.0f;
    nTS.xy *= matProps.normalFactor;
    nTS = normalize(nTS);

    // Tangent space -> world space
    float3 Nw = normalize((T * nTS.x) + (B * nTS.y) + (N * nTS.z));
    Nw *= (isFrontFace ? 1.0f : -1.0f);

    return Nw;
}

SurfaceData buildSurfaceFromMaterial(in MaterialProperties matProps, float2 uv)
{
    SurfaceData surface;

    surface.occlusion = sampleGlobalTexture(matProps.occlusionMap, uv).r * matProps.occlusionFactor;
    surface.roughness = sampleGlobalTexture(matProps.roughnessMap, uv).r * matProps.roughnessFactor;
    surface.metallic = sampleGlobalTexture(matProps.metallicMap, uv).r * matProps.metallicFactor;

    return surface;
}

[shader("fragment")]
FragmentOutput fragmentMain(VertexOutput input, bool isFrontFace: SV_IsFrontFace)
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

    SurfaceData surface = buildSurfaceFromMaterial(matProps, input.uv);
    surface.normal    = buildSurfaceNormal(input, matProps, isFrontFace);
    surface.toView    = normalize(_pc.passData.cameraPosition.xyz - input.positionWS);
    surface.baseColor = baseColor.rgb;
    
    const float3 lighting = shadeLights(_pc.lightData, surface);
    const float3 ambient  = _pc.lightData->ambient.rgb * surface.baseColor * (1.0f - surface.metallic);
    const float3 emissive = float3(0.0f);
    const float3 color    = lighting + ambient + emissive;

    output.color = float4(color.rgb, baseColor.a);

    // todo if transparency is premultiplied. again, need a specialization constant.
    // output.color.rgb *= baseColor.a;

    // output.color.rgb = surface.metallic.rrr;

    return output;
}