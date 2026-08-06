#include <memory>

#include "litl-core/file.hpp"
#include "litl-core/string.hpp"
#include "litl-core/stringId.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/importer.hpp"
#include "litl-import/exporter.hpp"

#include "litl-import/mesh/export/meshExporter.hpp"
#include "litl-import/mesh/import/fbx.hpp"
#include "litl-import/mesh/import/glb.hpp"
#include "litl-import/mesh/import/gltf.hpp"
#include "litl-import/mesh/import/obj.hpp"

namespace litl::import
{
    ImportService::ImportService()
    {
        registerImporters();
    }

    ImportService::~ImportService()
    {
        // ... required as this is an injectable service and can live in a std::shared_ptr ...
    }

    void ImportService::registerImporters() noexcept
    {
        m_importerRegistry.add<FbxImporter>();
        m_importerRegistry.add<GlbImporter>();
        m_importerRegistry.add<GltfImporter>();
        m_importerRegistry.add<ObjImporter>();
    }

    Result ImportService::convert(std::string_view sourcePath) noexcept
    {
        return convert(sourcePath, File(sourcePath).parentFolderPath());
    }

    Result ImportService::convert(std::string_view sourcePath, std::string_view destFolderPath) noexcept
    {
        // .. todo add timestamp/performance tracking ...

        File sourceFile = sourcePath;

        if (!sourceFile.exists())
        {
            return Result::Error(ErrorType::SourceFileDoesNotExist);
        }

        if (sourceFile.size() == 0u)
        {
            return Result::Error(ErrorType::EmptySourceFile);
        }

        auto importer = m_importerRegistry.create(sourceFile);
        auto exporter = nullptr;        // ... todo ...
        auto fileBytes = sourceFile.readAllBytes();

        if (importer == nullptr)
        {
            return Result::Error(ErrorType::NoImporterForSourceExtension);
        }

        if (exporter == nullptr)
        {
            return Result::Error(ErrorType::NoExporterForSourceExtension);
        }

        if (!fileBytes.has_value())
        {
            return Result::Error(ErrorType::FailedToReadSourceFile);
        }

        auto importResult = importer->import(sourceFile, *fileBytes);

        if (!importResult.success)
        {
            return importResult;
        }

        // todo
        /*
        auto exportResult = exporter->write(importer, sourceFile, destFolderPath);

        if (!exportResult.success)
        {
            return exportResult;
        }
        */

        return Result::Success();
    }
}