// -----------------------------------------------------------------------------------------
// [blit.slang]
// Used by BlitPass to blit the ongoing frame color render texture onto the swapchain as the final render step.
// It is inteded that this shader is used on its own and not as part of a Material.
// -----------------------------------------------------------------------------------------

import litl.core;

// Must match BlitPushConstants defined in blitPass.cpp
struct BlitPushConstants
{
    uint sourceTextureIndex;
};

[[vk::push_constant]] ConstantBuffer<BlitPushConstants> _pc;

// -----------------------------------------------------------------------------------------
// Vertex Shader (full-screen triangle)
// -----------------------------------------------------------------------------------------

struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : UV;
}

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    VertexOutput output = (VertexOutput)0;

    output.uv = float2((vertexId << 1) & 2, vertexId & 2); // (0,0) (2,0) (0,2)
    output.position = float4((output.uv * 2.0f) - 1.0f, 0.0f, 1.0f);

    return output;
}

// -----------------------------------------------------------------------------------------
// Fragment Shader
// -----------------------------------------------------------------------------------------

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return sampleGlobalTexture(_pc.sourceTextureIndex, input.uv);
}