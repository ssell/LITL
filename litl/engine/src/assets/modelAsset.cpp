#include <format>

#include "litl-engine/assets/modelAsset.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/model/intermediate/litlmdl.hpp"

namespace litl
{
    bool ModelAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        // There is no underlying Model game object.
        // A model is a collection of meshes, materials, etc. But those may be used independently of the model itself.
        return true;
    }

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
        const auto extension = modelAsset->file.extension();

        import::ImportService importer{};

        modelAsset->importedData = std::make_shared<import::ImportedData>();
        const auto importResult = importer.import(modelAsset->file, otherBytes, *modelAsset->importedData.get(), true);

        if (!importResult.success)
        {
            logError("Failed to import bytes of model from third-party asset with message '", importResult.message, "' and error code ", static_cast<uint32_t>(importResult.error));
            error = AssetErrorCode::ExternalFormatImportFailed;
            return false;
        }

        if (modelAsset->importedData->items.empty())
        {
            logError("Failed to import bytes of model from third-party asset as the imported items count is 0.");
            error = AssetErrorCode::DecodeBytesResultEmpty;
            return false;
        }

        modelAsset->importedData->calculateTypeCounts();
        const auto modelCount = modelAsset->importedData->getTypeCount(import::ImportedDataType::Model);

        if (modelCount != 1u)
        {
            logError("Failed to import bytes of model from third-party asset as the expected model count is 1 but the returned model count is ", modelCount);
            error = AssetErrorCode::DecodeBytesUnexpectedResult;
            return false;
        }

        uint32_t modelIndex = Constants::uint32_null_index;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(modelAsset->importedData->items.size()); ++i)
        {
            if (modelAsset->importedData->items[i].getType() == import::ImportedDataType::Model)
            {
                modelIndex = i;
                break;
            }
        }

        if (modelIndex == Constants::uint32_null_index)
        {
            logError("Failed to import bytes of model from third-party asset as the importer failed to find the expected model item index.");
            error = AssetErrorCode::DecodeBytesUnexpectedResult;
            return false;
        }

        auto& modelDataItem = modelAsset->importedData->items[0];
        auto* modelResult = modelDataItem.getDataPtr<import::ModelImportResult>();

        if ((modelResult == nullptr) || (modelResult->model == nullptr))
        {
            logError("Failed to import bytes of model from third-party asset as the model intermediate data is null.");
            error = AssetErrorCode::DecodeBytesUnexpectedResult;
            return false;
        }

        // Move from the private internal result to the public modelIntermediateData
        modelAsset->modelIntermediateData = std::move(modelResult->model);

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

    bool ModelAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... todo ...
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

        modelAsset->meshAssetHandles.resize(modelAsset->modelIntermediateData->getMeshNames().size(), {});
        //modelAsset->materialAssetHandles.resize(modelAsset->modelIntermediateData->getMaterialNames().size(), {});
        // ^ todo 

        for (auto& dataItem : modelAsset->importedData->items)
        {
            if (auto* meshItem = dataItem.getDataPtr<import::MeshImportResult>(); meshItem != nullptr)
            {
                auto meshHandle = assetManager.createMeshAssetFromMemory(dataItem.getName(), std::move(*meshItem->mesh));
                auto* meshAsset = assetManager.getMesh(meshHandle);

                if (meshAsset != nullptr)
                {
                    dependencies.push_back(meshAsset);
                    modelAsset->meshAssetHandles.push_back(meshHandle);
                }
                else
                {
                    logWarning("Failed to gather Mesh dependency '", dataItem.getName(), "' for Model '", modelAsset->key, "'");
                }
            }
            else if (auto* materialItem = dataItem.getDataPtr<import::MaterialImportResult>(); materialItem != nullptr)
            {
                auto materialHandle = assetManager.createMaterialAssetFromMemory(dataItem.getName(), std::move(*materialItem->intermediateMaterial));
                auto* materialAsset = assetManager.getMaterial(materialHandle);

                if (materialAsset != nullptr)
                {
                    dependencies.push_back(materialAsset);
                    modelAsset->materialAssetHandles.push_back(materialHandle);
                }
                else
                {
                    logWarning("Failed to gather Material dependency '", dataItem.getName(), "' for Model '", modelAsset->key, "'");
                }
            }
        }

        // All imported data has been moved.
        modelAsset->importedData.reset();

        return true;
    }

    bool ModelAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }
}