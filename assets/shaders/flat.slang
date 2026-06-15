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

// -----------------------------------------------------------------------------------------
// Frame Data
// -----------------------------------------------------------------------------------------

struct PerFrameData
{
    float elapsedTime;
    float deltaTime;
    uint frameCount;
};

struct PushConstants
{
    PerFrameData *frame;
};

[vk::push_constant] PushConstants _pc;

// -----------------------------------------------------------------------------------------
// Camera Data
// -----------------------------------------------------------------------------------------

struct PerCameraData
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
}

[vk::binding(0, 0)] ConstantBuffer<PerCameraData> _cameraData;

// -----------------------------------------------------------------------------------------
// Vertex Shader
// -----------------------------------------------------------------------------------------

struct VertexInput
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output = (VertexOutput)0;
    output.position = mul(_cameraData.viewProjection, float4(input.position.xy, 0.0, 1.0));
    output.color = input.color;

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
    output.color = float4(input.color * sin(_pc.frame->elapsedTime), 1.0);

    return output;
}
