#ifndef LITL_IMPORT_IMPORTER_H__
#define LITL_IMPORT_IMPORTER_H__

#include <array>
#include <concepts>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include "litl-import/result.hpp"
#include "litl-import/importedData.hpp"
#include "litl-import/importSourceType.hpp"
#include "litl-import/importSettings.hpp"
#include "litl-import/importDependencies.hpp"

namespace litl::import
{
    class EmbeddedImporter
    {
    public:
        virtual std::optional<uint32_t> importEmbedded(ImportSourceType sourceType, std::string_view name, std::span<std::byte const> sourceBytes, ImportSettings const& settings) const noexcept = 0;
    };

    struct ImportContext final
    {
        std::string_view location;
        ImportSettings settings;
        std::span<ImportCompanion const> companions;
        EmbeddedImporter const& embeddedImporter;
    };


    class Importer
    {
    public:

        virtual ~Importer() = default;

        [[nodiscard]] virtual Result import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept = 0;
        [[nodiscard]] virtual Result scanDependencies(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::vector<ImportDependency>& outDependencies) noexcept;
    };

    template <typename T>
    concept ValidImporter = std::derived_from<T, Importer>&& std::default_initializable<T> && requires 
    {
        { T::ImporterName }     -> std::convertible_to<std::string_view>;
        { T::SupportedTypes }   -> std::convertible_to<std::span<const ImportSourceType>>;
    };
}

#endif