#ifndef LITL_IMPORT_SETTINGS_H__
#define LITL_IMPORT_SETTINGS_H__

#include <type_traits>

#include "litl-import/material/import/materialImportSettings.hpp"
#include "litl-import/mesh/import/meshImportSettings.hpp"
#include "litl-import/model/import/modelImportSettings.hpp"
#include "litl-import/shader/import/shaderImportSettings.hpp"
#include "litl-import/texture/import/textureImportSettings.hpp"

namespace litl::import
{
    struct ImportSettings
    {
        MaterialImportSettings material{};
        MeshImportSettings mesh{};
        ModelImportSettings model{};
        ShaderImportSettings shader{};
        TextureImportSettings texture{};

        /// <summary>
        /// If true, processing of the asset will continue if one or more of its external dependencies fail to load.
        /// Otherwise the import action will fail if any dependencies fail. For example, if a model file should continue to be
        /// imported if one of its composite textures or materials fail then this should be true.
        /// </summary>
        bool continueOnDependencyFailure{ true };

        [[nodiscard]] static constexpr ImportSettings Default() noexcept { return {}; }
    };

    static_assert(std::is_trivially_copyable_v<ImportSettings>);
}

#endif