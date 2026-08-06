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

        static bool decodeBytesTxt(Authority<TextAsset> auth, TextAsset* asset, std::span<std::byte const> bytes) noexcept
        {
            std::string str(reinterpret_cast<char const*>(bytes.data()), bytes.size());

            return asset->text->create(auth, TextDescriptor{
                .objectInfo = ObjectDescriptor{
                    .name = asset->key,
                    .lifetime = ObjectLifetime::Application     // ... todo ...
                },
                .string = std::string(reinterpret_cast<char const*>(bytes.data()), bytes.size()),
                .type = TextType::Plain
            });
        }

        static bool decodeBytesJson(Authority<TextAsset> auth, TextAsset* asset, std::span<std::byte const> bytes) noexcept
        {
            std::string str(reinterpret_cast<char const*>(bytes.data()), bytes.size());

            return asset->text->create(auth, TextDescriptor{
                .objectInfo = ObjectDescriptor{
                    .name = asset->key,
                    .lifetime = ObjectLifetime::Application     // ... todo ...
                },
                .string = std::string(reinterpret_cast<char const*>(bytes.data()), bytes.size()),
                .type = TextType::Json
            });
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

        auto* textAsset = static_cast<TextAsset*>(asset);

        if ((textAsset == nullptr) || (textAsset->text == nullptr))
        {
            return false;
        }

        auto findExtensionType = g_textFileExtensionMap.find(textAsset->file.extension());

        if (findExtensionType == g_textFileExtensionMap.end())
        {
            // Unsupported extension type.
            return false;
        }

        switch (findExtensionType->second)
        {
        case TextFileExtension::Txt:
            return decodeBytesTxt({}, textAsset, bytes);

        case TextFileExtension::Json:
            return decodeBytesJson({}, textAsset, bytes);

        // Somehow still an unsupported extension type.
        case TextFileExtension::Unknown:
        default:
            return false;
        }
    }
}