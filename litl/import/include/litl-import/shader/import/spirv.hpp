#ifndef LITL_IMPORT_SHADER_SPIRV_H__
#define LITL_IMPORT_SHADER_SPIRV_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    class SpirvImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "SPIR-V Shader";
        static constexpr std::array SupportedTypes = { ImportSourceType::ShaderSpirv };

        SpirvImporter();
        ~SpirvImporter();

        SpirvImporter(SpirvImporter const&) = delete;
        SpirvImporter& operator=(SpirvImporter const&) = delete;

        [[nodiscard]] Result import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, ImportedData& importedData) noexcept override;
    };
}

#endif