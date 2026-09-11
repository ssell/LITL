#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/ecs/systems/loadingAssetSystem.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    void LoadingAssetSystem::setup(ServiceProvider& services)
    {
        m_pAssetManager = services.get<AssetManager>();
    }

    void LoadingAssetSystem::prepare()
    {
        // ... no action ...
    }

    void LoadingAssetSystem::update(SystemData const& data, Entity entity, LoadingAsset& loading)
    {
        bool allHandlesInvalid = true;
        bool allAssetsInMemory = true;

        if (loading.materialHandle.isValid())
        {
            const auto status = m_pAssetManager->getMaterialAssetStatus(loading.materialHandle);
            allHandlesInvalid &= (status == AssetStatus::Invalid);
            allAssetsInMemory &= (status == AssetStatus::InMemory);
        }

        if (loading.meshHandle.isValid())
        {
            const auto status = m_pAssetManager->getMeshAssetStatus(loading.meshHandle);
            allHandlesInvalid &= (status == AssetStatus::Invalid);
            allAssetsInMemory &= (status == AssetStatus::InMemory);
        }

        if (loading.modelHandle.isValid())
        {
            const auto status = m_pAssetManager->getModelAssetStatus(loading.modelHandle);
            allHandlesInvalid &= (status == AssetStatus::Invalid);
            allAssetsInMemory &= (status == AssetStatus::InMemory);

            if (status == AssetStatus::InMemory)
            {
                auto* model = m_pAssetManager->getModel(loading.modelHandle);
                
                for (uint32_t i = 0; i < model->meshAssetHandles.size() && i < 10; i++)
                {
                    auto* mesh = m_pAssetManager->getMesh(model->meshAssetHandles[i]);

                    if (mesh != nullptr)
                    {
                        logInfo("Mesh '", mesh->key, "' has status of '", static_cast<uint32_t>(mesh->status.load(std::memory_order::relaxed)), "'");
                    }
                }
            }
        }

        if (loading.shaderHandle.isValid())
        {
            const auto status = m_pAssetManager->getShaderAssetStatus(loading.shaderHandle);
            allHandlesInvalid &= (status == AssetStatus::Invalid);
            allAssetsInMemory &= (status == AssetStatus::InMemory);
        }

        if (loading.textHandle.isValid())
        {
            const auto status = m_pAssetManager->getTextAssetStatus(loading.textHandle);
            allHandlesInvalid &= (status == AssetStatus::Invalid);
            allAssetsInMemory &= (status == AssetStatus::InMemory);
        }

        if (loading.texture2DHandle.isValid())
        {
            const auto status = m_pAssetManager->getTexture2DAssetStatus(loading.texture2DHandle);
            allHandlesInvalid &= (status == AssetStatus::Invalid);
            allAssetsInMemory &= (status == AssetStatus::InMemory);
        }

        if (allHandlesInvalid || allAssetsInMemory)
        {
            data.commands.removeComponent<LoadingAsset>(entity);
            return;
        }
    }
}