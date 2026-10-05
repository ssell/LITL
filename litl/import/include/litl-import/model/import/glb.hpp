#ifndef LITL_IMPORT_MODEL_GLB_H__
#define LITL_IMPORT_MODEL_GLB_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    class GlbImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "GL Transmission Format Binary";
        static constexpr std::array SupportedTypes = { ImportSourceType::ModelGlb };

        GlbImporter();
        ~GlbImporter();

        GlbImporter(GlbImporter const&) = delete;
        GlbImporter& operator=(GlbImporter const&) = delete;

        [[nodiscard]] Result import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, std::span<ImportCompanion const> companions, ImportedData& importedData) noexcept override;
    };
}

#endif