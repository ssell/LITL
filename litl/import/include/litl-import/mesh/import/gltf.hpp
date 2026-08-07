#ifndef LITL_IMPORT_MESH_GLTF_H__
#define LITL_IMPORT_MESH_GLTF_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    /// <summary>
    /// The glTF format is a JSON-based format that defines the scene structure,
    /// node hierarchy, materials, animations paths, etc. with the actual mesh
    /// data stored in a binary (.bin) sidecar file.
    /// </summary>
    class GltfImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "glTF";
        static constexpr std::array SupportedExtensions = { std::string_view{".glTF"} };

        GltfImporter();
        ~GltfImporter();

        GltfImporter(GltfImporter const&) = delete;
        GltfImporter& operator=(GltfImporter const) = delete;

        [[nodiscard]] Result import(File const& file, std::span<std::byte const> bytes) noexcept override;

    private:
    };
}

#endif