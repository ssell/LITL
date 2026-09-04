#include <memory>

#include "litl-core/file.hpp"
#include "litl-core/string.hpp"
#include "litl-import/importService.hpp"

// Material
#include "litl-import/material/export/materialExporter.hpp"
#include "litl-import/material/import/litlmat.hpp"

// Mesh
#include "litl-import/mesh/export/meshExporter.hpp"
#include "litl-import/mesh/import/fbx.hpp"
#include "litl-import/mesh/import/glb.hpp"
#include "litl-import/mesh/import/gltf.hpp"
#include "litl-import/mesh/import/obj.hpp"

// Shader
#include "litl-import/shader/export/shaderExporter.hpp"
#include "litl-import/shader/import/slang.hpp"
#include "litl-import/shader/import/spirv.hpp"

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
        // Material
        m_exporterRegistry.add<MaterialExporter>();
        m_importerRegistry.add<LitlMatImporter>();

        // Mesh
        m_exporterRegistry.add<MeshExporter>();
        m_importerRegistry.add<FbxImporter>();
        m_importerRegistry.add<GlbImporter>();
        m_importerRegistry.add<GltfImporter>();
        m_importerRegistry.add<ObjImporter>();

        // Shader
        m_exporterRegistry.add<ShaderExporter>();
        m_importerRegistry.add<SlangImporter>();
        m_importerRegistry.add<SpirvImporter>();
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
            auto exporter = m_exporterRegistry.create(importedData.getType());

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
        auto exporter = m_exporterRegistry.create(importedData.getType());

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