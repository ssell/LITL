#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/ecs/systems/meshInstantiationSystem.hpp"

namespace litl
{
    namespace
    {
        void transitionToFailedMeshInstance(EntityCommands& commands, Entity entity, MeshAssetHandle meshHandle) noexcept
        {
            commands.removeComponent<PendingMeshInstance>(entity);
            commands.addComponent<FailedMeshInstance>(entity, FailedMeshInstance{ .meshHandle = meshHandle });
        }

        void processMeshInstantiation(EntityCommands& commands, AssetManager& assetManager, Entity entity, MeshAssetHandle meshAssetHandle, LocalBounds* localBounds) noexcept
        {
            // -----------------------------------------------------------------------------
            // Swap out PendingMeshInstance for either MeshRef or FailedMeshInstance
            // -----------------------------------------------------------------------------

            commands.removeComponent<PendingMeshInstance>(entity);

            const MeshAsset* meshAsset = assetManager.getMesh(meshAssetHandle);

            if ((meshAsset == nullptr) || (meshAsset->mesh == nullptr))
            {
                commands.addComponent<FailedMeshInstance>(entity, FailedMeshInstance{ .meshHandle = meshAssetHandle });
                return;
            }

            // -----------------------------------------------------------------------------
            // Add the MeshRef and update LocalBounds
            // -----------------------------------------------------------------------------

            commands.addComponent<MeshRef>(entity, MeshRef{ .handle = meshAsset->handle });

            if (localBounds != nullptr)
            {
                localBounds->bounds = meshAsset->bounds;
            }
        }
    }

    void MeshInstantiationSystem::setup(ServiceProvider& services)
    {
        m_pAssetManager = services.get<AssetManager>();
    }

    void MeshInstantiationSystem::prepare()
    {

    }

    void MeshInstantiationSystem::update(SystemData const& data, Entity entity, PendingMeshInstance const& pendingMesh, LocalBounds* localBounds)
    {
        if (!pendingMesh.meshHandle.isValid())
        {
            transitionToFailedMeshInstance(data.commands, entity, pendingMesh.meshHandle);
            return;
        }

        const auto meshAssetStatus = m_pAssetManager->getMeshAssetStatus(pendingMesh.meshHandle);

        switch (meshAssetStatus)
        {
        case AssetStatus::Unloaded:
            // Trigger the mesh load if it has not yet happened.
            std::ignore = m_pAssetManager->getMesh(pendingMesh.meshHandle);
            break;

        case AssetStatus::Loading:
            // Keep waiting to transition to either Error, InMemory, or Invalid.
            break;

        case AssetStatus::InMemory:
            processMeshInstantiation(data.commands, *m_pAssetManager.get(), entity, pendingMesh.meshHandle, localBounds);
            break;

        case AssetStatus::Error:
        case AssetStatus::Invalid:
        default:
            transitionToFailedMeshInstance(data.commands, entity, pendingMesh.meshHandle);
            break;
        }
    }
}