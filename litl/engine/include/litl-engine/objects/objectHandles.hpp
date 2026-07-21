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

    struct MeshHandleTag {};
    using MeshHandle = Handle<MeshHandleTag>;
}

#endif