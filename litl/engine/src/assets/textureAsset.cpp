#include <cstring>

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

        [[nodiscard]] bool decodeNonLitlTextureBytes(TextureAsset* textureAsset, AssetRegistration const& assetRegistration, std::span<std::byte const> otherBytes, std::span<import::ImportCompanion const> companions, AssetErrorCode& error) noexcept
        {
            import::ImportService importer{};
            import::ImportedData importedData{};

            const auto importResult = importer.importForMemory(assetRegistration.sourceType, assetRegistration.location, otherBytes, assetRegistration.importSettings, companions, importedData, true);

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

        static_assert(static_cast<size_t>(TextureChannel::R) == 1u);
        static_assert(static_cast<size_t>(TextureChannel::G) == 2u);
        static_assert(static_cast<size_t>(TextureChannel::B) == 3u);
        static_assert(static_cast<size_t>(TextureChannel::A) == 4u);

        [[nodiscard]] bool isolateTextureChannel(std::span<std::byte const> source, std::vector<std::byte>& isolated, DataFormat sourceFormat, DataFormat& destFormat, TextureChannel channel, std::string_view location) noexcept
        {
            if ((channel == TextureChannel::All) || (static_cast<uint32_t>(channel) > static_cast<uint32_t>(TextureChannel::A)))
            {
                return false;
            }

            size_t channelBytes = 0ull;

            switch (sourceFormat)
            {
            case DataFormat::RGBA8_UNorm:
                channelBytes = 1ull;
                destFormat = DataFormat::R8_UNorm;
                break;

            case DataFormat::RGBA32_SFloat:
                channelBytes = 4ull;
                destFormat = DataFormat::R32_SFloat;
                break;

            default:
                // Unsupported texture format for automatic channel isolation.
                logWarning("Texture Asset '", location, "' specified channel isolation for unsupported format ", getDataFormatName(sourceFormat), " (", static_cast<uint32_t>(sourceFormat), "). Supported formats: RGBA8_UNorm or RGBA32_SFloat.");
                return false;
            }

            if ((source.size() % (channelBytes * 4)) != 0)
            {
                logWarning("Unexpected source byte size of ", source.size(), " during channel isolation for texture at '", location, "'");
                return false;
            }

            const size_t texelCount = source.size() / (channelBytes * 4ull);
            isolated.resize(texelCount * channelBytes);

            const size_t channelOffset = (static_cast<size_t>(channel) - 1ull) * channelBytes;
            const std::byte* __restrict psrc = source.data();
            std::byte* __restrict pdst = isolated.data();

            // Branch once outside the loop so that the size is a compile constant which clang emits as a single load and store.
            if (channelBytes == 1ull)
            {
                for (size_t i = 0ull; i < texelCount; ++i)
                {
                    std::memcpy(pdst + i, psrc + (i * 4ull) + channelOffset, 1ull);
                }
            }
            else // if (channelBytes == 4ull)
            {
                for (size_t i = 0ull; i < texelCount; ++i)
                {
                    std::memcpy(pdst + (i * 4ull), psrc + (i * 16ull) + channelOffset, 4ull);
                }
            }

            return true;
        }
    }

    bool TextureAsset::decodeBytes(Asset* asset, AssetRegistration const& assetRegistration, std::span<std::byte const> bytes, std::span<import::ImportCompanion const> companions, AssetErrorCode& error) noexcept
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
            return decodeNonLitlTextureBytes(textureAsset, assetRegistration, bytes, companions, error);
        }
    }

    bool TextureAsset::processOnMain(Asset* asset, AssetRegistration const& assetRegistration, AssetManager& assetManager, ObjectPool& objectPool, AssetErrorCode& error) noexcept
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

        const auto& dataDescriptor = textureAsset->textureIntermediateData->getDataDescriptor();

        TextureResourceDescriptor resourceDescriptor{
            .width = dataDescriptor.width,
            .height = dataDescriptor.height,
            .depth = dataDescriptor.depth,
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
            .residesInTextureTable = assetRegistration.importSettings.texture.residesInTextureTable,
            .name = textureAsset->key
        };

        std::span<std::byte const> sourcePixels = textureAsset->textureIntermediateData->getPixelBytes();
        std::vector<std::byte> isolatedChannel;
        DataFormat isolatedFormat = dataDescriptor.format;

        if (isolateTextureChannel(sourcePixels, isolatedChannel, dataDescriptor.format, isolatedFormat, assetRegistration.importSettings.texture.channel, textureAsset->key))
        {
            sourcePixels = isolatedChannel;
            resourceDescriptor.format = isolatedFormat;
        }

        resourceDescriptor.calculateDimensionality();
        textureAsset->texture->updateDescriptor({}, resourceDescriptor, false);     // persistsOnCpu to false for now. may change if needed in the future.

        if (textureAsset->texture->setPixelBytes(sourcePixels))
        {
            if (!textureAsset->texture->apply(std::nullopt))
            {
                logError("Failed to apply pixel bytes for TextureAsset '", textureAsset->key, "'");
                return false;
            }
        }
        else
        {
            logError("Failed to set pixel bytes for TextureAsset '", textureAsset->key, "'");
            return false;
        }

        textureAsset->textureIntermediateData = nullptr;

        return true;
    }
}