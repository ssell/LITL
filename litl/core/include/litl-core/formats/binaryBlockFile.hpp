#ifndef LITL_CORE_FORMATS_BINARY_BLOCK_FILE_H__
#define LITL_CORE_FORMATS_BINARY_BLOCK_FILE_H__

#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "litl-core/stringId.hpp"

static_assert(std::endian::native == std::endian::little);

namespace litl
{
    /// <summary>
    /// A four character block/file identifier. For example: 'LMSH', 'VRTX', etc.
    /// These are used instead of an enum type as they are clearly visible when viewing hex dumps of the binary file.
    /// </summary>
    using BinaryBlockIdType = std::array<char, 4>;

    /// <summary>
    /// Each implementation of BinaryBlockFile must provide a FileFormatIdentity.
    /// See the BinaryBlockFileFormat concept.
    /// </summary>
    struct BinaryBlockFileFormatIdentity
    {
        BinaryBlockIdType magic{};
        uint16_t versionMajor{ 0u };
        uint16_t versionMinor{ 0u };
    };

    struct BinaryBlockFile;

    /// <summary>
    /// Enforces that any implementation of BinaryBlockFile provides an Identity const that specifies the magic bytes and current version of the file.
    /// </summary>
    template<typename T>
    concept BinaryBlockFileFormat = std::derived_from<T, BinaryBlockFile> && requires { 
            { T::Identity } -> std::convertible_to<BinaryBlockFileFormatIdentity const&>;
    };

    /// <summary>
    /// Shared base structure for all internal binary files that use a block layout structure.
    /// 
    /// The layout of a BinaryBlockFile is:
    /// 
    ///     [Header]
    ///     [BlockDescriptors 0..N]
    ///     [Blocks 0..N]
    /// 
    /// Note that "Blocks" on disk is not the same as the internal Block struct.
    /// The "Block" on disk is the binary blob composed of N number of elements, while "Block" struct is an internal read-only view of that blob.
    /// </summary>
    struct BinaryBlockFile
    {
        struct DefaultBlocks
        {
            static constexpr uint32_t DefaultBlocksCount = 1u;

            static constexpr BinaryBlockIdType Strings{ 'S', 'T', 'R', 'S' };
            static constexpr uint32_t StringsBlockIndex = 0u;
            static constexpr uint32_t FirstCustomBlockIndex = 1u;
        };

        enum class ErrorCode : uint32_t
        {
            None = 0u,

            // -----------------------------------------------------------------------------
            // Generic Error Codes
            // -----------------------------------------------------------------------------

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
            /// The offset of the first block descriptor is invalid.
            /// </summary>
            InvalidFirstDescriptorOffset = 6u,

            /// <summary>
            /// The offset of the first data block is invalid. It is either too small and intersects
            /// with the header, or the offset is not a multiple of 16 as expected.
            /// </summary>
            InvalidFirstBlockOffset = 7u,

            /// <summary>
            /// The declared number of data blocks exceeds the maximum number of supported blocks.
            /// </summary>
            TooManyBlocks = 8u,

            /// <summary>
            /// This file, which should have blocks, has none. Where are they?
            /// </summary>
            WhereTheBlocksAt = 9u,

            /// <summary>
            /// There are fewer block descriptors than the declared number of data blocks.
            /// </summary>
            MissingBlockDescriptor = 10u,

            /// <summary>
            /// The start or end of the descriptor block is out-of-bounds of the file.
            /// </summary>
            DescriptorBlockOutOfBounds = 11u,

            /// <summary>
            /// The start or end of the data block is out-of-bounds of the file.
            /// </summary>
            BlockSizeOutOfBounds = 12u,

            /// <summary>
            /// The declared size of the block in the descriptor does not match the actual size of the block.
            /// </summary>
            BlockSizeMismatch = 13u,

            /// <summary>
            /// Two or more blocks overlap their declared memory ranges.
            /// </summary>
            BlockOverlap = 14u,

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
            InvalidBlockOffset = 20u,

            MissingStringsBlock = 21u,

            StringRefEmpty = 22u,

            StringRefOutOfBounds = 23u,

            EmptyBlockMissingRequiredPadding = 24u,

            // -----------------------------------------------------------------------------
            // LitlMesh Error Codes
            // -----------------------------------------------------------------------------

            /// <summary>
            /// The source mesh is missing either vertices, indices, face index counts, or a combination thereof.
            /// </summary>
            SourceMeshEmpty = 1000u,

            /// <summary>
            /// Input file is missing a vertex data block.
            /// </summary>
            MissingVertexBlock = 1001u,

            /// <summary>
            /// Input file is missing an index data block.
            /// </summary>
            MissingIndexBlock = 1002u,

            /// <summary>
            /// Input file is missing a face data block.
            /// </summary>
            MissingFaceBlock = 1003u,

            /// <summary>
            /// Input file is missing a bounds data block.
            /// </summary>
            MissingBoundsBlock = 1004u,

            /// <summary>
            /// Deserialization found an index that exceeded the vertex count.
            /// </summary>
            InvalidIndexFound = 1005u,

