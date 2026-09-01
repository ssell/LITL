#ifndef LITL_IMPORT_MATERIAL_LITLMATB_H__
#define LITL_IMPORT_MATERIAL_LITLMATB_H__

#include "litl-core/formats/binaryBlockFile.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl::import
{
    /// <summary>
    /// The binary file format for MaterialIntermediateData.
    /// Composed of the following blocks:
    /// 
    ///     * Shaders (SHDR): Each element specifies the stage, shader resource, and entry point.
    ///     * Properties (PROP): Name, type, and value of all input properties.
    ///     * Settings (SETT): Material name, raster settings, and material hints.
    /// 
    /// To convert a MaterialIntermediateData to a binary blob simply use the serialize method.
    /// To deserialize a binary blob to a MaterialIntermediateData you must first call parse and then deserialize.
    /// </summary>
    struct LitlMatBinary final : public BinaryBlockFile
    {
        static constexpr BinaryBlockFileFormatIdentity Identity{
            .magic = { 'L', 'M', 'A', 'T' },
            .versionMajor = 1,
            .versionMinor = 0
        };

        struct BlockIds
        {
            static constexpr BinaryBlockIdType Shaders{ 'S', 'H', 'D', 'R' };
            static constexpr BinaryBlockIdType Properties{ 'P', 'R', 'O', 'P' };
            static constexpr BinaryBlockIdType Settings{ 'S', 'E', 'T', 'T' };
        };

        /// <summary>
        /// Given a MaterialIntermediateData object, populates the byte blob that will be recorded to disk as a .litlmatb file.
        /// </summary>
        [[nodiscard]] static bool serialize(MaterialIntermediateData const& material, std::vector<std::byte>& data, ErrorCode& error) noexcept;

        /// <summary>
        /// Populates the provided MaterialIntermediateData object from the BinaryBlockFile::data byte array from a .litlmatb that was processed via BinaryBlockFile::parse.
        /// </summary>
        [[nodiscard]] bool deserialize(MaterialIntermediateData& material, ErrorCode& error) const noexcept;
    };
}

#endif