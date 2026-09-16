#ifndef LITL_ENGINE_ASSETS_FILE_SOURCE_H__
#define LITL_ENGINE_ASSETS_FILE_SOURCE_H__

#include "litl-core/file.hpp"
#include "litl-engine/assets/assetSource.hpp"

namespace litl
{
    class FileAssetSource final : public AssetSource
    {
    public:

        /// <summary>
        /// Sets the root directory in which asset files will be scanned for.
        /// </summary>
        explicit FileAssetSource(std::string_view rootPath) noexcept;

        /// <summary>
        /// Traverses the asset source (root directory) and outputs all valid asset files.
        /// </summary>
        void enumerate(std::vector<AssetRegistration>& registrations) noexcept override;

        /// <summary>
        /// Given an asset located by the FileAssetSource, extracts the bytes from it.
        /// </summary>
        [[nodiscard]] bool read(AssetLocator locator, std::vector<std::byte>& bytes) noexcept override;

        /// <summary>
        /// Given an asset located by the FileAssetSource, returns a loggable identifier for it.
        /// </summary>
        [[nodiscard]] std::string describe(AssetLocator locator) const noexcept override;

    private:

        std::string m_root;
        std::vector<File> m_files;
    };
}

#endif