            /// <summary>
            /// Deserialization found that the total sum of all face index counts does not match the index count.
            /// </summary>
            InvalidFaceSum = 1006u,

            /// <summary>
            /// Deserialization found a face that was declared to have zero indices.
            /// </summary>
            ZeroFaceFound = 1007u,

            /// <summary>
            /// Mesh bounds block should have exactly 6 elements: [min.x, min.y, min.z, max.x, max.y, max.z].
            /// </summary>
            InvalidBoundsValues = 1008u,

            // -----------------------------------------------------------------------------
            // LitlMaterial Error Codes
            // -----------------------------------------------------------------------------

            MissingShadersBlock = 2000u,
            MissingPropertiesBlock = 2001u,
            MissingSettingsBlock = 2002u,
            MaterialPropertyDeserializationFailed = 2003u,
            UnknownMaterialPropertyType = 2004u,
            MissingMaterialNameSetting = 2005u,
                
            // -----------------------------------------------------------------------------
            // LitlShader Error Codes
            // -----------------------------------------------------------------------------

            MissingEntryPointsBlock = 3000u,
            MissingResourceBindingsBlock = 3001u,
            MissingPushConstantsBlock = 3002u,
            MissingPushConstantReferencePropertiesBlock = 3003u,
            MissingVertexFragmentInputOutputBlock = 3004u,
            MissingResourcePropertiesBlock = 3005u,
            MissingSpecializationsConstantBlock = 3006u,
            MissingSpirvBlock = 3007u,
            ShaderBinarySubspanOutOfBounds = 3008u,
            ShaderBinarySubspanInvalidInput = 3009u
        };

        static constexpr uint32_t MaxBlocks = 16u;

        /// <summary>
        /// The first segment of the file.
        /// Contains various metadata about the file validity and contents.
        /// </summary>
        struct Header
        {
            /// <summary>
            /// Identifies the file as a .litlmesh
            /// </summary>
            BinaryBlockIdType magic{};

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
            uint64_t totalBytes{ 0ull };

            /// <summary>
            /// The offset in the file of the first block descriptor.
            /// This typically immediately follows the header, but this field allows for the header to grow in the future and
            /// for something to be inserted between the header and the block descriptors without being a breaking change.
            /// </summary>
            uint64_t descriptorsOffset{ 0ull };

            /// <summary>
            /// The offset in the file to the first data block (header + descriptors).
            /// </summary>
            uint64_t blocksOffset{ 0ull };

            /// <summary>
            /// The number of data blocks in the file.
            /// </summary>
            uint32_t blockCount{ 0u };

            /// <summary>
            /// Currently unused.
            /// </summary>
            uint32_t flags{ 0u };

            /// <summary>
            /// Padding to ensure the Header size is equal to a multiple of 32.
            /// </summary>
            std::array<uint64_t, 2> padding{};

            /// <summary>
            /// Returns if the contents of the header are valid.
            /// </summary>
            [[nodiscard]] bool validate(ErrorCode& error, BinaryBlockFileFormatIdentity const& identity) const noexcept;
        };

        static_assert(sizeof(Header) == 64);
        static_assert(alignof(Header) == 8);
        static_assert(std::is_standard_layout_v<Header>);

        /// <summary>
        /// Describes the contents of a single block in the file.
        /// </summary>
        struct BlockDescriptor
        {
            /// <summary>
            /// Unique id of the block. Must match one of the predefined block ids (see Ids) or it will be skipped over during deserialization.
            /// </summary>
            BinaryBlockIdType blockId{};

            /// <summary>
            /// Optional block-specific flags.
            /// </summary>
            uint32_t flags{ 0u };

            /// <summary>
            /// The offset from the start of the file to where the block (not the descriptor) lives.
            /// </summary>
            uint64_t blockOffset{ 0ull };

            /// <summary>
            /// The size of the block.
            /// </summary>
            uint64_t blockBytes{ 0ull };

            /// <summary>
            /// The size of an individual element in the block.
            /// </summary>
            uint64_t elementBytes{ 0ull };

            /// <summary>
            /// The number of elements in the block.
            /// </summary>
            uint64_t elementCount{ 0ull };

            /// <summary>
            /// Padding to ensure the BlockDescriptor size is equal to a multiple of 32.
            /// </summary>
            std::array<uint64_t, 3> padding{};
        };

        static_assert(sizeof(BlockDescriptor) == 64);
        static_assert(alignof(BlockDescriptor) == 8);
        static_assert(std::is_standard_layout_v<BlockDescriptor>);
        static_assert(std::is_trivially_copyable_v<BlockDescriptor>);

        /// <summary>
        /// View and metadata for a contiguous block of memory within the file.
        /// The block is composed of a number of all of the same elements.
        /// The Block struct itself is not part of the on-disk file format and is a read-only view.
        /// </summary>
        struct Block
        {
            /// <summary>
            /// Unique id of the block. Must match one of the predefined block ids (see Ids) or it will be skipped over during deserialization.
            /// </summary>
            BinaryBlockIdType blockId{};

            /// <summary>
            /// The size of an individual element in the block.
            /// </summary>
            uint64_t elementBytes{ 0u };

