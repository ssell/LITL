#ifndef LITL_ENGINE_OBJECT_POOLS_H__
#define LITL_ENGINE_OBJECT_POOLS_H__

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/camera.hpp"
#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-engine/objects/material.hpp"
#include "litl-engine/objects/mesh.hpp"
#include "litl-engine/objects/text.hpp"
#include "litl-engine/objects/texture2d.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;
    class AssetManager;

    class ObjectPool
    {
    public:

        ObjectPool();
        ~ObjectPool();

        ObjectPool(ObjectPool const&) = delete;
        ObjectPool& operator=(ObjectPool const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth) noexcept;

        [[nodiscard]] CameraHandle createCamera(CameraDescriptor const& descriptor) noexcept;
        [[nodiscard]] Camera* getCamera(CameraHandle handle) noexcept;
        void getAllCameraHandles(std::vector<CameraHandle>& handles) const noexcept;
        void destroyCamera(CameraHandle handle) noexcept;

        [[nodiscard]] GpuBufferHandle createGpuBuffer(GpuBufferDescriptor const& descriptor) noexcept;
        [[nodiscard]] GpuBuffer* getGpuBuffer(GpuBufferHandle handle) noexcept;
        void getAllGpuBufferHandles(std::vector<GpuBufferHandle>& handles) const noexcept;
        void destroyGpuBuffer(GpuBufferHandle handle) noexcept;
        void deferDestroyGpuBuffer(GpuBufferHandle handle) noexcept;

        [[nodiscard]] MaterialHandle reserveMaterial(Authority<AssetManager> auth) noexcept;
        [[nodiscard]] MaterialHandle createMaterial(MaterialDescriptor const& descriptor) noexcept;
        [[nodiscard]] Material* getMaterial(MaterialHandle handle) noexcept;
        void getAllMaterialHandles(std::vector<MaterialHandle>& handles) const noexcept;
        void destroyMaterial(MaterialHandle handle) noexcept;
        void deferDestroyMaterial(MaterialHandle handle) noexcept;

        [[nodiscard]] MeshHandle reserveMesh(Authority<AssetManager> auth, ObjectDescriptor const& descriptor) noexcept;
        [[nodiscard]] MeshHandle createMesh(MeshDescriptor const& descriptor) noexcept;
        [[nodiscard]] Mesh* getMesh(MeshHandle handle) noexcept;
        void getAllMeshHandles(std::vector<MeshHandle>& handles) const noexcept;
        void destroyMesh(MeshHandle handle) noexcept;
        void deferDestroyMesh(MeshHandle handle) noexcept;

        [[nodiscard]] TextHandle reserveText(Authority<AssetManager> auth) noexcept;
        [[nodiscard]] TextHandle createText(TextDescriptor const& descriptor) noexcept;
        [[nodiscard]] Text* getText(TextHandle handle) noexcept;
        void getAllTextHandles(std::vector<TextHandle>& handles) const noexcept;
        void destroyText(TextHandle handle) noexcept;
        void deferDestroyText(TextHandle handle) noexcept;

        [[nodiscard]] Texture2DHandle reserveTexture2D(Authority<AssetManager> auth) noexcept;
        [[nodiscard]] Texture2DHandle createTexture2D(Texture2DDescriptor const& descriptor) noexcept;
        [[nodiscard]] Texture2D* getTexture2D(Texture2DHandle handle) noexcept;
        void getAllTexture2DHandles(std::vector<Texture2DHandle>& handles) const noexcept;
        void destroyTexture2D(Texture2DHandle handle) noexcept;
        void deferDestroyTexture2D(Texture2DHandle handle) noexcept;

    private:

        struct Impl;
        ImplPtr<Impl, 512> m_impl;
    };
}

#endif