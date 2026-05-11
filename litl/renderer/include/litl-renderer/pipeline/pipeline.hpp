#ifndef LITL_RENDERER_FIXED_PIPELINE_H__
#define LITL_RENDERER_FIXED_PIPELINE_H__

#include <cstdint>

namespace litl
{
    enum class CullMode : uint8_t
    {
        None,
        Front,
        Back,
        FrontBack
    };

    enum class FillMode : uint8_t
    {
        Solid, Wireframe
    };

    enum class FrontFace : uint8_t
    {
        Clockwise,
        CounterClockwise
    };

    enum class CompareOp : uint8_t
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class BlendFactor : uint8_t
    {
        Zero,
        One,
        SrcAlpha,
        OneMinusSrcAlpha
    };

    enum class BlendOp : uint8_t
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum class PrimitiveTopology : uint8_t
    {
        TriangleList,
        TriangleStrip,
        LineList,
        PointList
    };

    enum class Format : uint8_t
    {
        // ... todo ...
    };

    struct RasterState
    {
        CullMode cull = CullMode::Back;
        FillMode fill = FillMode::Solid;
        FrontFace frontFace = FrontFace::CounterClockwise;

        bool depthClamp = false;
        float depthBiasConstant = 0.0f;
        float depthBiasSlope = 0.0f;
    };

    struct DepthState
    {
        bool depthTest = true;
        bool depthWrite = true;
        CompareOp depthCompare = CompareOp::Less;
    };

    struct StencilState
    {
        // ... todo ...
    };

    struct BlendState
    {
        bool blendEnabled = false;
        BlendFactor srcColor = BlendFactor::One;
        BlendFactor dstColor = BlendFactor::Zero;
        BlendOp colorOp = BlendOp::Add;
        BlendFactor srcAlpha = BlendFactor::One;
        BlendFactor dstAlpha = BlendFactor::Zero;
        BlendOp alphaOp = BlendOp::Add;
        uint8_t writeMask = 0xF;
    };

    struct VertexAttribute 
    {
        uint32_t location;
        uint32_t binding;
        Format format;
        uint32_t offset;
    };

    struct VertexBinding 
    {
        uint32_t binding;
        uint32_t stride;
        bool per_instance;
    };
}

#endif