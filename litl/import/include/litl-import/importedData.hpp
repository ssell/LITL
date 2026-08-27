#ifndef LITL_IMPORT_IMPORTED_DATA_H__
#define LITL_IMPORT_IMPORTED_DATA_H__

#include <memory>

#include "litl-import/mesh/import/result.hpp"

namespace litl::import
{
    enum class ImportedDataType : uint32_t
    {
        Unknown = 0u,
        Material = 1u,
        Mesh = 2u
    };

    struct ImportedData final
    {
        ImportedData() {}
        ~ImportedData() {}
        ImportedData(ImportedData const&) = delete;
        ImportedData& operator=(ImportedData const&) = delete;

        ImportedDataType type{ ImportedDataType::Unknown };

        union {
            std::unique_ptr<MeshImportResult> mesh = nullptr;
        };
    };
}

#endif