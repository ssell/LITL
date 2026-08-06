#ifndef LITL_IMPORT_MESH_OBJ_H__
#define LITL_IMPORT_MESH_OBJ_H__

#include "litl-import/mesh/import/meshImporter.hpp"

namespace litl::import
{
    class ObjImporter final : public MeshImporter
    {
    public:

        ObjImporter();
        ~ObjImporter();

        ObjImporter(ObjImporter const&) = delete;
        ObjImporter& operator=(ObjImporter const&) = delete;

        [[nodiscard]] Result import(std::span<std::byte const> bytes) noexcept override;
    };
}

#endif