#ifndef LITL_ENGINE_OBJECT_POOLS_H__
#define LITL_ENGINE_OBJECT_POOLS_H__

#include "litl-core/impl.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/camera.hpp"
#include "litl-engine/objects/mesh.hpp"
#include "litl-engine/objects/gpuBuffer.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;

    class ObjectPool
    {
    public:

        ObjectPool();
        ~ObjectPool();

        ObjectPool(ObjectPool const&) = delete;
        ObjectPool& operator=(ObjectPool const&) = delete;

        [[nodiscard]] CameraHandle createCamera(CameraDescriptor const& descriptor) noexcept;
        [[nodiscard]] Camera* getCamera(CameraHandle handle) noexcept;
        void getAllCameraHandles(std::vector<CameraHandle>& handles) const noexcept;
        void destroyCamera(CameraHandle handle) noexcept;

        [[nodiscard]] GpuBufferHandle createGpuBuffer(GpuBufferDescriptor const& descriptor) noexcept;
        [[nodiscard]] GpuBuffer* getGpuBuffer(GpuBufferHandle handle) noexcept;
        void getAllGpuBufferHandles(std::vector<GpuBufferHandle>& handles) const noexcept;
        void destroyGpuBuffer(GpuBufferHandle handle) noexcept;

        [[nodiscard]] MeshHandle createMesh(MeshDescriptor const& descriptor) noexcept;
        [[nodiscard]] Mesh* getMesh(MeshHandle handle) noexcept;
        void getAllMeshHandles(std::vector<MeshHandle>& handles) const noexcept;
        void destroyMesh(MeshHandle handle) noexcept;

    private:

        friend class Engine;

        void setup(ServiceProvider& services) noexcept;
        void destroy() noexcept;

        struct Impl;
        ImplPtr<Impl, 512> m_impl;
    };
}

#endif