            /// <summary>
            /// The number of elements in the block.
            /// </summary>
            uint64_t elementCount{ 0u };

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
                if (error != ErrorCode::None)   // error carryover from likely chain-calling 
                {
                    return std::nullopt;
                }

                if (sizeof(T) != static_cast<size_t>(elementBytes))
                {
                    // Size mismatch between expected block element size and provided type.
                    error = ErrorCode::ElementSizeMismatch;
                    return std::nullopt;
                }

                if ((elementCount > 0u) && (bytes.size() != (static_cast<size_t>(elementBytes) * elementCount)))
                {
                    // Block data not large enough to hold required number of elements of type.
                    error = ErrorCode::ElementBlockSizeMismatch;
                    return std::nullopt;
                }
                else if ((elementCount == 0u) && (bytes.size() != 16u))
                {
                    // Even with 0 elements, still expect the block to have 16 bytes of padding to avoid overlap with neighboring blocks.
                    error = ErrorCode::EmptyBlockMissingRequiredPadding;
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

        static_assert(std::is_trivially_copyable_v<Block>);

        /// <summary>
        /// Defines the expected data layout for a block.
        /// </summary>
        struct BlockDataDescriptor
        {
            BlockDescriptor* descriptor;
            BinaryBlockIdType id;
            uint64_t elementSize;
            std::span<std::byte const> data;
        };

        struct StringRef
        {
            /// <summary>
            /// Byte offset into the 'STRS' block.
            /// </summary>
            uint64_t offset{ 0u };

            /// <summary>
            /// Length of the string, in bytes.
            /// </summary>
            uint32_t length{ 0u };

            /// <summary>
            /// Unused padding.
            /// </summary>
            uint32_t padding{ 0u };
        };

        static_assert(sizeof(StringRef) == 16);
        static_assert(std::is_trivially_copyable_v<StringRef>);

        struct StringMap
        {
            std::vector<StringRef> stringRefs;
            std::vector<std::string> strings;
            std::vector<std::byte> stringBlob;
            StringIdMap<uint32_t> map;
            uint64_t runningOffset{ 0ull };
        };

        /// <summary>
        /// Given a binary blob, attempts to parse it into the provided file format implementation.
        /// This will validate and populate the header and descriptors which is needed for deserialiation.
        /// </summary>
        template<typename TFormat>
        [[nodiscard]] static bool parse(std::span<std::byte const> data, TFormat& file, ErrorCode& error) noexcept requires BinaryBlockFileFormat<TFormat>
        {
            return parseImpl(data, TFormat::Identity, static_cast<BinaryBlockFile&>(file), error);
        }

        /// <summary>
        /// Calculates the hash value of all file bytes following the header.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static uint64_t calculateContentHash(std::span<std::byte const> data, Header const& header) noexcept;

        /// <summary>
        /// Given a block, serializes it.
        /// </summary>
        /// <returns></returns>
        static void serializeBlock(BlockDataDescriptor& data, uint64_t& runningBlockOffset) noexcept;

        /// <summary>
        /// 
        /// </summary>
        [[nodiscard]] static StringRef serializeString(std::string_view string, StringMap& stringOffsetMap) noexcept;

        /// <summary>
        /// 
        /// </summary>
        [[nodiscard]] static std::string_view deserializeString(std::span<char const> strings, StringRef ref, ErrorCode& error) noexcept;

        /// <summary>
        /// 
        /// </summary>
        [[nodiscard]] bool addDataBlockDescriptor(std::vector<BinaryBlockFile::BlockDataDescriptor>& blockDataTable, uint32_t descriptorIndex, BinaryBlockIdType const& id, size_t elementSize, std::span<std::byte const> data, BinaryBlockFile::ErrorCode& error) noexcept;

        /// <summary>
        /// 
        /// </summary>
        void addDefaultBlockDescriptors(std::vector<BlockDataDescriptor>& blockDataTable) noexcept;

        /// <summary>
        /// 
        /// </summary>
        void serializeDefaultBlocks(std::vector<BlockDataDescriptor>& blockDataTable, StringMap& stringOffsetMap) noexcept;

        /// <summary>
        /// Retrieves the block with the corresponding id.
        /// </summary>
        /// <returns>std::nullopt if no such block was found.</returns>
        [[nodiscard]] std::optional<Block> find(BinaryBlockIdType id) const noexcept;


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

    protected:

        static void serializeDataBuffer(BinaryBlockFile& blockFile, std::vector<BlockDataDescriptor>& blockDataTable, std::vector<std::byte>& data) noexcept;

    private:

        /// <summary>
        /// Populates a BinaryBlockFile file view from a supplied blob of data.
        /// Performs various validations on the header and descriptor blocks.
        /// </summary>
        /// <returns>False if the supplied blob is invalid. See the supplied error code for more information.</returns>
        [[nodiscard]] static bool parseImpl(std::span<std::byte const> data, BinaryBlockFileFormatIdentity const& identity, BinaryBlockFile& file, ErrorCode& error) noexcept;
    };

    static_assert(std::is_trivially_copyable_v<BinaryBlockFile>);
}

#endif