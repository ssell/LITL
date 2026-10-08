#include <deque>
#include <format>

#include "litl-core/assert.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/material.hpp"
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

        void processModelInstantiation(EntityCommands& commands, AssetManager& assetManager, ObjectPool& objectPool, Entity entity, PendingModelInstance const& pendingModel, Transform const& transform, LocalBounds& localBounds) noexcept
        {
            // -----------------------------------------------------------------------------
            // Swap out PendingModelInstance for either ModelInstance or FailedModelInstance
            // -----------------------------------------------------------------------------

            commands.removeComponent<PendingModelInstance>(entity);

            const ModelAsset* modelAsset = assetManager.getModel(pendingModel.modelHandle);

            if ((modelAsset == nullptr) || (modelAsset->modelIntermediateData == nullptr))
            {
                commands.addComponent<FailedModelInstance>(entity, FailedModelInstance{ .modelHandle = pendingModel.modelHandle });
                return;
            }

            commands.addComponent<ModelInstance>(entity, ModelInstance{ .modelHandle = pendingModel.modelHandle });

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

            const auto meshNames = modelAsset->modelIntermediateData->getMeshNames();
            const auto nodes = modelAsset->modelIntermediateData->getNodes();
            const auto rootNodeIndices = modelAsset->modelIntermediateData->getRootNodes();

            std::deque<PendingModelNode> frontierNodes;

            for (auto rootNodeIndex : rootNodeIndices)
            {
                if (rootNodeIndex < nodes.size())
                {
                    frontierNodes.push_back(PendingModelNode{
                        .index = rootNodeIndex
                    });
                }
            }

            uint32_t cycles = 0u;       // Protect against cyclic node hierarchies

            while (!frontierNodes.empty() && (cycles++ < nodes.size()))
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

                commands.addComponent<Transform>(nodeEntity, Transform::create(mat4(node.localTransform)));

                if (node.meshIndex < modelAsset->meshAssetHandles.size())
                {
                    const auto* meshAsset = assetManager.getMesh(modelAsset->meshAssetHandles[node.meshIndex]);

                    if ((meshAsset != nullptr) && meshAsset->handle.isValid())
                    {
                        commands.addComponent<MeshRef>(nodeEntity, MeshRef{ .handle = meshAsset->handle });
                        commands.addComponent<LocalBounds>(nodeEntity, LocalBounds{ .bounds = meshAsset->bounds });

                        if (node.materialIndices.size() <= 1ull)
                        {
                            // This mesh has zero or one materials. Use a MaterialRef with either the single material or the fallback material.
                            MaterialRef materialRef = fallbackMaterialRef;

                            if ((node.materialIndices.size() == 1ull) && (node.materialIndices[0] < modelAsset->materialAssetHandles.size()))
                            {
                                auto* materialAsset = assetManager.getMaterial(modelAsset->materialAssetHandles[(node.materialIndices[0])]);

                                if ((materialAsset != nullptr) && (materialAsset->material != nullptr) && materialAsset->materialHandle.isValid())
                                {
                                    materialRef.handle = materialAsset->materialHandle;
                                    materialRef.slot = materialAsset->material->allocateSlot();
                                }
                            }

                            if (materialRef.handle.isValid())
                            {
                                commands.addComponent<MaterialRef>(nodeEntity, materialRef);
                            }
                            else
                            {
                                logWarning("Model '", modelAsset->key, "' node '", node.name, "' has no valid material and no fallback was supplied. It will not be rendered.");
                            }
                        }
                        else
                        {
                            // Mesh has multiple materials. Use a VariableMaterialRef
                            MaterialBindingsDescriptor bindingsDesc{};
                            bindingsDesc.bindings.reserve(node.materialIndices.size());

                            for (auto materialIndex : node.materialIndices)
                            {
                                auto* materialAsset = assetManager.getMaterial(modelAsset->materialAssetHandles[materialIndex]);

                                if ((materialAsset != nullptr) && (materialAsset->material != nullptr) && materialAsset->materialHandle.isValid())
                                {
                                    bindingsDesc.bindings.push_back(MaterialBinding{
                                        .handle = materialAsset->materialHandle,
                                        .slot = materialAsset->material->allocateSlot()
                                    });
                                }
                            }

                            if (!bindingsDesc.bindings.empty())
                            {
                                const VariableMaterialsRef variableMaterialsRef{
                                    .handle = objectPool.createMaterialBindings(bindingsDesc)
                                };

                                if (variableMaterialsRef.handle.isValid())
                                {
                                    commands.addComponent<VariableMaterialsRef>(nodeEntity, variableMaterialsRef);
                                }
                                else
                                {
                                    logWarning("Model '", modelAsset->key, "' node '", node.name, "' failed to create MaterialBindingsHandle and so no VariableMaterialsRef component was added. It will not be rendered.");
                                }
                            }
                            else
                            {
                                logWarning("Model '", modelAsset->key, "' node '", node.name, "' declared multiple materials but failed to retrieve any valid material bindings. No VariableMaterialsRef component was added. It will not be rendered.");
                            }
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

            if (cycles > nodes.size())
            {
                logWarning("ModelInstantiationSystem encountered model node cycle for model asset '", modelAsset->key, "'");
            }
        }
    }

    void ModelInstantiationSystem::setup(ServiceProvider& services)
    {
        m_pAssetManager = services.get<AssetManager>();
        m_pObjectPool = services.get<ObjectPool>();

        LITL_FATAL_ASSERT_MSG(m_pAssetManager != nullptr, "Failed to inject AssetManager into ModelInstantiationSystem");
        LITL_FATAL_ASSERT_MSG(m_pObjectPool != nullptr, "Failed to inject ObjectPool into ModelInstantiationSystem");
    }

    void ModelInstantiationSystem::prepare()
    {
        // ... intentionally empty ...
    }

    void ModelInstantiationSystem::update(SystemData const& data, Entity entity, PendingModelInstance const& pendingModel, Transform const& transform, LocalBounds& localBounds)
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
            processModelInstantiation(data.commands, *m_pAssetManager.get(), *m_pObjectPool.get(), entity, pendingModel, transform, localBounds);
            break;

        case AssetStatus::Error:
        case AssetStatus::Invalid:
        default:
            transitionToFailedModelInstance(data.commands, entity, pendingModel.modelHandle);
            break;
        }
    }
}