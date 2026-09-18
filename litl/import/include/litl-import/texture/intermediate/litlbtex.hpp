#ifndef LITL_IMPORT_TEXTURE_LITLBTEX_H__
#define LITL_IMPORT_TEXTURE_LITLBTEX_H__

#include "litl-core/formats/binaryBlockFile.hpp"
#include "litl-import/texture/intermediate/textureIntermediateData.hpp"

namespace litl::import
{
    struct LitlTextureBinary final : public BinaryBlockFile
    {
        static constexpr BinaryBlockFileFormatIdentity Identity{
            .magic = { 'L', 'T', 'E', 'X' },
            .versionMajor = 1u,
            .versionMinor = 0u
        };

        struct BlockIds
        {
            // ... todo ...
        };

        /// <summary>
        /// Given a TextureIntermediateData object, populates the byte blob that will be recorded to disk as a .litlbtex file.
        /// </summary>
        [[nodiscard]] static bool serialize(TextureIntermediateData const& texture, std::vector<std::byte>& data, ErrorCode& error) noexcept;

        /// <summary>
        /// Populates the provided TextureIntermediateData object from the BinaryBlockFile::data byte array from a .litlbtex that was processed via BinaryBlockFile::parse.
        /// </summary>
        [[nodiscard]] bool deserialize(TextureIntermediateData const& texture, ErrorCode& error) const noexcept;
    };
}

#endif