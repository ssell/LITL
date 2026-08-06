#ifndef LITL_IMPORT_MESH_GLB_H__
#define LITL_IMPORT_MESH_GLB_H__

#include "litl-import/mesh/import/meshImporter.hpp"

namespace litl::import
{
    class GlbImporter final : public MeshImporter
    {
    public:

        GlbImporter();
        ~GlbImporter();

        GlbImporter(GlbImporter const&) = delete;
        GlbImporter& operator=(GlbImporter const&) = delete;

        [[nodiscard]] Result import(std::span<std::byte const> bytes) noexcept override;

    private:
    };
}

#endif