#ifndef LITL_IMPORT_IMPORT_SERVICE_H__
#define LITL_IMPORT_IMPORT_SERVICE_H__

#include <string_view>

#include "litl-core/file.hpp"
#include "litl-import/result.hpp"
#include "litl-import/importerRegistry.hpp"
#include "litl-import/exporterRegistry.hpp"

namespace litl::import
{
    // Import general todo: all of the matching is based off of extension.
    // Likely want a format enum or something in the event multiple formats share an extension.
    // Not yet a problem, but may be one day ...

    class ImportService
    {
    public:

        ImportService();
        ~ImportService();

        ImportService(ImportService const&) = delete;
        ImportService& operator=(ImportService const&) = delete;

        /// <summary>
        /// Given a source file, attempts to convert it to an internal representation format.
        /// </summary>
        /// <param name="shouldPrepare">If true, the relevant Exporter::prepare will be run on the data to perform any necessary internal conversions. Otherwise the data will be returned untransformed.</param>
        [[nodiscard]] Result import(File const& sourceFile, ImportedData& importedData, bool shouldPrepare) noexcept;

        /// <summary>
        /// Given a source block of bytes and the the file it originates from, attempts to convert it to an internal representation format.
        /// </summary>
        /// <param name="shouldPrepare">If true, the relevant Exporter::prepare will be run on the data to perform any necessary internal conversions. Otherwise the data will be returned untransformed.</param>
        [[nodiscard]] Result import(File const& sourceFile, std::span<std::byte const> sourceBytes, ImportedData& importedData, bool shouldPrepare) noexcept;

        /// <summary>
        /// Given an external format source file, attempts to convert it to an internal representation format.
        /// The internal format is then saved to disk alongside the original file.
        /// </summary>
        [[nodiscard]] Result convert(std::string_view sourcePath) noexcept;

        /// <summary>
        /// Given an external format source file, attempts to convert it to an internal representation format.
        /// The internal format is then saved to disk in the specified directory.
        /// </summary>
        [[nodiscard]] Result convert(std::string_view sourcePath, std::string_view destFolderPath) noexcept;

    private:

        void registerProcessors() noexcept;

        ImporterRegistry m_importerRegistry{};
        ExporterRegistry m_exporterRegistry{};
    };
}

#endif