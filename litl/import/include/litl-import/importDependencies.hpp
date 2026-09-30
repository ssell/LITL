#ifndef LITL_IMPORT_DEPENDENCIES_H__
#define LITL_IMPORT_DEPENDENCIES_H__

#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace litl::import
{
    enum class ImportDependencyKind : uint32_t
    {
        Unknown = 0u,
        MaterialLibrary = 1u,       // Companion material file. For example: .mtl for .obj.
        BinaryBuffer = 2u,          // Companion binary file. For example: .bin for .gltf.
        Image = 3u                  // Companion external textures.
    };

    struct ImportDependency
    {
        ImportDependencyKind kind{ ImportDependencyKind::Unknown };
        std::string reference;
        bool required{ true };
    };

    struct ImportCompanion
    {
        std::string_view reference;
        std::span<std::byte const> bytes;
    };
}

#endif