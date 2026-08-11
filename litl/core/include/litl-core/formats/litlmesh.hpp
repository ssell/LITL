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
    /// </summary>
    class LitlMesh final
    {
    private:

        static constexpr uint32_t MaxBlocks = 8u;

        struct Header
        {
            static constexpr uint16_t MajorVersion = 1u;
            static constexpr uint16_t MinorVersion = 0u;

            std::array<char, 4> magic{ 'L', 'M', 'S', 'H' };
            uint16_t versionMajor{ MajorVersion };
            uint16_t versionMinor{ MinorVersion };
            uint32_t headerBytes{ 0u };
            uint32_t totalBytes{ 0u };
            uint32_t blockCount{ 0u };
            uint32_t flags{ 0u };
            uint64_t contentHash{ 0ull };
            std::array<float, 3> boundsMin = { 0.0f, 0.0f, 0.0f };
            std::array<float, 3> boundsMax = { 0.0f, 0.0f, 0.0f };
            uint64_t reserve{ 0ull };
        };

        static_assert(sizeof(Header) == 64u);

        struct BlockIds
        {
            static constexpr std::array<char, 4> Vertices{ 'V', 'T', 'X', 'B' };
            static constexpr std::array<char, 4> Indices{ 'I', 'D', 'X', 'B' };
        };

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

        [[nodiscard]] static bool parse(std::span<std::byte const> data, LitlMesh& file) noexcept;
        [[nodiscard]] bool serialize(GeoMesh const& mesh, std::vector<std::byte>& data) noexcept;
        [[nodiscard]] bool deserialize(std::span<std::byte const> data, GeoMesh& mesh) noexcept;
        [[nodiscard]] std::optional<Block> find(std::array<char, 4> id) const noexcept;

        Header header{};
        std::array<BlockDescriptor, MaxBlocks> descriptors{};
        uint32_t descriptorCount{ 0u };
        std::span<std::byte const> data;
    };
}

#endif