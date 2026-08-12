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
        using BlockIdType = std::array<char, 4>;

        enum class ErrorCode : uint32_t
        {
            None = 0u,

            /// <summary>
            /// The size of the file data does not match the declared file size.
            /// </summary>
            InvalidFileSize = 1u,

            /// <summary>
            /// The file header is missing the expected magic bytes.
            /// </summary>
            InvalidFileType = 2u,

            /// <summary>
            /// The file was created using a different major version than what is supported.
            /// </summary>
            MajorVersionMismatch = 3u,

            /// <summary>
            /// The file was created using a different (greater) minor version than what is supported.
            /// </summary>
            MinorVersionMismatch = 4u,

            /// <summary>
            /// The hash of the file blocks does not match the recorded hash in the file.
            /// </summary>
            ContentHashMismatch = 5u,

            /// <summary>
            /// The offset of the first data block is invalid. It is either too small and intersects
            /// with the header, or the offset is not a multiple of 16 as expected.
            /// </summary>
            InvalidFirstBlockOffset = 6u,

            /// <summary>
            /// The declared number of data blocks exceeds the maximum number of supported blocks.
            /// </summary>
            TooManyBlocks = 7u,

            /// <summary>
            /// There are fewer block descriptors than the declared number of data blocks.
            /// </summary>
            MissingBlockDescriptor = 8u,

            /// <summary>
            /// The start or end of the descriptor block is out-of-bounds of the file.
            /// </summary>
            DescriptorBlockOutOfBounds = 9u,

            /// <summary>
            /// The start or end of the data block is out-of-bounds of the file.
            /// </summary>
            BlockSizeOutOfBounds = 10u,

            /// <summary>
            /// The declared size of the block in the descriptor does not match the actual size of the block.
            /// </summary>
            BlockSizeMismatch = 11u,

            /// <summary>
            /// Two or more blocks overlap their declared memory ranges.
            /// </summary>
            BlockOverlap = 12u,

            /// <summary>
            /// The calculated total bytes of the file exceed 2^32-1 (4GB).
            /// </summary>
            ContentTooLarge = 13u,

            /// <summary>
            /// The source mesh is missing either vertices, indices, face index counts, or a combination thereof.
            /// </summary>
            SourceMeshEmpty = 14u,

            /// <summary>
            /// The size of the provided type does not match the expected element size.
            /// </summary>
            ElementSizeMismatch = 15u,

            /// <summary>
            /// The total size of the element block does not match the expected element block size.
            /// </summary>
            ElementBlockSizeMismatch = 16u,

            /// <summary>
            /// The alignment of the provided type is not a multiple of 16, which is required.
            /// </summary>
            ElementOffsetAlignmentMismatch = 17u,

            /// <summary>
            /// One or more elements have a size of zero.
            /// </summary>
            ElementSizeOfZero = 18u,

            /// <summary>
            /// The size of the block of elements is not evenly divisible by the size of an individual element.
            /// </summary>
            ElementBlockIsNotWhole = 19u,

            /// <summary>
            /// One (or more) of the blocks have an invalid offset which is not evenly divisible by 16.
            /// </summary>
            InvalidBlockOffset = 20u
        };

        struct Ids
        {
            /// <summary>
            /// Magic bytes for the .litlmesh file - LMSH
            /// </summary>
            static constexpr BlockIdType Magic{ 'L', 'M', 'S', 'H' };

            /// <summary>
            /// Id for a block of vertex data - VRTX
            /// </summary>
            static constexpr BlockIdType Vertices{ 'V', 'R', 'T', 'X' };

            /// <summary>
            /// Id for a block of index data - INDX
            /// </summary>
            static constexpr BlockIdType Indices{ 'I', 'N', 'D', 'X' };

            /// <summary>
            /// Id for a block of face index count data - FACE
            /// </summary>
            static constexpr BlockIdType Faces{ 'F', 'A', 'C', 'E' };
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
            BlockIdType magic{};

            /// <summary>
            /// The major version of the file.
            /// Differences in major versions indicate breaking changes.
            /// </summary>
            uint16_t versionMajor{ 0u };

            /// <summary>
            /// The minor version of the file.
            /// Differences in minor versions indicate changes that do not break from previous versions.
            /// </summary>
            uint16_t versionMinor{ 0u };

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
            uint64_t reserved{ 0u };

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
            BlockIdType blockId{};

            /// <summary>
            /// The offset from the start of the file to where the block (not the descriptor) lives.
            /// </summary>
            uint32_t blockOffset{ 0u };

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

            /// <summary>
            /// Currently unused padding.
            /// </summary>
            uint64_t reserved{ 0ull };
        };

        static_assert(sizeof(BlockDescriptor) == 32u);

        struct Block
        {
            /// <summary>
            /// Unique id of the block. Must match one of the predefined block ids (see Ids) or it will be skipped over during deserialization.
            /// </summary>
            BlockIdType blockId{};

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
            [[nodiscard]] std::optional<std::span<T const>> as(ErrorCode& error) const noexcept
            {
                error = ErrorCode::None;

                if (sizeof(T) != static_cast<size_t>(elementBytes))
                {
                    // Size mismatch between expected block element size and provided type.
                    error = ErrorCode::ElementSizeMismatch;
                    return std::nullopt;
                }

                if (bytes.size() != (static_cast<size_t>(elementBytes) * elementCount))
                {
                    // Block data not large enough to hold required number of elements of type.
                    error = ErrorCode::ElementBlockSizeMismatch;
                    return std::nullopt;
                }

                if (reinterpret_cast<uintptr_t>(bytes.data()) % alignof(T) != 0)
                {
                    error = ErrorCode::ElementOffsetAlignmentMismatch;
                    return std::nullopt;
                }

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
        [[nodiscard]] static bool serialize(GeoMesh const& mesh, std::vector<std::byte>& data, ErrorCode& error) noexcept;

        /// <summary>
        /// Populates the provided GeoMesh with the data that this view is over.
        /// </summary>
        /// <returns>False if deserialization failed. See the supplied error code for more information.</returns>
        [[nodiscard]] bool deserialize(GeoMesh& mesh, ErrorCode& error) const noexcept;

        /// <summary>
        /// Retrieves the block with the corresponding id.
        /// </summary>
        /// <returns>std::nullopt if no such block was found.</returns>
        [[nodiscard]] std::optional<Block> find(BlockIdType id) const noexcept;

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
        /// Non-owning view of the entire file binary blob (including the header, etc.).
        /// </summary>
        std::span<std::byte const> data;
    };

    static_assert(std::is_trivially_copyable_v<LitlMesh>);
    static_assert(std::is_trivially_copyable_v<LitlMesh::BlockDescriptor>);
    static_assert(std::is_trivially_copyable_v<LitlMesh::Block>);
}

#endif