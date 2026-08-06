#ifndef LITL_IMPORT_MESH_GLTF_H__
#define LITL_IMPORT_MESH_GLTF_H__

#include "litl-import/mesh/import/meshImporter.hpp"

namespace litl::import
{
    class GltfImporter final : public MeshImporter
    {
    public:

        GltfImporter();
        ~GltfImporter();

        GltfImporter(GltfImporter const&) = delete;
        GltfImporter& operator=(GltfImporter const) = delete;

        [[nodiscard]] Result import(std::span<std::byte const> bytes) noexcept override;

    private:
    };
}

#endif