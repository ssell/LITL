#ifndef LITL_IMPORT_SHADER_LITLSHADER_H__
#define LITL_IMPORT_SHADER_LITLSHADER_H__

#include "litl-core/formats/binaryBlockFile.hpp"
#include "litl-import/shader/intermediate/shaderIntermediateData.hpp"

namespace litl::import
{
    /// <summary>
    /// The binary file format for ShaderIntermediateData.
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

        struct BlockIds
        {
            static constexpr BinaryBlockIdType EntryPoints{ 'E', 'N', 'T', 'R' };
            static constexpr BinaryBlockIdType ResourceBindings{ 'R', 'E', 'S', 'B' };
            static constexpr BinaryBlockIdType PushConstants{ 'P', 'U', 'S', 'H' };
            static constexpr BinaryBlockIdType PushConstantReferenceProperties{ 'P', 'U', 'R', 'P' };
            static constexpr BinaryBlockIdType VertexFragmentInputOutput{ 'V', 'F', 'I', 'O' };
            static constexpr BinaryBlockIdType ResourceProperties{ 'R', 'E', 'S', 'P' };
            static constexpr BinaryBlockIdType SpecializationConstants{ 'S', 'P', 'E', 'C' };
            static constexpr BinaryBlockIdType Spirv{ 'S', 'P', 'R', 'V' };
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