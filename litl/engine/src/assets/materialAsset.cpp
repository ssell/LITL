#include "litl-engine/assets/materialAsset.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl
{
    bool MaterialAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        auto* material = static_cast<MaterialAsset*>(asset);
        material->material = objectPool.getMaterial(material->handle);
        return (material->material != nullptr);
    }

    bool decodeLitlMaterialBinaryBytes(MaterialAsset* materialAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return false;
    }

    bool decodeNonLitlMaterialBinaryBytes(MaterialAsset* materialAsset, std::span<std::byte const> otherBytes, AssetErrorCode& error) noexcept
    {
        const auto extension = materialAsset->file.extension();

        import::ImportService importer{};
        import::ImportedData importedData{};

        const auto importResult = importer.import(materialAsset->file, otherBytes, importedData, true);

        if (importResult.success)
        {
            // ... todo ...
            return true;
        }
        else
        {
            logError("Failed to import bytes of material from third-party asset with message '", importResult.message, "' and error code ", static_cast<uint32_t>(importResult.error));
            error = AssetErrorCode::ExternalFormatImportFailed;
            return false;
        }
    }

    bool MaterialAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            error = AssetErrorCode::DecodeBytesEmpty;
            return false;
        }

        MaterialAsset* materialAsset = static_cast<MaterialAsset*>(asset);

        if (materialAsset->file.extension() == ".litlmatb")
        {
            return decodeLitlMaterialBinaryBytes(materialAsset, bytes, error);
        }
        else
        {
            logWarning("Decoding material asset with key '", asset->key, "' directly from external format. It is recommended to first convert the material to the internal .litmatb format to improve loading performance.");
            return decodeNonLitlMaterialBinaryBytes(materialAsset, bytes, error);
        }


        return true;
    }

    bool MaterialAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... no action ...
        return true;
    }

    bool MaterialAsset::gatherDependencies(Asset* asset, AssetManager& assetManager, std::vector<Asset*>& dependencies) noexcept
    {
        dependencies.clear();
        MaterialAsset* materialAsset = static_cast<MaterialAsset*>(asset);

        if (materialAsset->materialIntermediateData == nullptr)
        {
            return true;
        }

        auto& shaders = materialAsset->materialIntermediateData->getShaders();

        for (auto& shader : shaders)
        {
            if (shader.stage != import::LitlMatShaderStage::Unknown)
            {
                auto* shaderAsset = assetManager.getShader(shader.resource);

                if (shaderAsset != nullptr)
                {
                    dependencies.push_back(shaderAsset);
                }
            }
        }

        auto& properties = materialAsset->materialIntermediateData->getProperties();

        for (auto& property : properties)
        {
            if ((property.type == import::LitlMatPropertyType::Texture2D) || (property.type == import::LitlMatPropertyType::Texture3D))
            {
                auto* textureResource = std::get_if<std::string>(&property.value);

                if (textureResource != nullptr)
                {
                    if (property.type == import::LitlMatPropertyType::Texture2D)
                    {
                        auto* texture2DAsset = assetManager.getTexture2D(*textureResource);

                        if (texture2DAsset != nullptr)
                        {
                            dependencies.push_back(texture2DAsset);
                        }
                    }
                    else if (property.type == import::LitlMatPropertyType::Texture3D)
                    {
                        // ... todo add Texture3DAsset support ...
                    }
                }
            }
        }
        

        return true;
    }

    bool MaterialAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        // ... todo ...
        return true;
    }
}