#include "litl-import/texture/intermediate/litlbtex.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/assets/textureAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"

namespace litl
{
    bool TextureAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        TextureAsset* textureAsset = static_cast<TextureAsset*>(asset);
        textureAsset->texture = objectPool.getTexture(textureAsset->handle);
        return (textureAsset->texture != nullptr);
    }

    namespace
    {
        [[nodiscard]] bool decodeLitlTextureBytes(TextureAsset* textureAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
        {
            import::TextureIntermediateData intermediateData{};
            import::LitlTextureBinary litltexture;
            BinaryBlockFile::ErrorCode litltextureError = BinaryBlockFile::ErrorCode::None;

            if (!import::LitlTextureBinary::parse(bytes, litltexture, litltextureError))
            {
                logError("Failed to parse texture asset with error code ", static_cast<uint32_t>(litltextureError));
                error = AssetErrorCode::ParseFailed;
                return false;
            }

            if (!litltexture.deserialize(intermediateData, litltextureError))
            {
                logError("Failed to decode texture asset with error code ", static_cast<uint32_t>(litltextureError));
                error = AssetErrorCode::DeserializationFailed;
                return false;
            }

            // Move only after success
            textureAsset->textureIntermediateData = std::make_shared<import::TextureIntermediateData>(std::move(intermediateData));

            return true;
        }

        [[nodiscard]] bool decodeNonLitlTextureBytes(TextureAsset* textureAsset, AssetRegistration const& assetRegistration, std::span<std::byte const> otherBytes, AssetErrorCode& error) noexcept
        {
            import::ImportService importer{};
            import::ImportedData importedData{};

            const auto importResult = importer.importForMemory(assetRegistration.sourceType, assetRegistration.location, otherBytes, assetRegistration.importSettings, importedData, true);

            if (importResult.success)
            {
                if (importedData.items.size() != 1)
                {
                    error = AssetErrorCode::InvalidImportedItemCount;
                    return false;
                }

                auto& importedItem = importedData.items[0];

                if (importedItem.getType() == import::ImportedDataType::Texture)
                {
                    auto* importedTexture = importedItem.getDataPtr<import::TextureImportResult>();

                    if ((importedTexture != nullptr) && (importedTexture->intermediateTexture != nullptr))
                    {
                        textureAsset->textureIntermediateData = importedTexture->intermediateTexture;
                        return true;
                    }
                    else
                    {
                        logError("Unexpected null imported texture in asset decode.");
                        error = AssetErrorCode::ExternalFormatImportFailed;
                        return false;
                    }
                }
                else
                {
                    logError("Import of texture bytes from third-party asset failed due to detected import format was not texture but instead format type ", static_cast<uint32_t>(importedItem.getType()));
                    error = AssetErrorCode::ExternalFormatImportFailed;
                    return false;
                }
            }
            else
            {
                logError("Failed to import bytes of texture from third-party asset with message '", importResult.message, "' and error code ", static_cast<uint32_t>(importResult.error));
                error = AssetErrorCode::ExternalFormatImportFailed;
                return false;
            }
        }
    }

    bool TextureAsset::decodeBytes(Asset* asset, AssetRegistration const& assetRegistration, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            error = AssetErrorCode::DecodeBytesEmpty;
            return false;
        }

        TextureAsset* textureAsset = static_cast<TextureAsset*>(asset);

        if (assetRegistration.sourceType == import::ImportSourceType::TextureLitlBinary)
        {
            return decodeLitlTextureBytes(textureAsset, bytes, error);
        }
        else
        {
            logWarning("Decoding texture asset with key '", asset->key, "' directly from external format. It is recommended to first convert the mesh to the internal .litlbtex format to improve loading performance.");
            return decodeNonLitlTextureBytes(textureAsset, assetRegistration, bytes, error);
        }

        const auto& dataDescriptor = textureAsset->textureIntermediateData->getDataDescriptor();

        TextureResourceDescriptor resourceDescriptor{
            .width = dataDescriptor.width,
            .height = dataDescriptor.height,
            .depth = dataDescriptor.height,
            .format = dataDescriptor.format,
            .usage = TextureUsageFlagBits::TransferDest | TextureUsageFlagBits::Sampled,
            .memory = BufferMemoryType::Auto,
            .memoryUsage = BufferMemoryUsage::GpuOnly,  // asset-loaded textures are not to be modified
            .sharing = SharingMode::Exclusive,
            .mipLevels = (assetRegistration.importSettings.texture.mipmaps ? mipLevelCount(dataDescriptor.width, dataDescriptor.height, dataDescriptor.depth) : 1u),
            .arrayLayers = 1u,                          // update when adding support for arrays
            .faceCount = 1u,                            // update when adding support for faces
            .sampleCount = MultisampleCount::Count1,    // update when adding support for multi-sampling
            .isCubeMap = false,                         // update when adding support for cubemaps
            .name = textureAsset->key
        };

        resourceDescriptor.calculateDimensionality();

        textureAsset->texture->updateDescriptor({}, resourceDescriptor, false);     // persistsOnCpu to false for now. may change if needed in the future.
    }

    bool TextureAsset::processOnMain(Asset* asset, AssetManager& assetManager, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        TextureAsset* textureAsset = static_cast<TextureAsset*>(asset);

        if (textureAsset->texture == nullptr)
        {
            logError("Processing TextureAsset '", textureAsset->key, "' failed as material object is null.");
            return false;
        }

        if (textureAsset->textureIntermediateData == nullptr)
        {
            logError("Processing TextureAsset '", textureAsset->key, "' failed as intermediate data is null.");
            return false;
        }

        if (textureAsset->texture->setPixelBytes(textureAsset->textureIntermediateData->getPixelBytes()))
        {
            if (!textureAsset->texture->apply(std::nullopt))
            {
                logError("Failed to apply pixel bytes for TextureAsset '", textureAsset->key, "'");
            }
        }
        else
        {
            logError("Failed to set pixel bytes for TextureAsset '", textureAsset->key, "'");
        }

        textureAsset->textureIntermediateData = nullptr;

        return true;
    }
}