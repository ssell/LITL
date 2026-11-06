// -----------------------------------------------------------------------------------------
// [library.slang]
// Common shader inputs including vertex input layout and push constants.
// -----------------------------------------------------------------------------------------

module input;

import common;
import lighting;

// -----------------------------------------------------------------------------------------
// Properties Input
// -----------------------------------------------------------------------------------------

// Maps to RenderPerFrameData in litl-engine/render/renderStructs.hpp
public struct PerFrameData
{
    public uint frame;
    public uint frameIndex;
    public float elapsedTime;
    public float deltaTime;
};

// Maps to RenderPerPassData in litl-engine/render/renderStructs.hpp
public struct PerPassData
{
    public float4x4 view;
    public float4x4 projection;
    public float4x4 viewProjection;
    public float4 cameraPosition;
};

// Maps to RenderDataMap in litl-engine/render/renderStructs.hpp
public struct RenderInstanceData
{
    public uint gpuIndex;
    public uint materialSlot;
};

// Maps to RenderPushConstants in litl-engine/render/renderStructs.hpp
public struct PushConstants<MatProps>
{
    public PerFrameData *frameData;
    public PerPassData *passData;
    public RenderInstanceData *instanceData;
    public float4x4 *worldMatrices;
    public MatProps *materialProperties;
    public LightData *lightData;

    public uint getGpuIndex(in uint globalInstanceId)
    {
        return instanceData[globalInstanceId].gpuIndex;
    }

    public float4x4 getWorldMatrix(in uint globalInstanceId)
    {
        return worldMatrices[getGpuIndex(globalInstanceId)];
    }

    public float4x4 getViewMatrix()
    {
        return passData->view;
    }

    public float4x4 getProjectionMatrix()
    {
        return passData->projection;
    }

    public float4x4 getViewProjectionMatrix()
    {
        return passData->viewProjection;
    }

    public uint getMaterialSlot(in uint globalInstanceId)
    {
        return instanceData[globalInstanceId].materialSlot;
    }

    public MatProps getMaterial(in uint slot)
    {
        return materialProperties[slot];
    }
};

// -----------------------------------------------------------------------------------------
// Vertex Input
// -----------------------------------------------------------------------------------------

// Maps to Vertex in litl-core/math/geometry/vertex.hpp
public struct VertexInput
{
    public float3 position;
    public float2 uv;
    public float3 normal;
    public float4 tangent;
};