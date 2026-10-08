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

        [[nodiscard]] Result import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept override;
    };
}

#endif