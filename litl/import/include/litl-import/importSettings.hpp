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

        [[nodiscard]] static constexpr ImportSettings Default() noexcept { return {}; }
    };

    static_assert(std::is_trivially_copyable_v<ImportSettings>);
}

#endif