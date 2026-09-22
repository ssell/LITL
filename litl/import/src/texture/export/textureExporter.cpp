#include <format>

#include "litl-import/texture/export/textureExporter.hpp"
#include "litl-import/texture/intermediate/litlbtex.hpp"

namespace litl::import
{
    TextureExporter::TextureExporter()
    {

    }

    TextureExporter::~TextureExporter()
    {

    }

    Result TextureExporter::prepare(ImportedData& data, ImportSettings const& settings, uint32_t dataIndex) noexcept
    {
        // ... todo mip generation ...
        // ... todo bc encoding ...

        return Result::Success();
    }

    Result TextureExporter::write(std::vector<std::byte>& serialized, ImportedData const& data, uint32_t dataIndex) noexcept
    {
        auto errorCode = BinaryBlockFile::ErrorCode::None;
        auto* textureResult = data.items[dataIndex].getDataPtr<TextureImportResult>();

        if (textureResult == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        auto* texture = textureResult->intermediateTexture.get();

        if (!LitlTextureBinary::serialize(*texture, serialized, errorCode))
        {
            return Result::Error(ErrorType::SerializationFailed, std::format("Serialization of Texture to litlbtex failed with error code {}", static_cast<uint32_t>(errorCode)));
        }

        if (serialized.empty())
        {
            return Result::Error(ErrorType::SerializedResultEmpty);
        }

        return Result::Success();
    }
}