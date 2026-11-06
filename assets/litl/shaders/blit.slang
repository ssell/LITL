// -----------------------------------------------------------------------------------------
// [blit.slang]
// Used by BlitPass to blit the ongoing frame color render texture onto the swapchain as the final render step.
// It is inteded that this shader is used on its own and not as part of a Material.
// -----------------------------------------------------------------------------------------

import litl.library;

static const uint BlitModeColor = 0;
static const uint BlitModeDepthFinite = 1;
static const uint BlitModeDepthInfinite = 2;
static const uint BlitModeDepthOrthographic = 3;

// Must match BlitPushConstants defined in blitPass.cpp
struct BlitPushConstants
{
    uint sourceTextureIndex;
    uint mode;
    float zNear;
    float zFar;
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

float linearizeReverseZFinite(float depth, float near, float far)
{
    return (near * far) / (depth * (far - near) + near);
}

float linearizeReverseZInfinite(float depth, float near, float far)
{
    return (near / depth);
}

float linearizeReverseZOrthographic(float depth, float near, float far)
{
    return depth;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    const float4 sampled = sampleGlobalTexture(_pc.sourceTextureIndex, input.uv);
    float4 color = sampled;

    if (_pc.mode != BlitModeColor)
    {
        float viewZ = 0.0f;

        switch (_pc.mode)
        {
        case BlitModeDepthFinite:
            viewZ = linearizeReverseZFinite(sampled.r, _pc.zNear, _pc.zFar);
            break;

        case BlitModeDepthInfinite:
            viewZ = linearizeReverseZInfinite(sampled.r, _pc.zNear, _pc.zFar);
            break;

        case BlitModeDepthOrthographic:
            viewZ = linearizeReverseZOrthographic(sampled.r, _pc.zNear, _pc.zFar);
            break;

        default:
            break;
        }

        const float t = 1.0f - saturate((viewZ - _pc.zNear) / (_pc.zFar - _pc.zNear)); // near = white, far = black
        color = float4(t.rrr, 1.0f);
    }

    return color;
}