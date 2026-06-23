/**
 * Slang Buffer Cheatsheet:
 *
 *     Push Constants:              [vk::push_constant] {Structure} {name};
 *     Uniform Buffer:              ConstantBuffer<{Structure}> {name};
 *     Read-Only Buffer:            StructuredBuffer<{Structure}> {name};
 *     Read-Write Storage Buffer:   RWStructuredBuffer<{Structure}> {name};
 *     Untyped Storage Buffer:      ByteAddressBuffer {name};
 *
 * Typically you need to preceed these with a [vk::binding(x,y)] to indicate the the descriptor set binding.
 * However a ParameterBlock<{Structure}> can be used to store all input and automatically provide the bindings.
 *
 *    struct FrameData
 *    {
 *        ConstantBuffer<ViewUniforms> view;
 *        StructuredBuffer<Light> lights;
 *        StructuredBuffer<Particle> particles;
 *    };
 *
 *    ParameterBlock<FrameData> frame;
 *
 * Is equivalent to:
 *
 *    [vk::binding(0,0)] ConstantBuffer<ViewUniforms> view;
 *    [vk::binding(1,0)] StructuredBuffer<Light> lights;
 *    [vk::binding(2,0)] StructuredBuffer<Particle> particles;
 *
 * Where [vk::binding(binding_index, set_index)].
 */

struct PerFrameData
{
    float elapsedTime;
    float deltaTime;
};

struct PushConstants
{
    PerFrameData *frame;
};

struct PerCameraData
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
}

[vk::push_constant] PushConstants _pc;
[vk::binding(0, 0)] ConstantBuffer<PerCameraData> _cameraData;
[vk::binding(1, 0)] Texture2D<float4> _texture;
[vk::binding(2, 0)] SamplerState _texture_sampler;
// ^ Remember, for vkCmdPushDescriptorSet it can only push to a single set. It can be called multiple times, but must be for the same set.
//   In the future, there will be a set designated as the "push descriptor set" and shader authors need to use that set.
//   The other sets will be handled/populated internally by the engine using descriptor pools or BDA.

// -----------------------------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------------------------

struct VertexInput
{
    float3 position : POSITION;
    float3 color : COLOR;
    float2 uv : UV;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
    float2 uv : UV;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output = (VertexOutput)0;
    output.position = mul(_cameraData.viewProjection, float4(input.position.xy, 0.0, 1.0));
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

    float transition = (sin(_pc.frame->elapsedTime) + 1.0f) * 0.5f;
    float3 vrtColor = input.color.rgb;
    float3 texColor = _texture.Sample(_texture_sampler, input.uv).rgb;

    output.color = float4(lerp(vrtColor, texColor, transition), 1.0f);

    return output;
}
