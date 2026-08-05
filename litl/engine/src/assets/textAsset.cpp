#include "litl-engine/assets/textAsset.hpp"
#include "litl-core/stringId.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    namespace
    {
        enum class TextFileExtension : uint8_t
        {
            Unknown = 0u,
            Txt     = 1u,
            Json    = 2u
        };

        static const StringIdMap<TextFileExtension> g_textFileExtensionMap = {
            { ".txt"_sid, TextFileExtension::Txt },
            { ".json"_sid, TextFileExtension::Json }
        };

        static bool decodeBytesTxt(Asset* asset, std::span<std::byte const> bytes) noexcept
        {
            return true;
        }

        static bool decodeBytesJson(Asset* asset, std::span<std::byte const> bytes) noexcept
        {
            return true;
        }
    }

    bool TextAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        auto* text = static_cast<TextAsset*>(asset);
        text->text = objectPool.getText(text->handle);
        return (text->text != nullptr);
    }

    bool TextAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes) noexcept
    {
        if (bytes.empty())
        {
            return false;
        }

        auto findExtensionType = g_textFileExtensionMap.find(asset->file.extension());

        if (findExtensionType == g_textFileExtensionMap.end())
        {
            // Unsupported extension type.
            return false;
        }

        switch (findExtensionType->second)
        {
        case TextFileExtension::Txt:
            return decodeBytesTxt(asset, bytes);

        case TextFileExtension::Json:
            return decodeBytesJson(asset, bytes);

        // Somehow still an unsupported extension type.
        case TextFileExtension::Unknown:
        default:
            return false;
        }
    }
}