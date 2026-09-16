#include <format>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "litl-core/directory.hpp"
#include "litl-core/file.hpp"
#include "litl-core/string.hpp"
#include "litl-core/stringId.hpp"
#include "litl-import/importService.hpp"

// Material
#include "litl-import/material/export/materialExporter.hpp"
#include "litl-import/material/import/litlmat.hpp"

// Mesh
#include "litl-import/mesh/export/meshExporter.hpp"
#include "litl-import/mesh/import/fbx.hpp"
#include "litl-import/mesh/import/glb.hpp"
#include "litl-import/mesh/import/gltf.hpp"

// Model
#include "litl-import/model/export/modelExporter.hpp"
#include "litl-import/model/import/obj.hpp"

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

        // Model
        m_exporterRegistry.add<ModelExporter>();
        m_importerRegistry.add<ObjImporter>();

        // Shader
        m_exporterRegistry.add<ShaderExporter>();
        m_importerRegistry.add<SlangImporter>();
        m_importerRegistry.add<SpirvImporter>();
    }

    Result ImportService::import(File const& sourceFile, std::string_view location, ImportedData& importedData, bool shouldPrepare) noexcept
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

        return import(location, *fileBytes, importedData, shouldPrepare);
    }

    Result ImportService::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportedData& importedData, bool shouldPrepare) noexcept
    {
        auto importer = m_importerRegistry.create(sourceFile.extension());

        if (importer == nullptr)
        {
            return Result::Error(ErrorType::NoImporterForSourceExtension);
        }

        Result const importResult = importer->import(location, sourceBytes, importedData);

        if (!importResult.success)
        {
            return importResult;
        }

        sanitizeAndDeduplicateImportedItemNames(importedData);
        
        for (uint32_t i = 0u; i < static_cast<uint32_t>(importedData.items.size()); ++i)
        {
            if (shouldPrepare)
            {
                auto exporter = m_exporterRegistry.create(importedData.items[i].getType());

                if (exporter == nullptr)
                {
                    importedData.result.allSuccess = false;
                    importedData.result.firstNonSuccessResult =  Result::Error(ErrorType::NoExporterForImportedDataType);
                    importedData.result.errorIndex = i;

                    return Result::Error(ErrorType::ProcessFailedSeeIndividualItemResult);
                }

                auto result = exporter->prepare(importedData, i);

                if (!result.success)
                {
                    importedData.result.allSuccess = false;
                    importedData.result.firstNonSuccessResult = result;
                    importedData.result.errorIndex = i;

                    return Result::Error(ErrorType::ProcessFailedSeeIndividualItemResult);
                }
            }
        }

        return Result::Success();
    }

    Result ImportService::convert(std::string_view sourcePath) noexcept
    {
        return convert(sourcePath, File(sourcePath).parentFolderPath());
    }

    Result ImportService::convert(std::string_view sourcePath, std::string_view destFolderPath) noexcept
    {
        // Import from one external file
        File const sourceFile = sourcePath;
        ImportedData importedData{};
        Result const importResult = import(sourcePath, sourcePath, importedData, false);

        if (!importResult.success || importedData.items.empty())
        {
            return importResult;
        }


        const std::string rootItemFolderPath =  toLowercase(destFolderPath);
        const std::string childItemFolderPath = toLowercase(Directory::appendFolder(rootItemFolderPath, sourceFile.name()));

        if (importedData.items.size() > 1)
        {
            if (!Directory::ensureExists(childItemFolderPath))
            {
                return Result::Error(ErrorType::FailedToCreateChildItemSubDirectory);
            }
        }

        for (uint32_t i = 0u; i < static_cast<uint32_t>(importedData.items.size()); ++i)
        {
            auto& importedDataItem = importedData.items[i];
            auto exporter = m_exporterRegistry.create(importedDataItem.getType());

            if (exporter == nullptr)
            {
                importedData.result.allSuccess = false;
                importedData.result.firstNonSuccessResult = Result::Error(ErrorType::NoExporterForImportedDataType);
                importedData.result.errorIndex = i;

                return Result::Error(ErrorType::ProcessFailedSeeIndividualItemResult);
            }

            Result const prepareResult = exporter->prepare(importedData, i);

            if (!prepareResult.success)
            {
                return prepareResult;
            }

            Result exportResult = Result::Success();

            if (i == 0u)
            {
                exportResult = exporter->write(sourceFile, rootItemFolderPath, importedData, i, std::nullopt);
            }
            else
            {
                exportResult = exporter->write(sourceFile, childItemFolderPath, importedData, i, importedDataItem.getName());
            }

            if (!exportResult.success)
            {
                return exportResult;
            }
        }

        return Result::Success();
    }

    namespace
    {
        /// <summary>
        /// Ensures all names are safe for use as file names and asset keys.
        /// Any unsafe names are either modified to remove the offending characters or set to empty.
        /// </summary>
        void sanitizeItemNames(ImportedData& importedData) noexcept
        {
            if (importedData.items.empty())
            {
                return;
            }

            for (auto& importedDataItem : importedData.items)
            {
                const auto lowercaseName = toLowercase(importedDataItem.getName());
                auto sanitizedName = File::SanitizeFilename(lowercaseName);

                if (File::IsReservedFileName(sanitizedName))
                {
                    // Entire name is invalid. Clear it, and let the follow-up call to ensureItemsHaveNames give it a name.
                    importedDataItem.setName("");
                }
                else
                {
                    importedDataItem.setName(sanitizedName);
                }
            }
        }

        /// <summary>
        /// Assigns a name based on the item type if it does not already have a name.
        /// For example: mesh_0, mesh_1, material_0, etc.
        /// </summary>
        void ensureItemsHaveNames(ImportedData& importedData) noexcept
        {
            if (importedData.items.empty())
            {
                return;
            }

            std::unordered_map<uint32_t, uint32_t> unnamedItemsOfTypeCount;

            for (auto& importedDataItem : importedData.items)
            {
                if (!importedDataItem.getName().empty())
                {
                    continue;
                }

                const uint32_t key = static_cast<uint32_t>(importedDataItem.getType());
                const uint32_t count = unnamedItemsOfTypeCount[key];        // defaults to 0

                importedDataItem.setName(std::format("{}_{}", ImportedDataTypeNames[key], count));
                unnamedItemsOfTypeCount[key] = count + 1;
            }
        }

        /// <summary>
        /// Ensures all item names are unique.
        /// If there are any duplicated names then they are appened with the current duplication count.
        /// For example: wall (first), wall_1, wall_2, wall_3, etc.
        /// Names are also all set to lowercase as part of the deduplication process.
        /// </summary>
        void deduplicateItemNames(ImportedData& importedData) noexcept
        {
            if (importedData.items.empty())
            {
                return;
            }

            std::unordered_set<uint64_t> isNameTaken;
            isNameTaken.reserve(importedData.items.size());

            for (auto& importedDataItem : importedData.items)
            {
                const auto originalName = importedDataItem.getName();
                auto uniqueName = std::string(originalName);
                auto uniqueNameId = StringId(uniqueName);
                auto occurrenceCount = 0u;

                while (isNameTaken.contains(uniqueNameId.value))
                {
                    occurrenceCount++;
                    uniqueName = std::format("{}_{}", originalName, occurrenceCount);
                    uniqueNameId = StringId(uniqueName);
                }

                isNameTaken.insert(uniqueNameId.value);
                importedDataItem.setName(uniqueName);
            }
        }
    }

    void sanitizeAndDeduplicateImportedItemNames(ImportedData& importedData) noexcept
    {
        sanitizeItemNames(importedData);
        ensureItemsHaveNames(importedData);
        deduplicateItemNames(importedData);

        importedData.propagateNameUpdates();
    }
}
