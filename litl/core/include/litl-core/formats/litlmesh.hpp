#ifndef LITL_CORE_FORMATS_LITLMESH_H__
#define LITL_CORE_FORMATS_LITLMESH_H__

#include <array>
#include <bit>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "litl-core/math/geometry/mesh.hpp"

static_assert(std::endian::native == std::endian::little);

namespace litl
{
    /// <summary>
    /// Binary file representation of a GeoMesh that is stored on disk as a ".litlmesh".
    /// This is effectively a non-owning view over the raw data blob.
    /// </summary>
    class LitlMesh final
    {
    public:

        enum class ErrorCode : uint32_t
        {
            None = 0u,
            InvalidFileSize = 1u,
            InvalidFileType = 2u,
            MajorVersionMismatch = 3u,
            MinorVersionMismatch = 4u,
            HeaderStructureMismatch = 5u,
            FileTooSmall = 6u,
            MissingBlocks = 7u,
            TooManyBlocks = 8u,
            FileSizeMismatch = 9u,
            DescriptorBlockOutOfBounds = 10u,
            BlockSizeOutOfBounds = 11u,
            BlockSizeMismatch = 12u
        };

    private:

        static constexpr uint32_t MaxBlocks = 8u;

        struct Ids
        {
            static constexpr std::array<char, 4> Magic{ 'L', 'M', 'S', 'H' };
            static constexpr std::array<char, 4> Vertices{ 'V', 'T', 'X', 'B' };
            static constexpr std::array<char, 4> Indices{ 'I', 'D', 'X', 'B' };
        };

        struct Header
        {
            static constexpr uint16_t MajorVersion = 1u;
            static constexpr uint16_t MinorVersion = 0u;

            std::array<char, 4> magic = Ids::Magic;
            uint16_t versionMajor{ MajorVersion };
            uint16_t versionMinor{ MinorVersion };
            uint32_t headerBytes{ 0u };
            uint32_t totalBytes{ 0u };
            uint32_t blockCount{ 0u };
            uint32_t flags{ 0u };
            std::array<float, 3> boundsMin = { 0.0f, 0.0f, 0.0f };
            std::array<float, 3> boundsMax = { 0.0f, 0.0f, 0.0f };

            [[nodiscard]] bool validate(ErrorCode& error) noexcept;
        };

        static_assert(sizeof(Header) == 48u);

        struct BlockDescriptor
        {
            std::array<char, 4> blockId{};
            uint32_t offset{ 0u };
            uint32_t blockBytes{ 0u };
            uint32_t elementBytes{ 0u };
            uint32_t elementCount{ 0u };
            uint32_t flags{ 0u };
        };

        static_assert(sizeof(BlockDescriptor) == 24u);

        struct Block
        {
            std::array<char, 4> blockId{};
            uint32_t elementBytes{ 0u };
            uint32_t elementCount{ 0u };
            std::span<std::byte const> bytes;

            template<typename T> requires std::is_trivially_copyable_v<T>
            [[nodiscard]] std::span<T const> as() const noexcept;
        };

        static_assert(sizeof(Block) == 32u);

    public:

        /// <summary>
        /// Populates a LitlMesh file view from a supplied blob of data.
        /// Performs various validations on the header and descriptor blocks.
        /// </summary>
        /// <returns>False if the supplied blob is invalid. See the supplied error code for more information.</returns>
        [[nodiscard]] static bool parse(std::span<std::byte const> data, LitlMesh& file, ErrorCode& error) noexcept;

        /// <summary>
        /// Given a GeoMesh, converts its contents into a binary blob represented by the LitlMesh layout.
        /// </summary>
        /// <returns>False if serialization failed. See the supplied error code for more information.</returns>
        [[nodiscard]] bool serialize(GeoMesh const& mesh, std::vector<std::byte>& data, ErrorCode& error) noexcept;

        /// <summary>
        /// Populates the provided GeoMesh with the data that this view is over.
        /// </summary>
        /// <returns>False if deserialization failed. See the supplied error code for more information.</returns>
        [[nodiscard]] bool deserialize(GeoMesh& mesh, ErrorCode& error) noexcept;

        /// <summary>
        /// Retrieves the block with the corresponding id.
        /// </summary>
        /// <returns>std::nullopt if no such block was found.</returns>
        [[nodiscard]] std::optional<Block> find(std::array<char, 4> id) const noexcept;

        /// <summary>
        /// The file header with the magic number, version, bounds, and expected sizes.
        /// </summary>
        Header header{};

        /// <summary>
        /// Series of descriptors for each block in the file.
        /// They describe the starting offset of the block and the size and count of its elements.
        /// </summary>
        std::array<BlockDescriptor, MaxBlocks> descriptors{};

        /// <summary>
        /// The number of actual descriptor blocks in the file.
        /// </summary>
        uint32_t descriptorCount{ 0u };

        /// <summary>
        /// Non-owning view of the entire file binary blob (including the header, etc.).
        /// </summary>
        std::span<std::byte const> data;
    };

    static_assert(std::is_trivially_copyable_v<LitlMesh>);
}

#endif