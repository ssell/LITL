#ifndef LITL_CORE_FORMATS_LITLMESH_H__
#define LITL_CORE_FORMATS_LITLMESH_H__

#include <array>
#include <bit>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/math/geometry/mesh.hpp"

static_assert(std::endian::native == std::endian::little);

namespace litl
{
    /// <summary>
    /// Binary file representation of a GeoMesh that is stored on disk as a ".litlmesh".
    /// This is effectively a non-owning view over the raw data blob.
    /// </summary>
    struct LitlMesh final
    {
        enum class ErrorCode : uint32_t
        {
            None                       = 0u,
            InvalidFileSize            = 1u,
            InvalidFileType            = 2u,
            MajorVersionMismatch       = 3u,
            MinorVersionMismatch       = 4u,
            InvalidFirstBlockOffset    = 5u,
            FileTooSmall               = 6u,
            TooManyBlocks              = 7u,
            MissingBlockDescriptor     = 8u,
            FileSizeMismatch           = 9u,
            DescriptorBlockOutOfBounds = 10u,
            BlockSizeOutOfBounds       = 11u,
            BlockSizeMismatch          = 12u
        };

        struct Ids
        {
            static constexpr std::array<char, 4> Magic{ 'L', 'M', 'S', 'H' };
            static constexpr std::array<char, 4> Vertices{ 'V', 'T', 'X', 'B' };
            static constexpr std::array<char, 4> Indices{ 'I', 'D', 'X', 'B' };
        };

        static constexpr uint32_t MaxBlocks = 8u;

        /// <summary>
        /// The first segment of the file.
        /// Contains various metadata about the file validity and contents.
        /// </summary>
        struct Header
        {
            static constexpr uint16_t MajorVersion = 1u;
            static constexpr uint16_t MinorVersion = 0u;

            /// <summary>
            /// Identifies the file as a .litlmesh
            /// </summary>
            std::array<char, 4> magic = Ids::Magic;

            /// <summary>
            /// The major version of the file.
            /// Differences in major versions indicate breaking changes.
            /// </summary>
            uint16_t versionMajor{ MajorVersion };

            /// <summary>
            /// The minor version of the file.
            /// Differences in minor versions indicate changes that do not break from previous versions.
            /// </summary>
            uint16_t versionMinor{ MinorVersion };

            /// <summary>
            /// Hash of the entire file. Used to check for corruption or truncation.
            /// </summary>
            uint64_t contentHash{ 0ull };

            /// <summary>
            /// Total size of the file in bytes.
            /// </summary>
            uint32_t totalBytes{ 0u };

            /// <summary>
            /// The number of data blocks in the file.
            /// </summary>
            uint32_t blockCount{ 0u };

            /// <summary>
            /// The offset in the file to the first data block (header + descriptors).
            /// </summary>
            uint32_t blocksOffset{ 0u };

            /// <summary>
            /// Currently unused.
            /// </summary>
            uint32_t flags{ 0u };

            /// <summary>
            /// The minimum point of the AABB that encapsulates the mesh.
            /// </summary>
            std::array<float, 3> boundsMin = { 0.0f, 0.0f, 0.0f };

            /// <summary>
            /// The maximum point of the AABB that encapsulates the mesh.
            /// </summary>
            std::array<float, 3> boundsMax = { 0.0f, 0.0f, 0.0f };

            /// <summary>
            /// Currently unused padding.
            /// </summary>
            uint32_t reserved{ 0u };

            /// <summary>
            /// Returns if the contents of the header are valid.
            /// </summary>
            /// <param name="error"></param>
            [[nodiscard]] bool validate(ErrorCode& error) const noexcept;
        };

        static_assert(sizeof(Header) == 64u);

        /// <summary>
        /// Describes the contents of a single block in the file.
        /// </summary>
        struct BlockDescriptor
        {
            /// <summary>
            /// Unique id of the block. Must match one of the predefined block ids (see Ids) or it will be skipped over during deserialization.
            /// </summary>
            std::array<char, 4> blockId{};

            /// <summary>
            /// The offset from the start of the file to where the block (not the descriptor) lives.
            /// </summary>
            uint32_t offset{ 0u };

            /// <summary>
            /// The size of the block.
            /// </summary>
            uint32_t blockBytes{ 0u };

            /// <summary>
            /// The size of an individual element in the block.
            /// </summary>
            uint32_t elementBytes{ 0u };

            /// <summary>
            /// The number of elements in the block.
            /// </summary>
            uint32_t elementCount{ 0u };

            /// <summary>
            /// Optional block-specific flags.
            /// </summary>
            uint32_t flags{ 0u };
        };

        static_assert(sizeof(BlockDescriptor) == 24u);

        struct Block
        {
            /// <summary>
            /// Unique id of the block. Must match one of the predefined block ids (see Ids) or it will be skipped over during deserialization.
            /// </summary>
            std::array<char, 4> blockId{};

            /// <summary>
            /// The size of an individual element in the block.
            /// </summary>
            uint32_t elementBytes{ 0u };

            /// <summary>
            /// The number of elements in the block.
            /// </summary>
            uint32_t elementCount{ 0u };

            /// <summary>
            /// Non-owning view of the data blob of the block.
            /// </summary>
            std::span<std::byte const> bytes;

            /// <summary>
            /// Converts the data blob into a non-owning span of individual elements.
            /// </summary>
            /// <typeparam name="T"></typeparam>
            /// <returns></returns>
            template<typename T> requires std::is_trivially_copyable_v<T>
            [[nodiscard]] std::optional<std::span<T const>> as() const noexcept
            {
                LITL_ASSERT_MSG(sizeof(T) == elementBytes, "Size mismatch between expected block element size and provided type.", std::nullopt);
                LITL_ASSERT_MSG(bytes.size() == static_cast<uint64_t>(elementBytes * elementCount), "Block data not large enough to hold required number of elements of type.", std::nullopt);

                return std::span<T const>(
                    reinterpret_cast<T const*>(bytes.data()),
                    bytes.size() / sizeof(T));
            }
        };

        static_assert(sizeof(Block) == 32u);

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
        [[nodiscard]] bool deserialize(GeoMesh& mesh, ErrorCode& error) const noexcept;

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