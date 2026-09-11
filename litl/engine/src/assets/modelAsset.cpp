#include <format>

#include "litl-engine/assets/modelAsset.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/model/intermediate/litlmdl.hpp"

namespace litl
{
    bool ModelAsset::decodeLitlModelBytes(ModelAsset* modelAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        import::LitlModel litlmdl;
        import::LitlModel::ErrorCode litlmdlError = import::LitlModel::ErrorCode::None;

        if (!import::LitlModel::parse(bytes, litlmdl, litlmdlError))
        {
            logError("Failed  to parse model asset with error code ", static_cast<uint32_t>(litlmdlError));
            error = AssetErrorCode::ParseFailed;
            return false;
        }

        modelAsset->modelIntermediateData = std::make_shared<import::ModelIntermediateData>();

        if (!litlmdl.deserialize(*modelAsset->modelIntermediateData, litlmdlError))
        {
            logError("Failed to decode model asset with error code ", static_cast<uint32_t>(litlmdlError));
            error = AssetErrorCode::DeserializationFailed;
            return false;
        }

        return true;
    }

    bool ModelAsset::decodeNonLitlModelBytes(ModelAsset* modelAsset, std::span<std::byte const> otherBytes, AssetErrorCode& error) noexcept
    {
        import::ImportService importer{};
        import::ImportedData importedData{};

        const auto importResult = importer.import(modelAsset->file, otherBytes, importedData, true);

        if (!importResult.success)
        {
            logError("Failed to import bytes of model from third-party asset with message '", importResult.message, "' and error code ", static_cast<uint32_t>(importResult.error));
            error = AssetErrorCode::ExternalFormatImportFailed;
            return false;
        }

        if (importedData.items.empty())
        {
            logError("Failed to import bytes of model from third-party asset as the imported items count is 0.");
            error = AssetErrorCode::DecodeBytesResultEmpty;
            return false;
        }

        importedData.calculateTypeCounts();
        const auto modelCount = importedData.getTypeCount(import::ImportedDataType::Model);

        if (modelCount != 1u)
        {
            logError("Failed to import bytes of model from third-party asset as the expected model count is 1 but the returned model count is ", modelCount);
            error = AssetErrorCode::DecodeBytesUnexpectedResult;
            return false;
        }

        const auto modelIndex = importedData.getFirstIndexOfType(import::ImportedDataType::Model);

        if (!modelIndex.has_value())
        {
            logError("Failed to import bytes of model from third-party asset as the importer failed to find the expected model item index.");
            error = AssetErrorCode::DecodeBytesUnexpectedResult;
            return false;
        }

        auto& modelDataItem = importedData.items[modelIndex.value()];
        auto* modelResult = modelDataItem.getDataPtr<import::ModelImportResult>();

        if ((modelResult == nullptr) || (modelResult->model == nullptr))
        {
            logError("Failed to import bytes of model from third-party asset as the model intermediate data is null.");
            error = AssetErrorCode::DecodeBytesUnexpectedResult;
            return false;
        }

        // Move from the private internal result to the public modelIntermediateData
        modelAsset->modelIntermediateData = std::move(modelResult->model);

        // Move imported data to the model asset after success
        modelAsset->importedData = std::make_shared<import::ImportedData>(std::move(importedData));

        return true;
    }

