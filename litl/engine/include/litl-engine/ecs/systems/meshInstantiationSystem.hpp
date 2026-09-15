#ifndef LITL_ENGINE_ECS_MESH_INSTANTIATION_SYSTEM_H__
#define LITL_ENGINE_ECS_MESH_INSTANTIATION_SYSTEM_H__

#include <memory>
#include "litl-engine/ecs/common.hpp"

namespace litl
{
    class AssetManager;
    class ServiceProvider;

    /// <summary>
    /// Polls each frame to check the status of the mesh asset referenced by a PendingMeshInstance component.
    /// If the mesh is in memory, then this system replaces the PendingMeshInstance component with a MeshRef component.
    /// Additionally, if the entity also has a LocalBounds component then it is updated with the mesh bounds.
    /// 
    /// If the asset failed to load then the PendingMeshInstance is replaced with a FailedMeshInstance component.
    /// </summary>
    class MeshInstantiationSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, PendingMeshInstance const& pendingMesh, LocalBounds* localBounds);

    private:

        std::shared_ptr<AssetManager> m_pAssetManager;
    };
}

#endif