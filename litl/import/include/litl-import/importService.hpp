#ifndef LITL_IMPORT_IMPORT_SERVICE_H__
#define LITL_IMPORT_IMPORT_SERVICE_H__

#include <cstdint>
#include <string_view>
#include <vector>

#include "litl-core/file.hpp"
#include "litl-import/result.hpp"
#include "litl-import/importerRegistry.hpp"
#include "litl-import/exporterRegistry.hpp"

namespace litl::import
{
    struct WriteableImportResults
    {
        /// <summary>
        /// The imported and converted items in their intermediate form.
        /// </summary>
        ImportedData importedData{};

        /// <summary>
        /// The raw bytes of each of item. The index into this vector matches the index into the importedData structure.
        /// </summary>
        std::vector<std::vector<std::byte>> bytes;
    };

    class ImportService
    {
    public:

        ImportService();
        ~ImportService();

        ImportService(ImportService const&) = delete;
        ImportService& operator=(ImportService const&) = delete;

        /// <summary>
        /// Given a source block of bytes attempts to convert it to an internal representation format.
        /// The output is an ImportedData structure that has the converted data objects in their intermediate object form.
        /// 
        /// This method should be used if you only need the intermediate internal objects in memory. Use importForWriting if they need to be written to disk/embedded in a bundle/etc.
        /// </summary>
        /// <param name="shouldPrepare">If true, the relevant Exporter::prepare will be run on the data to perform any necessary internal conversions. Otherwise the data will be returned untransformed.</param>
        [[nodiscard]] Result importForMemory(ImportSourceType sourceType, std::string_view location, std::span<std::byte const> sourceBytes, ImportedData& importedData, bool shouldPrepare) noexcept;

        /// <summary>
        /// Given an external format source file, attempts to convert it to an internal representation format.
        /// This invokes (import) to retrieve the objects in their converted intermediate forms and then serializes those to their byte representation.
        /// These bytes are then read to be written to standalone files, embedded into bundles, etc.
        /// 
        /// This method should be used if you need to write the results to disk/embed in bundle/etc. If you only need the intermediate internal object in memory, then use importForMemory.
        /// </summary>
        [[nodiscard]] Result importForWriting(ImportSourceType sourceType, std::string_view location, std::span<std::byte const> sourceBytes, WriteableImportResults& writeableResults) noexcept;

    private:

        void registerProcessors() noexcept;

        ImporterRegistry m_importerRegistry{};
        ExporterRegistry m_exporterRegistry{};
    };

    /// <summary>
    /// Ensures the names of all imported data items are sanitized and unique for the scope of the provided ImportedData.
    /// After running, all names should be collision-free and safe for both use as file name and AssetManager map keys.
    /// </summary>
    void sanitizeAndDeduplicateImportedItemNames(ImportedData& importedData) noexcept;
}

#endif
