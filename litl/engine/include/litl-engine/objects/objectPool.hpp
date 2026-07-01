#ifndef LITL_ENGINE_OBJECT_POOLS_H__
#define LITL_ENGINE_OBJECT_POOLS_H__

#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/mesh/mesh.hpp"
#include "litl-engine/objects/buffer/gpuBuffer.hpp"

namespace litl
{
    class ObjectPool
    {
    public:

        ObjectPool();
        ~ObjectPool();

        ObjectPool(ObjectPool const&) = delete;
        ObjectPool& operator=(ObjectPool const&) = delete;

        [[nodiscard]] GpuBufferHandle createGpuBuffer(GpuBufferDescriptor const& descriptor) noexcept;
        [[nodiscard]] GpuBuffer* getGpuBuffer(GpuBufferHandle handle) noexcept;
        void destroyGpuBuffer(GpuBufferHandle handle) noexcept;

        [[nodiscard]] MeshHandle createMesh(MeshDescriptor const& descriptor) noexcept;
        [[nodiscard]] Mesh* getMesh(MeshHandle handle) noexcept;
        void destroyMesh(MeshHandle handle) noexcept;

    private:

        HandlePool<GpuBuffer, GpuBufferHandleTag> m_gpuBufferPool;
        HandlePool<Mesh, MeshHandleTag> m_meshPool;
    };
}

#endif