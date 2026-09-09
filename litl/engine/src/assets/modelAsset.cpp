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

    bool decodeLitlModelBytes(ModelAsset* modelAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
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

    bool decodeNonLitlModelBytes(ModelAsset* modelAsset, std::span<std::byte const> otherBytes, AssetErrorCode& error) noexcept
    {
        const auto extension = modelAsset->file.extension();

        import::ImportService importer{};
        import::ImportedData importedData{};

        const auto importResult = importer.import(modelAsset->file, otherBytes, importedData, true);

        if (importResult.success)
        {
            importedData.calculateTypeCounts();

            modelAsset->meshAssetHandles.reserve(importedData.getTypeCount(import::ImportedDataType::Mesh));
            // ... todo other types ...

            for (auto& importedItem : importedData.items)
            {
                if (importedItem.getType() == import::ImportedDataType::Mesh)
                {
                    // ... todo register/import as a distinct Mesh Asset in the Asset Manager ...
                }
                else
                {
                    // ... todo other types ...
                }
            }
            
            return true;
        }
        else
        {
            logError("Failed to import bytes of model from third-party asset with message '", importResult.message, "' and error code ", static_cast<uint32_t>(importResult.error));
            error = AssetErrorCode::ExternalFormatImportFailed;
            return false;
        }
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
        // ... todo ...
        return true;
    }

    bool ModelAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }
}