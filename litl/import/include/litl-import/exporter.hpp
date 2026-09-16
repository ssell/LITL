#ifndef LITL_IMPORT_EXPORTER_H__
#define LITL_IMPORT_EXPORTER_H__

#include <array>
#include <concepts>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "litl-core/file.hpp"
#include "litl-import/result.hpp"
#include "litl-import/importedData.hpp"

namespace litl::import
{
    class Exporter
    {
    public:

        virtual ~Exporter() = default;

        /// <summary>
        /// Called prior to write or after import if direct importing and not converting to file on disk.
        /// This ensures the data is correct for the destination internal format. For example with meshes,
        /// this is the step that ensures the mesh is triangulated, optimized, has normals, etc.
        /// </summary>
        virtual Result prepare(ImportedData& data, uint32_t dataIndex) noexcept = 0;

        /// <summary>
        /// Writes the object bytes to the given vector.
        /// </summary>
        virtual Result write(std::vector<std::byte>& serialized, ImportedData const& data, uint32_t dataIndex) noexcept = 0;
    };

    template <typename T>
    concept ValidExporter = std::derived_from<T, Exporter>&& std::default_initializable<T>&& requires
    {
        { T::ExporterName }                 -> std::convertible_to<std::string_view>;
        { T::OperatesOnImportedDataType }   -> std::convertible_to<ImportedDataType>;
        { T::ExportedExtension }            -> std::convertible_to<std::string_view>;
    };
}

#endif