    bool ModelAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            error = AssetErrorCode::DecodeBytesEmpty;
            return false;
        }

        ModelAsset* modelAsset = static_cast<ModelAsset*>(asset);

        if (modelAsset->file.extension() == ".litlmdl")
        {
            return decodeLitlModelBytes(modelAsset, bytes, error);
        }
        else
        {
            logWarning("Decoding model asset with key '", asset->key, "' directly from external format. It is recommended to first convert the material to the internal .litlmdl format to improve loading performance.");
            return decodeNonLitlModelBytes(modelAsset, bytes, error);
        }
    }

    bool ModelAsset::gatherDependenciesFromLitlModel(ModelAsset* modelAsset, AssetManager& assetManager, std::span<std::string const> meshNames, std::span<std::string const> materialNames, std::vector<Asset*>& dependencies) noexcept
    {
        // ... todo ...
        return false;
    }

    /// <summary>
    /// Traverses the model data items and builds proper keys for each item. The imported object data (mesh, material, etc.) is then used to create a new asset at the key.
    /// </summary>
    bool ModelAsset::gatherDependenciesFromNonLitlModel(ModelAsset* modelAsset, AssetManager& assetManager, std::span<std::string const> meshNames, std::span<std::string const> materialNames, std::vector<Asset*>& dependencies) noexcept
    {
        const auto modelIndex = modelAsset->importedData->getFirstIndexOfType(import::ImportedDataType::Model);

        if (!modelIndex.has_value())
        {
            logWarning("Failed to get Model imported data item while gathering dependencies for Model '", modelAsset->key, "'");
            return false;
        }

        auto* modelDataPtr = modelAsset->importedData->items[modelIndex.value()].getDataPtr<import::ModelImportResult>();

        if (modelDataPtr == nullptr)
        {
            logWarning("Failed to get Model imported data item pointer while gathering dependencies for Model '", modelAsset->key, "'");
            return false;
        }

        for (auto& modelDataItem : modelDataPtr->dataItems)
        {
            if (modelDataItem.importedDataItemIndex >= modelAsset->importedData->items.size())
            {
                continue;
            }

            auto& importedDataItem = modelAsset->importedData->items[modelDataItem.importedDataItemIndex];

            if (auto* meshItem = importedDataItem.getDataPtr<import::MeshImportResult>(); meshItem != nullptr)
            {
                if (modelDataItem.modelNameIndex >= meshNames.size())
                {
                    logWarning("Found invalid mesh name index while gathering dependencies for Model '", modelAsset->key, "'");
                    continue;
                }

                if (meshItem->mesh == nullptr)
                {
                    logWarning("Found null mesh reference while gathering dependencies for Model '", modelAsset->key, "'");
                    continue;
                }

                auto const& meshName = meshNames[modelDataItem.modelNameIndex];
                auto meshHandle = assetManager.createMeshAssetFromMemory({}, std::format("{}/{}", modelAsset->key, meshName), std::move(*meshItem->mesh), modelAsset->file);
                auto* meshAsset = assetManager.getMesh(meshHandle);

                if (meshAsset != nullptr)
                {
                    dependencies.push_back(meshAsset);
                    modelAsset->meshAssetHandles[modelDataItem.modelNameIndex] = meshHandle;
                }
                else
                {
                    logWarning("Failed to gather Mesh dependency '", importedDataItem.getName(), "' for Model '", modelAsset->key, "'");
                }
            }
            else if (auto* materialItem = importedDataItem.getDataPtr<import::MaterialImportResult>(); materialItem != nullptr)
            {
                if (modelDataItem.modelNameIndex >= materialNames.size())
                {
                    logWarning("Found invalid material name index while gathering dependencies for Model '", modelAsset->key, "'");
                    continue;
                }

                if (materialItem->intermediateMaterial == nullptr)
                {
                    logWarning("Found null material reference while gathering dependencies for Model '", modelAsset->key, "'");
                    continue;
                }

                auto const& materialName = materialNames[modelDataItem.modelNameIndex];
                auto materialHandle = assetManager.createMaterialAssetFromMemory({}, std::format("{}/{}", modelAsset->key, materialName), std::move(*materialItem->intermediateMaterial), modelAsset->file);
                auto* materialAsset = assetManager.getMaterial(materialHandle);

                if (materialAsset != nullptr)
                {
                    dependencies.push_back(materialAsset);
                    modelAsset->materialAssetHandles[modelDataItem.modelNameIndex] = materialHandle;
                }
                else
                {
                    logWarning("Failed to gather Material dependency '", importedDataItem.getName(), "' for Model '", modelAsset->key, "'");
                }
            }
        }

        return true;
    }

    bool ModelAsset::gatherDependencies(Asset* asset, AssetManager& assetManager, std::vector<Asset*>& dependencies) noexcept
    {
        ModelAsset* modelAsset = static_cast<ModelAsset*>(asset);
        dependencies.clear();

        if (modelAsset->modelIntermediateData == nullptr)
        {
            return true;
        }

        auto meshNames = modelAsset->modelIntermediateData->getMeshNames();
        auto materialNames = modelAsset->modelIntermediateData->getMaterialNames();

        modelAsset->meshAssetHandles.clear();
        modelAsset->meshAssetHandles.resize(meshNames.size(), {});

        modelAsset->materialAssetHandles.clear();
        modelAsset->materialAssetHandles.resize(materialNames.size(), {});

        // Split in logic because loading from a .litlmdl will NOT have an ImportedData while loading from a third-party will have an ImportedData.
        /// With the .litlmdl we already have fully formed asset keys. With the third-party data we have locally unique item names but not actual asset keys.
        if (modelAsset->importedData == nullptr)
        {
            return gatherDependenciesFromLitlModel(modelAsset, assetManager, meshNames, materialNames, dependencies);
        }
        else
        {
            const bool success = gatherDependenciesFromNonLitlModel(modelAsset, assetManager, meshNames, materialNames, dependencies);
            modelAsset->importedData.reset();
            return success;
        }
    }

    bool ModelAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }
}