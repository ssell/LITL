#ifndef LITL_IMPORT_MESH_OBJ_H__
#define LITL_IMPORT_MESH_OBJ_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    /// <summary>
    /// The Wavefront OBJ file format is a simple format that stores only static mesh geometry.
    /// </summary>
    class ObjImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "Wavefront OBJ";
        static constexpr std::array SupportedExtensions = { std::string_view{".obj"} };

        ObjImporter();
        ~ObjImporter();

        ObjImporter(ObjImporter const&) = delete;
        ObjImporter& operator=(ObjImporter const&) = delete;

        [[nodiscard]] Result import(File const& file, std::span<std::byte const> bytes) noexcept override;
    };
}

#endif