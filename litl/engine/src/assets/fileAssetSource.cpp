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
            AssetType assetType{ AssetType::Unknown };
            AssetFormat format{ AssetFormat::Unknown };
            import::ImportSourceType sourceType{ import::ImportSourceType::Unknown };
        };

        static const StringIdMap<AssetTypeMapping> g_assetTypeMap = {
            // Material
            { ".litlmat"_sid, { AssetMappingPriority::High, AssetType::Material, AssetFormat::Internal, import::ImportSourceType::MaterialLitl } },
            { ".litlbmat"_sid, { AssetMappingPriority::Medium, AssetType::Material, AssetFormat::Internal, import::ImportSourceType::MaterialLitlBinary } },

            // Mesh
            { ".litlbmsh"_sid, { AssetMappingPriority::High, AssetType::Mesh, AssetFormat::Internal, import::ImportSourceType::MeshLitlBinary } },

            // Model
            { ".litlmdl"_sid, { AssetMappingPriority::High, AssetType::Model, AssetFormat::Internal, import::ImportSourceType::ModelLitl } },
            { ".glb"_sid, { AssetMappingPriority::Medium, AssetType::Mesh, AssetFormat::External, import::ImportSourceType::ModelGlb } },
            { ".fbx"_sid, { AssetMappingPriority::Low, AssetType::Mesh, AssetFormat::External, import::ImportSourceType::ModelFbx } },
            { ".gltf"_sid, { AssetMappingPriority::Low, AssetType::Mesh, AssetFormat::External, import::ImportSourceType::ModelGltf } },
            { ".obj"_sid, { AssetMappingPriority::Low, AssetType::Model, AssetFormat::External, import::ImportSourceType::ModelObj } },

            // Shader Module
            { ".litlbshd"_sid, { AssetMappingPriority::High, AssetType::Shader, AssetFormat::Internal, import::ImportSourceType::ShaderLitlBinary } },
            { ".spv"_sid, { AssetMappingPriority::Medium, AssetType::Shader, AssetFormat::External, import::ImportSourceType::ShaderSpirv } },
            { ".slang"_sid, { AssetMappingPriority::Low, AssetType::Shader, AssetFormat::External, import::ImportSourceType::ShaderSlang } },

            // Text
            { ".txt"_sid, { AssetMappingPriority::Medium, AssetType::Text, AssetFormat::External, import::ImportSourceType::TextPlain } },
            { ".json"_sid, { AssetMappingPriority::Medium, AssetType::Text, AssetFormat::External, import::ImportSourceType::TextJson } },

            // Texture
            { ".litlbtex"_sid, { AssetMappingPriority::High, AssetType::Texture, AssetFormat::Internal, import::ImportSourceType::TextureLitlBinary } },
            { ".hdr"_sid, { AssetMappingPriority::Medium, AssetType::Texture, AssetFormat::External, import::ImportSourceType::TextureHdr } },
            { ".png"_sid, { AssetMappingPriority::Medium, AssetType::Texture, AssetFormat::External, import::ImportSourceType::TexturePng } },
            { ".tga"_sid, { AssetMappingPriority::Medium, AssetType::Texture, AssetFormat::External, import::ImportSourceType::TextureTga } },
        };

        [[nodiscard]] std::string relativePathFromRoot(std::filesystem::path const& path, std::string_view root) noexcept
        {
            return path.lexically_relative(root).generic_string();;
        }

        [[nodiscard]] std::string assetKeyFromRoot(std::filesystem::path const& path, std::string_view root) noexcept
        {
            return toLowercase(path.lexically_relative(root).replace_extension().generic_string());
        }
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
                    const auto relativePath = relativePathFromRoot(path, m_root);
                    const auto assetKey = assetKeyFromRoot(path, m_root);
                    const auto hashedKey = StringId(assetKey);
                    const auto locator = AssetLocator{ .entryIndex = static_cast<uint32_t>(m_files.size()) };                   // .sourceIndex is populated by the AssetManager

                    m_files.push_back(file);

                    registrations.push_back(AssetRegistration{
                        .key = assetKey,
                        .location = describe(locator),
                        .hashedKey = StringId(assetKey),
                        .assetType = assetFileType->second.assetType,
                        .format = assetFileType->second.format,
                        .sourceType = assetFileType->second.sourceType,
                        .priority = static_cast<uint32_t>(assetFileType->second.priority),
                        .locator = locator
                    });
                }
            }
        }
    }

    bool FileAssetSource::read(AssetLocator locator, std::vector<std::byte>& bytes) noexcept
    {
        if (locator.entryIndex >= m_files.size())
        {
            return false;
        }

        auto& file = m_files[locator.entryIndex];
        
        if (!file.readAllBytes(bytes))
        {
            return false;
        }

        return true;
    }

    std::string FileAssetSource::describe(AssetLocator locator) const noexcept
    {
        if (locator.entryIndex >= m_files.size())
        {
            return "FileAssetSource::UnknownPath";
        }

        return relativePathFromRoot(m_files[locator.entryIndex].getFileSystempath(), m_root);
    }
}