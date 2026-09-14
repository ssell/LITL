#include <deque>
#include <format>
#include <optional>

#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/ecs/systems/modelInstantiationSystem.hpp"
#include "litl-import/model/intermediate/modelIntermediateData.hpp"

namespace litl
{
    namespace
    {
        struct PendingModelNode
        {
            DeferredEntity parent{};
            uint32_t index{ Constants::uint32_null_index };
        };

        void transitionToFailedModelInstance(EntityCommands& commands, Entity entity, ModelAssetHandle modelHandle) noexcept
        {
            commands.removeComponent<PendingModelInstance>(entity);
            commands.addComponent<FailedModelInstance>(entity, FailedModelInstance{ .modelHandle = modelHandle });
        }

        void processModelInstantiation(EntityCommands& commands, AssetManager& assetManager, Entity entity, Transform const* rootTransform, PendingModelInstance const& pendingModel) noexcept
        {
            // -----------------------------------------------------------------------------
            // Swap out PendingModelInstance for either ModelInstance or FailedModelInstance
            // -----------------------------------------------------------------------------

            commands.removeComponent<PendingModelInstance>(entity);

            const ModelAsset* model = assetManager.getModel(pendingModel.modelHandle);

            if ((model == nullptr) || (model->modelIntermediateData == nullptr))
            {
                commands.addComponent<FailedModelInstance>(entity, FailedModelInstance{ .modelHandle = pendingModel.modelHandle });
                return;
            }

            // -----------------------------------------------------------------------------
            // Add a Transform if one is not present
            // -----------------------------------------------------------------------------

            commands.addComponent<ModelInstance>(entity, ModelInstance{ .modelHandle = pendingModel.modelHandle });
            Transform transform{};

            if (rootTransform == nullptr)
            {
                commands.addComponent<Transform>(entity, transform);
            }
            else
            {
                transform = *rootTransform;
            }

            // -----------------------------------------------------------------------------
            // Create the fallback MaterialRef
            // -----------------------------------------------------------------------------

            MaterialRef fallbackMaterialRef{};

            if (pendingModel.fallbackMaterialHandle.isValid())
            {
                fallbackMaterialRef.handle = pendingModel.fallbackMaterialHandle;

                if (pendingModel.fallbackMaterialSlot.isValid())
                {
                    fallbackMaterialRef.slot = pendingModel.fallbackMaterialSlot;
                }
                else
                {
                    // ... todo allocate a slot ...
                }
            }

            // -----------------------------------------------------------------------------
            // Loop over the Model node hierarchy
            // -----------------------------------------------------------------------------

            const auto meshNames = model->modelIntermediateData->getMeshNames();
            const auto nodes = model->modelIntermediateData->getNodes();
            const auto rootNodeIndices = model->modelIntermediateData->getRootNodes();
            
            std::deque<PendingModelNode> frontierNodes;

            for (auto rootNodeIndex : rootNodeIndices)
            {
                if (rootNodeIndex < nodes.size())
                {
                    frontierNodes.push_back(PendingModelNode {
                        .index = rootNodeIndex
                    });
                }
            }

            uint32_t cycles = 0u;       // Protect against cyclic node hierarchies

            while (!frontierNodes.empty() && (cycles++ <= nodes.size()))
            {
                // -------------------------------------------------------------------------
                // Pop from the frontier and create the new child node for the mesh if valid
                // -------------------------------------------------------------------------

                const auto pendingNode = frontierNodes.front(); frontierNodes.pop_front();
                const auto& node = nodes[pendingNode.index];
                const auto nodeEntity = commands.createEntity();

                if (!pendingNode.parent.isNull())
                {
                    commands.setParent(nodeEntity, pendingNode.parent);
                }
                else
                {
                    commands.setParent(nodeEntity, entity);
                }

                commands.addComponent<Transform>(nodeEntity, Transform::create(node.localTransform));

                if (node.meshIndex.has_value() && (node.meshIndex.value() < meshNames.size()))
                {
                    const auto meshAssetName = std::format("{}/{}", model->key, meshNames[node.meshIndex.value()]);
                    const auto* mesh = assetManager.getMesh(meshAssetName);

                    if ((mesh != nullptr) && mesh->handle.isValid())
                    {
                        commands.addComponent<MeshRef>(nodeEntity, MeshRef{ .handle = mesh->handle });
                        commands.addComponent<LocalBounds>(nodeEntity, LocalBounds{ .bounds = mesh->bounds });

                        if (false /* todo materials from the model */)
                        {
                            // ... todo ...
                        }
                        else
                        {
                            commands.addComponent<MaterialRef>(nodeEntity, fallbackMaterialRef);
                        }
                    }


                }

                // -------------------------------------------------------------------------
                // Add all valid child indices to the frontier
                // -------------------------------------------------------------------------

                if (!node.children.empty())
                {
                    for (auto childNodeIndex : node.children)
                    {
                        if (childNodeIndex >= nodes.size())
                        {
                            continue;
                        }

                        frontierNodes.push_back(PendingModelNode{
                            .parent = nodeEntity,
                            .index = childNodeIndex
                        });
                    }
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

    void ModelInstantiationSystem::update(SystemData const& data, Entity entity, Transform const* transform, PendingModelInstance const& pendingModel)
    {
        if (!pendingModel.modelHandle.isValid())
        {
            transitionToFailedModelInstance(data.commands, entity, pendingModel.modelHandle);
            return;
        }

        const auto modelAssetStatus = m_pAssetManager->getModelAssetStatus(pendingModel.modelHandle);

        switch (modelAssetStatus)
        {
        case AssetStatus::Unloaded:
            // Trigger the model load if it has not yet happened.
            std::ignore = m_pAssetManager->getModel(pendingModel.modelHandle);
            break;

        case AssetStatus::Loading:
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