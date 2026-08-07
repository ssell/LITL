#ifndef LITL_IMPORT_EXPORTER_H__
#define LITL_IMPORT_EXPORTER_H__

#include <array>
#include <concepts>
#include <cstdint>
#include <span>

#include "litl-core/file.hpp"
#include "litl-import/result.hpp"
#include "litl-import/importedData.hpp"

namespace litl::import
{
    class Exporter
    {
    public:

        virtual ~Exporter() = default;
        virtual Result write(File const& sourceFile, File const& destFolderPath, ImportedData const& data) noexcept = 0;
    };

    template <typename T>
    concept ValidExporter = std::derived_from<T, Exporter>&& std::default_initializable<T>&& requires
    {
        { T::ExporterName }                 -> std::convertible_to<std::string_view>;
        { T::OperatesOnImportedDataType }   -> std::convertible_to<ImportedDataType>;
    };
}

#endif