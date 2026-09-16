#ifndef LITL_ENGINE_ASSETS_SOURCE_H__
#define LITL_ENGINE_ASSETS_SOURCE_H__

#include <cstdint>
#include <string>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-core/stringId.hpp"
#include "litl-engine/assets/assetRegistration.hpp"

namespace litl
{
    /// <summary>
    /// Level of indirection above where the bytes of an asset come from: loose file, bundle, etc.
    /// </summary>
    class AssetSource
    {
    public:

        virtual ~AssetSource() = default;

        /// <summary>
        /// Called once during setup. The source reports everything it can provide.
        /// </summary>
        virtual void enumerate(std::vector<AssetRegistration>& registrations) noexcept = 0;

        /// <summary>
        /// Called on a worker thread during asset load to fetch the bytes for the asset from the file, bundle, etc.
        /// </summary>
        [[nodiscard]] virtual bool read(AssetLocator locator, std::vector<std::byte>& bytes) noexcept = 0;

        /// <summary>
        /// Used to describe an asset location for error logging. May be a file path, bundle path, etc.
        /// </summary>
        [[nodiscard]] virtual std::string describe(AssetLocator locator) const noexcept = 0;
    };
}

#endif