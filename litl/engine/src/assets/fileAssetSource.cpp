#include "litl-engine/assets/fileAssetSource.hpp"

namespace litl
{
    namespace
    {

    }
    FileAssetSource::FileAssetSource(std::string_view rootPath) noexcept
    {
        m_root = std::string(rootPath);
    }

    void FileAssetSource::enumerate(std::vector<AssetRegistration>& registrations) noexcept
    {

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