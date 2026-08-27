#include <expected>
#include <glaze/toml.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

#include "litl-import/material/import/litlmat.hpp"

namespace litl::import
{
    struct ShaderResourceStruct
    {
        std::string resource;
        std::string entry;
    };

    struct RasterSettingsStruct
    {
        std::string cullMode;
        std::string frontFace;
    };

    struct HintsStruct
    {
        bool frequentUpdates = false;
    };

    struct ExpectedMaterialStructure
    {
        std::string name;
        std::unordered_map<std::string, ShaderResourceStruct> shaders;
        RasterSettingsStruct raster;
        HintsStruct hints;
    };

    LitlMatImporter::LitlMatImporter()
    {

    }

    LitlMatImporter::~LitlMatImporter()
    {

    }

    Result LitlMatImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        const auto sourceBytesString = std::string_view{ reinterpret_cast<char const*>(sourceBytes.data()), sourceBytes.size() };

        ExpectedMaterialStructure inputMaterial{};
        const auto readTomlResult = glz::read_toml(inputMaterial, sourceBytesString);

        if (readTomlResult)
        {

        }
        else
        {
            return Result::Error(ErrorType::ImporterFailed, readTomlResult.custom_error_message);
        }

        return Result::Success();
    }
}