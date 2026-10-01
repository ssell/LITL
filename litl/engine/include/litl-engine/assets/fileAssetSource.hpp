#ifndef LITL_ENGINE_ASSETS_FILE_SOURCE_H__
#define LITL_ENGINE_ASSETS_FILE_SOURCE_H__

#include <deque>
#include <mutex>

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

        /// <summary>
        /// Given a file asset reference, composes the expected file path that the reference resides at.
        /// </summary>
        [[nodiscard]] bool resolve(AssetLocator base, std::string_view reference, AssetLocator& outLocator) noexcept override;

    private:

        [[nodiscard]] bool isFileIndexSafe(uint32_t index) const noexcept;

        std::string m_root;
        std::deque<File> m_files;
        mutable std::mutex m_filesMutex{};          // resolve is typically called on a worker thread and can add additional files to m_file. So this is needed to guard .size() and the resolve's .push_back
    };
}

#endif