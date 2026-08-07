#ifndef LITL_IMPORT_IMPORTED_DATA_H__
#define LITL_IMPORT_IMPORTED_DATA_H__

#include <memory>

namespace litl::import
{
    class Mesh;

    enum class ImportedDataType : uint32_t
    {
        Unknown = 0u,
        Mesh = 1u
    };

    struct ImportedData final
    {
        ImportedData() {}
        ~ImportedData() {}
        ImportedData(ImportedData const&) = delete;
        ImportedData& operator=(ImportedData const&) = delete;

        ImportedDataType type{ ImportedDataType::Unknown };

        union {
            std::unique_ptr<Mesh> importedMesh;
        };
    };
}

#endif