#include <cstring>
#include <limits>
#include <stb_image.h>

#include "litl-import/texture/import/tga.hpp"
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::import
{
    namespace
    {
        struct ScopedData
        {
            uint8_t* data{ nullptr };
            ~ScopedData() { if (data != nullptr) stbi_image_free(data); }
        };

        [[nodiscard]] bool importToIntermediate(std::byte const* data, uint32_t width, uint32_t height, ImportedDataItem& dataItem) noexcept
        {
            auto* textureResult = dataItem.getDataPtr<TextureImportResult>();
            textureResult->intermediateTexture = std::make_shared<TextureIntermediateData>();
            auto& textureDataDescriptor = textureResult->intermediateTexture->getDataDescriptorWriteRef();
            auto& texturePixelData = textureResult->intermediateTexture->getPixelBytesWriteRef();

            textureDataDescriptor.format = DataFormat::RGBA32_SFloat;
            textureDataDescriptor.transfer = TransferFunction::SRGB;
            textureDataDescriptor.width = width;
            textureDataDescriptor.height = height;
            textureDataDescriptor.depth = 1u;
            textureDataDescriptor.arrayLayers = 1u;
            textureDataDescriptor.semantic = TextureSemantic::Albedo;
            textureDataDescriptor.isCubeMap = false;
            textureDataDescriptor.alphaPremultiplied = false;

            if (!textureResult->intermediateTexture->store8BitPixelsAsFloat(std::span<std::byte const>{data, (width * height * 4)}))        // 4 components forced to RGBA
            {
                return false;
            }

            return true;
        }
    }
    TgaImporter::TgaImporter()
    {

    }

    TgaImporter::~TgaImporter()
    {

    }

    Result TgaImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        if (sourceBytes.size() > std::numeric_limits<int>::max())
        {
            return Result::Error(ErrorType::ImporterFailed, "Input texture source bytes too large.");
        }

        int width = 0;
        int height = 0;
        int channels = 0;

        // Note that stb_image automatically handles flipping images to top-left origin if they are not already
        // Note that we do not use stbi_loadf_from_memory as that performs a forced sRGB conversion, and it uses a different EOTF than we do.
        const ScopedData scopedData { .data = stbi_load_from_memory(
            reinterpret_cast<stbi_uc const*>(sourceBytes.data()),
            static_cast<int>(sourceBytes.size_bytes()),
            &width,
            &height,
            &channels,
            4)             // Force to RGBA
        };

        if (scopedData.data == nullptr)
        {
            return Result::Error(ErrorType::ImporterFailed, "stbi_load_from_memory failed.");
        }

        importedData.items.push_back({});
        auto& dataItem = importedData.items.back();

        if (!dataItem.setType(ImportedDataType::Texture))
        {
            return Result::Error(ErrorType::ImporterFailed, "Failed to create texture import data.");
        }

        if (!importToIntermediate(reinterpret_cast<std::byte const*>(scopedData.data), static_cast<uint32_t>(width), static_cast<uint32_t>(height), dataItem))
        {
            return Result::Error(ErrorType::ImporterFailed, "Failed to validate processed TGA data to intermediate format.");
        }

        return Result::Success();
    }
}