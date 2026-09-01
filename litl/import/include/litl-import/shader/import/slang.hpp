#ifndef LITL_IMPORT_SHADER_SLANG_H__
#define LITL_IMPORT_SHADER_SLANG_H__

#include "litl-import/importer.hpp"

namespace litl::import
{
    class SlangImporter final : public Importer
    {
    public:

        static constexpr std::string_view ImporterName = "Slang Shader";
        static constexpr std::array SupportedExtensions = { std::string_view{".slang"} };

        SlangImporter();
        ~SlangImporter();

        SlangImporter(SlangImporter const&) = delete;
        SlangImporter& operator=(SlangImporter const&) = delete;

        [[nodiscard]] Result import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept override;
    };
}

#endif