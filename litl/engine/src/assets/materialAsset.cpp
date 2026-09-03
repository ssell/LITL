#include "litl-engine/assets/materialAsset.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/material/intermediate/litlmatb.hpp"

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
        import::LitlMatBinary litlmatb;
        BinaryBlockFile::ErrorCode litlmatbError = BinaryBlockFile::ErrorCode::None;

        if (!import::LitlMatBinary::parse(bytes, litlmatb, litlmatbError))
        {
            logError("Failed to parse material asset with error code ", static_cast<uint32_t>(litlmatbError));
            error = AssetErrorCode::ParseFailed;
            return false;
        }

        materialAsset->materialIntermediateData = std::make_shared<import::MaterialIntermediateData>();

        if (!litlmatb.deserialize(*materialAsset->materialIntermediateData, litlmatbError))
        {
            logError("Failed to decode material asset with error code ", static_cast<uint32_t>(litlmatbError));
            error = AssetErrorCode::DeserializationFailed;
            return false;
        }

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
            if (importedData.type == import::ImportedDataType::Material)
            {
                materialAsset->materialIntermediateData = importedData.material->intermediateMaterial;
                return true;
            }
            else
            {
                logError("Import of material bytes from third-party asset failed due to detected import format was not shader but instead format type ", static_cast<uint32_t>(importedData.type));
                error = AssetErrorCode::ExternalFormatImportFailed;
                return false;
            }
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
        MaterialAsset* materialAsset = static_cast<MaterialAsset*>(asset);

        dependencies.clear();
        materialAsset->materialShaderDependencies.clear();

        if (materialAsset->materialIntermediateData == nullptr)
        {
            return true;
        }

        auto& shaders = materialAsset->materialIntermediateData->getShaders();

        for (auto& shader : shaders)
        {
            if (shader.stage != import::LitlMatShaderStage::Unknown)
            {
                auto shaderHandle = assetManager.getShaderHandle(shader.resource);
                auto* shaderAsset = assetManager.getShader(shaderHandle);

                if (shaderAsset != nullptr)
                {
                    dependencies.push_back(shaderAsset);

                    materialAsset->materialShaderDependencies.push_back(MaterialAssetShaderDependency{
                        .stage = static_cast<ShaderStage>(shader.stage),
                        .handle = shaderHandle
                    });
                }
            }
        }
        /*
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
                        auto texture2DHandle = assetManager.getTexture2DHandle(*textureResource);
                        auto* texture2DAsset = assetManager.getTexture2D(texture2DHandle);

                        if (texture2DAsset != nullptr)
                        {
                            dependencies.push_back(texture2DAsset);

                            materialAsset->materialDependencies.push_back(MaterialAssetDependency{
                                .type = AssetType::Texture2D,
                                .handle = texture2DHandle
                            });
                        }
                    }
                    else if (property.type == import::LitlMatPropertyType::Texture3D)
                    {
                        // ... todo add Texture3DAsset support ...
                    }
                }
            }
        }
        */

        return true;
    }

    bool MaterialAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        MaterialAsset* materialAsset = static_cast<MaterialAsset*>(asset);

        if (materialAsset->material == nullptr)
        {
            logError("Processing MaterialAsset '", materialAsset->key, "' failed as material object is null.");
            return false;
        }

        if (materialAsset->materialIntermediateData == nullptr)
        {
            logError("Processing MaterialAsset '", materialAsset->key, "' failed as intermediate data is null.");
            return false;
        }

        const bool success = materialAsset->material->setData({}, *materialAsset->materialIntermediateData, materialAsset->materialShaderDependencies);

        if (!success)
        {
            logError("Failed to create Material object for MaterialAsset '", materialAsset->key, "'");
        }

        materialAsset->materialIntermediateData = nullptr;

        return success;
    }
}