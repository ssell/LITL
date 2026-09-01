#ifndef LITL_IMPORT_IMPORTED_DATA_H__
#define LITL_IMPORT_IMPORTED_DATA_H__

#include <memory>

#include "litl-import/material/import/result.hpp"
#include "litl-import/mesh/import/result.hpp"
#include "litl-import/shader/import/result.hpp"

namespace litl::import
{
    enum class ImportedDataType : uint32_t
    {
        Unknown = 0u,
        Material = 1u,
        Mesh = 2u,
        Shader = 3u,
        Texture = 4u
    };

    struct ImportedData final
    {
        ImportedData() {}
        ~ImportedData() {}
        ImportedData(ImportedData const&) = delete;
        ImportedData& operator=(ImportedData const&) = delete;

        ImportedDataType type{ ImportedDataType::Unknown };

        union {
            std::unique_ptr<MaterialImportResult> material = nullptr;
            std::unique_ptr<MeshImportResult> mesh;
            std::unique_ptr<ShaderImportResult> shader;
        };
    };
}

#endif