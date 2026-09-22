#ifndef LITL_IMPORT_MATERIAL_SETTINGS_H__
#define LITL_IMPORT_MATERIAL_SETTINGS_H__

#include <type_traits>

namespace litl::import
{
    struct MaterialImportSettings
    {

    };

    static_assert(std::is_trivially_copyable_v<MaterialImportSettings>);
}

#endif