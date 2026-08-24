#ifndef LITL_ENGINE_OBJECT_HANDLES_H__
#define LITL_ENGINE_OBJECT_HANDLES_H__

#include "litl-core/handles.hpp"

namespace litl
{
    struct CameraHandleTag {};
    using CameraHandle = Handle<CameraHandleTag>;

    struct GpuBufferHandleTag {};
    using GpuBufferHandle = Handle<GpuBufferHandleTag>;

    struct MaterialHandleTag {};
    using MaterialHandle = Handle<MaterialHandleTag>;

    struct MaterialPipelineHandleTag {};
    using MaterialPipelineHandle = Handle<MaterialPipelineHandleTag>;

    struct MeshHandleTag {};
    using MeshHandle = Handle<MeshHandleTag>;

    struct TextHandleTag {};
    using TextHandle = Handle<TextHandleTag>;

    struct Texture2DHandleTag {};
    using Texture2DHandle = Handle<Texture2DHandleTag>;
}

#endif