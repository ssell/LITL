#ifndef LITL_IMPORT_SHADER_SETTINGS_H__
#define LITL_IMPORT_SHADER_SETTINGS_H__

#include <type_traits>

namespace litl::import
{
    struct ShaderImportSettings
    {

    };

    static_assert(std::is_trivially_copyable_v<ShaderImportSettings>);
}

#endif