#include <memory>

#include "litl-core/file.hpp"
#include "litl-core/string.hpp"
#include "litl-import/importService.hpp"

#include "litl-import/mesh/export/meshExporter.hpp"
#include "litl-import/mesh/import/fbx.hpp"
#include "litl-import/mesh/import/glb.hpp"
#include "litl-import/mesh/import/gltf.hpp"
#include "litl-import/mesh/import/obj.hpp"

namespace litl::import
{
    ImportService::ImportService()
    {
        registerProcessors();
    }

    ImportService::~ImportService()
    {
        // ... required as this is an injectable service and can live in a std::shared_ptr ...
    }

    void ImportService::registerProcessors() noexcept
    {
        m_importerRegistry.add<FbxImporter>();
        m_importerRegistry.add<GlbImporter>();
        m_importerRegistry.add<GltfImporter>();
        m_importerRegistry.add<ObjImporter>();

        m_exporterRegistry.add<MeshExporter>();
    }

    Result ImportService::import(File const& sourceFile, ImportedData& importedData) noexcept
    {
        if (!sourceFile.exists())
        {
            return Result::Error(ErrorType::SourceFileDoesNotExist);
        }

        if (sourceFile.size() == 0u)
        {
            return Result::Error(ErrorType::EmptySourceFile);
        }

        auto importer = m_importerRegistry.create(sourceFile);
        auto fileBytes = sourceFile.readAllBytes();

        if (importer == nullptr)
        {
            return Result::Error(ErrorType::NoImporterForSourceExtension);
        }

        if (!fileBytes.has_value())
        {
            return Result::Error(ErrorType::FailedToReadSourceFile);
        }

        Result const result = importer->import(sourceFile, *fileBytes, importedData);

        return result;
    }

    Result ImportService::convert(std::string_view sourcePath) noexcept
    {
        return convert(sourcePath, File(sourcePath).parentFolderPath());
    }

    Result ImportService::convert(std::string_view sourcePath, std::string_view destFolderPath) noexcept
    {
        // Import
        File const sourceFile = sourcePath;
        ImportedData importedData{};
        Result const importResult = import(sourcePath, importedData);

        if (!importResult.success)
        {
            return importResult;
        }

        // Export
        auto exporter = m_exporterRegistry.create(importedData.type);

        if (exporter == nullptr)
        {
            return Result::Error(ErrorType::NoExporterForImportedDataType);
        }

        auto exportResult = exporter->write(sourceFile, destFolderPath, importedData);

        if (!exportResult.success)
        {
            return exportResult;
        }

        return Result::Success();
    }
}