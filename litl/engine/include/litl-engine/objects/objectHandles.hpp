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

    struct ShaderHandleTag {};
    using ShaderHandle = Handle<ShaderHandleTag>;

    struct TextHandleTag {};
    using TextHandle = Handle<TextHandleTag>;

    struct Texture2DHandleTag {};
    using Texture2DHandle = Handle<Texture2DHandleTag>;
}

// Make the handles compatible with standard library hash maps (std::map, std:unordered_map, etc.)
LITL_STD_HASH_HANDLE(litl::CameraHandleTag);
LITL_STD_HASH_HANDLE(litl::GpuBufferHandleTag);
LITL_STD_HASH_HANDLE(litl::MaterialHandle);
LITL_STD_HASH_HANDLE(litl::MeshHandleTag);
LITL_STD_HASH_HANDLE(litl::ShaderHandleTag);
LITL_STD_HASH_HANDLE(litl::TextHandleTag);
LITL_STD_HASH_HANDLE(litl::Texture2DHandleTag);

#endif