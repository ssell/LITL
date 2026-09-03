#ifndef LITL_IMPORT_MATERIAL_LITLMAT_H__
#define LITL_IMPORT_MATERIAL_LITLMAT_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    /// <summary>
    /// Imports our internal .litlmat material file format.
    /// 
    /// This is a TOML-based format and not to be confused with our intermediate
    /// material binary file format that uses the .litlbmat extension.
    /// </summary>
    class LitlMatImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "LITL Material";
        static constexpr std::array SupportedExtensions = { std::string_view{".litlmat"} };

        LitlMatImporter();
        ~LitlMatImporter();

        LitlMatImporter(LitlMatImporter const&) = delete;
        LitlMatImporter& operator=(LitlMatImporter const&) = delete;

        [[nodiscard]] Result import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept override;
    };
}

#endif