#ifndef LITL_ENGINE_OBJECT_HANDLES_H__
#define LITL_ENGINE_OBJECT_HANDLES_H__

#include "litl-core/handles.hpp"

namespace litl
{
    struct GpuBufferHandleTag {};
    using GpuBufferHandle = Handle<GpuBufferHandleTag>;

    struct MeshHandleTag {};
    using MeshHandle = Handle<MeshHandleTag>;
}

#endif