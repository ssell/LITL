#ifndef LITL_IMPORT_MODEL_SETTINGS_H__
#define LITL_IMPORT_MODEL_SETTINGS_H__

#include <type_traits>

namespace litl::import
{
    struct ModelImportSettings
    {

    };

    static_assert(std::is_trivially_copyable_v<ModelImportSettings>);
}

#endif