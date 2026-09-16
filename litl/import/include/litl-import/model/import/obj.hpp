#ifndef LITL_IMPORT_MODEL_OBJ_H__
#define LITL_IMPORT_MODEL_OBJ_H__

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
        static constexpr std::array SupportedTypes = { ImportSourceType::ModelObj };

        ObjImporter();
        ~ObjImporter();

        ObjImporter(ObjImporter const&) = delete;
        ObjImporter& operator=(ObjImporter const&) = delete;

        [[nodiscard]] Result import(std::string_view location, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept override;
    };
}

#endif