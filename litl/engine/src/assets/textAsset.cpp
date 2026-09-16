#include "litl-engine/assets/textAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    namespace
    {
        [[nodiscard]] constexpr TextType resolveType(import::ImportSourceType sourceType) noexcept
        {
            switch (sourceType)
            {
            case import::ImportSourceType::TextPlain:
                return TextType::Plain;

            case import::ImportSourceType::TextJson:
                return TextType::Json;

            default:
                return TextType::Unknown;
            }
        }
    }

    bool TextAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        auto* text = static_cast<TextAsset*>(asset);
        text->text = objectPool.getText(text->handle);
        return (text->text != nullptr);
    }

    bool TextAsset::decodeBytes(Asset* asset, AssetRegistration const& assetRegistration, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            error = AssetErrorCode::DecodeBytesEmpty;
            return false;
        }

        auto* textAsset = static_cast<TextAsset*>(asset);

        if ((textAsset == nullptr) || (textAsset->text == nullptr))
        {
            error = AssetErrorCode::DecodeAssetNull;
            return false;
        }

        const bool result = textAsset->text->create(Authority<TextAsset>{}, TextDescriptor{
            .objectInfo = ObjectDescriptor{ .name = textAsset->key, .lifetime = ObjectLifetime::Application },
            .string = std::string(reinterpret_cast<char const*>(bytes.data()), bytes.size()),
            .type = resolveType(assetRegistration.sourceType)
        });

        if (!result)
        {
            error = AssetErrorCode::CreationFailed;
        }

        return result;
    }

    bool TextAsset::processOnMain(Asset* asset, AssetManager& assetManager, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        // ... no action ...
        return true;
    }
}