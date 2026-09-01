#ifndef LITL_IMPORT_SHADER_LITLSHADER_H__
#define LITL_IMPORT_SHADER_LITLSHADER_H__

#include "litl-core/formats/binaryBlockFile.hpp"
#include "litl-import/shader/intermediate/shaderIntermediateData.hpp"

namespace litl::import
{
    /// <summary>
    /// The binary file format for ShaderIntermediateData.
    /// Composed of the following blocks:
    /// 
    ///     * SPIR-V (SPRV): The original SPIR-V bytecode.
    ///     * Reflected Bindings (RFLC): Merged reflection and names.
    ///     * Material Property Layout (MPRP): Stride and property counts.
    ///     * Resource Property Record (RPRP): Offset / size / scalarSize / componentCount
    ///     * Metadata (META): Source .slang /.spirv path, slangc version, flags
    /// 
    /// To convert a ShaderIntermediateData to a binary blob simply use the serialize method.
    /// To deserialize a binary blob to a ShaderIntermediateData you must first call parse and then deserialize.
    /// </summary>
    struct LitlShader final : public BinaryBlockFile
    {
        static constexpr BinaryBlockFileFormatIdentity Identity{
            .magic = { 'L', 'S', 'H', 'D' },
            .versionMajor = 1,
            .versionMinor = 0
        };

        struct Blockids
        {
            static constexpr BinaryBlockIdType Spirv{ 'S', 'P', 'R', 'V' };
            static constexpr BinaryBlockIdType Reflected{ 'R', 'F', 'L', 'C' };
            static constexpr BinaryBlockIdType MaterialProperty{ 'M', 'P', 'R', 'P' };
            static constexpr BinaryBlockIdType ResourceProperty{ 'R', 'P', 'R', 'P' };
            static constexpr BinaryBlockIdType Metadata{ 'M', 'E', 'T', 'A' };
        };

        /// <summary>
        /// Given a ShaderIntermediateData object, populates the byte blob that will be recorded to disk as a .litlshader file.
        /// </summary>
        [[nodiscard]] static bool serialize(ShaderIntermediateData const& shader, std::vector<std::byte>& data, ErrorCode& error) noexcept;

        /// <summary>
        /// Populates the provided ShaderIntermediateData object from the BinaryBlockFile::data byte array from a .litlshader that was processed via BinaryBlockFile::parse.
        /// </summary>
        [[nodiscard]] bool deserialize(ShaderIntermediateData& shader, ErrorCode& error) const noexcept;
    };
}

#endif