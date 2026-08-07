#include "litl-import/mesh/export/meshExporter.hpp"

namespace litl::import
{
    MeshExporter::MeshExporter()
    {

    }

    MeshExporter::~MeshExporter()
    {

    }

    Result MeshExporter::write(File const& sourceFile, File const& destFolderPath, ImportedData const& data) noexcept
    {
        if (data.type != ImportedDataType::Mesh)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }

        if (data.importedMesh == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        return Result::Error(ErrorType::ExporterNotImplemented);
    }
}