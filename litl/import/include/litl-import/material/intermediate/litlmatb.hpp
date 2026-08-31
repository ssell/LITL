#ifndef LITL_IMPORT_MATERIAL_LITLMATB_H__
#define LITL_IMPORT_MATERIAL_LITLMATB_H__

#include "litl-core/formats/binaryBlockFile.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl::import
{

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

        [[nodiscard]] static bool serialize(MaterialIntermediateData const& material, std::vector<std::byte>& data, ErrorCode& error) noexcept;
        [[nodiscard]] bool deserialize(MaterialIntermediateData& material, ErrorCode& error) const noexcept;
    };
}

#endif