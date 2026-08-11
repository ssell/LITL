#include <format>

#include "litl-core/directory.hpp"
#include "litl-import/mesh/export/meshExporter.hpp"

namespace litl::import
{
    MeshExporter::MeshExporter()
    {

    }

    MeshExporter::~MeshExporter()
    {

    }

    Result MeshExporter::write(File const& sourceFile, std::string_view destFolderPath, ImportedData const& data) noexcept
    {
        if (data.type != ImportedDataType::Mesh)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }

        if (data.mesh == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (!Directory::ensureExists(destFolderPath))
        {
            return Result::Error(ErrorType::ExportDestinationDoesNotExist);
        }

        std::string_view destFilePath = std::format("{}/{}{}", destFolderPath, sourceFile.name(), MeshExporter::ExportedExtension);



        return Result::Success();
    }
}