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

    Result ImportService::import(File const& sourceFile, ImportedData& importedData, bool shouldPrepare) noexcept
    {
        if (!sourceFile.exists())
        {
            return Result::Error(ErrorType::SourceFileDoesNotExist);
        }

        if (sourceFile.size() == 0u)
        {
            return Result::Error(ErrorType::EmptySourceFile);
        }

        auto fileBytes = sourceFile.readAllBytes();

        if (!fileBytes.has_value())
        {
            return Result::Error(ErrorType::FailedToReadSourceFile);
        }

        return import(sourceFile, *fileBytes, importedData, shouldPrepare);
    }

    Result ImportService::import(File const& sourceFile, std::span<std::byte const> sourceBytes, ImportedData& importedData, bool shouldPrepare) noexcept
    {
        auto importer = m_importerRegistry.create(sourceFile.extension());

        if (importer == nullptr)
        {
            return Result::Error(ErrorType::NoImporterForSourceExtension);
        }

        Result const importResult = importer->import(sourceFile, sourceBytes, importedData);

        if (!importResult.success)
        {
            return importResult;
        }

        if (shouldPrepare)
        {
            auto exporter = m_exporterRegistry.create(importedData.type);

            if (exporter == nullptr)
            {
                return Result::Error(ErrorType::NoExporterForImportedDataType);
            }

            return exporter->prepare(importedData);
        }
        else
        {
            return importResult;
        }
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
        Result const importResult = import(sourcePath, importedData, false);

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

        Result const prepareResult = exporter->prepare(importedData);

        if (!prepareResult.success)
        {
            return prepareResult;
        }

        Result const exportResult = exporter->write(sourceFile, destFolderPath, importedData);

        if (!exportResult.success)
        {
            return exportResult;
        }

        return Result::Success();
    }
}