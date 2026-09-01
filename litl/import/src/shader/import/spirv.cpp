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
        // Convert the std::byte to uint32_t as SPIR-V works on 4-byte words.
        importedData.type = ImportedDataType::Shader;
        importedData.shader = std::make_unique<ShaderImportResult>();
        importedData.shader->intermediateShader = std::make_unique<ShaderIntermediateData>();
        importedData.shader->intermediateShader->setSpirvWords(std::span<uint32_t const>{ reinterpret_cast<const uint32_t*>(sourceBytes.data()), sourceBytes.size_bytes() / sizeof(uint32_t) });

        return Result::Success();
    }
}