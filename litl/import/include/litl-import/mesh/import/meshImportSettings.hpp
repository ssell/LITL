#ifndef LITL_IMPORT_MESH_SETTINGS_H__
#define LITL_IMPORT_MESH_SETTINGS_H__

#include <type_traits>

namespace litl::import
{
    struct MeshImportSettings
    {

    };

    static_assert(std::is_trivially_copyable_v<MeshImportSettings>);
}

#endif