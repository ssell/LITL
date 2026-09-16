#include <filesystem>

#include "litl-core/directory.hpp"
#include "litl-core/string.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/assets/fileAssetSource.hpp"

namespace litl
{
    namespace
    {
        struct AssetTypeMapping
        {
            AssetMappingPriority priority{ AssetMappingPriority::Low };
            AssetType type{ AssetType::Unknown };
            AssetFormat format{ AssetFormat::Unknown };
        };

        static const StringIdMap<AssetTypeMapping> g_assetTypeMap = {
            // Material
            { ".litlmat"_sid, { AssetMappingPriority::High, AssetType::Material, AssetFormat::Internal } },
            { ".litlbmat"_sid, { AssetMappingPriority::Medium, AssetType::Material, AssetFormat::Internal } },

            // Mesh
            { ".litlbmsh"_sid, { AssetMappingPriority::High, AssetType::Mesh, AssetFormat::Internal } },
            { ".glb"_sid, { AssetMappingPriority::Medium, AssetType::Mesh, AssetFormat::External } },
            { ".fbx"_sid, { AssetMappingPriority::Low, AssetType::Mesh, AssetFormat::External } },
            { ".gltf"_sid, { AssetMappingPriority::Low, AssetType::Mesh, AssetFormat::External } },

            // Model
            { ".litlmdl"_sid, { AssetMappingPriority::High, AssetType::Model, AssetFormat::Internal } },
            { ".obj"_sid, { AssetMappingPriority::Low, AssetType::Model, AssetFormat::External } },

            // Shader Module
            { ".litlbshd"_sid, { AssetMappingPriority::High, AssetType::Shader, AssetFormat::Internal } },
            { ".spv"_sid, { AssetMappingPriority::Medium, AssetType::Shader, AssetFormat::External } },
            { ".slang"_sid, { AssetMappingPriority::Low, AssetType::Shader, AssetFormat::External } },

            // Text
            { ".txt"_sid, { AssetMappingPriority::Medium, AssetType::Text, AssetFormat::External } },
            { ".json"_sid, { AssetMappingPriority::Medium, AssetType::Text, AssetFormat::External } }
        };
    }

    FileAssetSource::FileAssetSource(std::string_view rootPath) noexcept
    {
        m_root = std::string(rootPath);
    }

    void FileAssetSource::enumerate(std::vector<AssetRegistration>& registrations) noexcept
    {
        if (!Directory::exists(m_root))
        {
            logError("Root directory '", m_root, "' provided to FileAssetSource does not exists.");
            return;
        }

        const uint32_t maxFileCount = Directory::fileCount(m_root, true);

        if (maxFileCount == 0u)
        {
            return;
        }

        m_files.reserve(maxFileCount);

        for (auto const& fileEntry : std::filesystem::recursive_directory_iterator(m_root))
        {
            if (fileEntry.is_regular_file())
            {
                auto path = fileEntry.path();
                auto file = File(fileEntry);
                auto absPath = file.absolutePath();
                auto assetFileType = g_assetTypeMap.find(StringId(toLowercase(file.extension())));

                if (assetFileType != g_assetTypeMap.end())
                {
                    const auto relativePath = path.lexically_relative(m_root).generic_string();
                    const auto assetKey = toLowercase(path.lexically_relative(m_root).replace_extension().generic_string());
                    const auto hashedKey = StringId(assetKey);

                    registrations.push_back(AssetRegistration{
                        .key = assetKey,
                        .hashedKey = StringId(assetKey),
                        .type = assetFileType->second.type,
                        .format = assetFileType->second.format,
                        .priority = static_cast<uint32_t>(assetFileType->second.priority),
                        .locator = AssetLocator { .entryIndex = static_cast<uint32_t>(m_files.size()) }     // .sourceIndex is populated by the AssetManager
                    });

                    m_files.push_back(file);
                }
            }
        }
    }

    bool FileAssetSource::read(AssetLocator locator, std::vector<std::byte>& bytes) noexcept
    {
        return false;
    }

    std::string FileAssetSource::describe(AssetLocator locator) const noexcept
    {
        return "";
    }
}