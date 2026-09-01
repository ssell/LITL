#include "litl-import/shader/import/spirv.hpp"

namespace litl::import
{
    SpirvImporter::SpirvImporter()
    {

    }

    SpirvImporter::~SpirvImporter()
    {

    }

    Result SpirvImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        // Super easy: just pass through to the ShaderIntermediate data.
        importedData.type = ImportedDataType::Shader;
        importedData.shader = std::make_unique<ShaderImportResult>();
        importedData.shader->intermediateShader = std::make_unique<ShaderIntermediateData>();
        importedData.shader->intermediateShader->setSpirvBytes(sourceBytes);

        return Result::Success();
    }
}