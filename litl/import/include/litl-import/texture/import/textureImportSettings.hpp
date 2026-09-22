#ifndef LITL_IMPORT_TEXTURE_SETTINGS_H__
#define LITL_IMPORT_TEXTURE_SETTINGS_H__

#include <type_traits>

namespace litl::import
{
    struct TextureImportSettings
    {

    };

    static_assert(std::is_trivially_copyable_v<TextureImportSettings>);
}

#endif