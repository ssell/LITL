#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/ecs/systems/modelInstantiationSystem.hpp"

namespace litl
{
    namespace
    {
        void transitionToFailedModelInstance(EntityCommands& commands, Entity entity, ModelAssetHandle modelHandle) noexcept
        {
            commands.removeComponent<PendingModelInstance>(entity);
            commands.addComponent<FailedModelInstance>(entity, FailedModelInstance{ .modelHandle = modelHandle });
        }

        void processModelInstantiation(EntityCommands& commands, AssetManager& assetManager, Entity entity, Transform const& transform, PendingModelInstance const& pendingModel) noexcept
        {
            commands.removeComponent<PendingModelInstance>(entity);
            commands.addComponent<ModelInstance>(entity, ModelInstance{ .modelHandle = pendingModel.modelHandle });

            ModelAsset* model = assetManager.getModel(pendingModel.modelHandle);

            if (model == nullptr)
            {
                return;
            }

            for (auto meshHandle : model->meshAssetHandles)
            {
                auto* mesh = assetManager.getMesh(meshHandle);

                if ((mesh == nullptr) || (mesh->status != AssetStatus::InMemory) || !mesh->handle.isValid())
                {
                    continue;
                }

                // Create one child entity for every mesh handle.
                auto childEntity = commands.createEntity();
                commands.addComponent<Transform>(childEntity, transform);
                commands.addComponent<MeshRef>(childEntity, MeshRef{ .handle = mesh->handle });
                commands.addComponent<LocalBounds>(childEntity, LocalBounds{ .bounds = mesh->bounds });

                // ... todo traverse the actual model node tree to pull out hierarchy and materials ...

                if (pendingModel.fallbackMaterialHandle.isValid())
                {
                    auto materialSlot = pendingModel.fallbackMaterialSlot;

                    if (!materialSlot.isValid())
                    {
                        // ... todo allocate a slot ...
                    }

                    commands.addComponent<MaterialRef>(childEntity, MaterialRef{
                        .handle = pendingModel.fallbackMaterialHandle,
                        .slot = materialSlot
                    });
                }
            }
        }
    }

    void ModelInstantiationSystem::setup(ServiceProvider& services)
    {
        m_pAssetManager = services.get<AssetManager>();
    }

    void ModelInstantiationSystem::prepare()
    {

    }

    void ModelInstantiationSystem::update(SystemData const& data, Entity entity, Transform const& transform, PendingModelInstance const& pendingModel)
    {
        if (!pendingModel.modelHandle.isValid())
        {
            transitionToFailedModelInstance(data.commands, entity, pendingModel.modelHandle);
            return;
        }

        const auto modelAssetStatus = m_pAssetManager->getModelAssetStatus(pendingModel.modelHandle);

        switch (modelAssetStatus)
        {
        case AssetStatus::Loading:
        case AssetStatus::Unloaded:
            // Keep waiting to transition to either Error, InMemory, or Invalid.
            break;

        case AssetStatus::InMemory:
            processModelInstantiation(data.commands, *m_pAssetManager.get(), entity, transform, pendingModel);
            break;

        case AssetStatus::Error:
        case AssetStatus::Invalid:
        default:
            transitionToFailedModelInstance(data.commands, entity, pendingModel.modelHandle);
            break;
        }
    }
}