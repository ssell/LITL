#include <array>
#include <cstddef>
#include <cstring>
#include <vector>

#include "tests.hpp"
#include "litl-core/formats/litlmesh.hpp"

namespace litl::tests
{
    namespace
    {
        using ErrorCode = BinaryBlockFile::ErrorCode;
        using Header = BinaryBlockFile::Header;
        using BlockDescriptor = BinaryBlockFile::BlockDescriptor;

        // -------------------------------------------------------------------------------------
        // Mesh factories
        // -------------------------------------------------------------------------------------

        /// <summary>
        /// Smallest valid mesh: one triangle. 3 indices means the FACE block needs real padding.
        /// </summary>
        void makeTriangleMesh(GeoMesh& mesh) noexcept
        {
            std::array<Vertex, 3> vertices{};
            vertices[0].position = vec3{ 0.0f, 0.0f, 0.0f };
            vertices[1].position = vec3{ 1.0f, 0.0f, 0.0f };
            vertices[2].position = vec3{ 0.0f, 1.0f, 0.0f };

            std::array<uint32_t, 3> const indices{ 0u, 1u, 2u };
            std::array<uint32_t, 1> const faces{ 3u };

            mesh.setVertices(vertices);
            mesh.setIndices(indices);
            mesh.setFaceIndexCounts(faces);
            mesh.recalculateBounds();
        }

        /// <summary>
        /// Two triangles sharing an edge. Used where a test needs more than one face.
        /// </summary>
        void makeQuadMesh(GeoMesh& mesh) noexcept
        {
            std::array<Vertex, 4> vertices{};
            vertices[0].position = vec3{ 0.0f, 0.0f, 0.0f };
            vertices[1].position = vec3{ 1.0f, 0.0f, 0.0f };
            vertices[2].position = vec3{ 1.0f, 1.0f, 0.0f };
            vertices[3].position = vec3{ 0.0f, 1.0f, 0.0f };

            std::array<uint32_t, 6> const indices{ 0u, 1u, 2u, 0u, 2u, 3u };
            std::array<uint32_t, 2> const faces{ 3u, 3u };

            mesh.setVertices(vertices);
            mesh.setIndices(indices);
            mesh.setFaceIndexCounts(faces);
            mesh.recalculateBounds();
        }

        /// <summary>
        /// Non-triangulated: faces of 3, 4 and 5 indices. Exercises the variable-face path.
        /// </summary>
        void makeMixedFaceMesh(GeoMesh& mesh) noexcept
        {
            std::array<Vertex, 6> vertices{};

            for (uint32_t i = 0u; i < 6u; ++i)
            {
                vertices[i].position = vec3{ static_cast<float>(i), static_cast<float>(i) * -2.0f, 1.5f };
            }

            std::array<uint32_t, 12> const indices{ 0u, 1u, 2u, 0u, 1u, 2u, 3u, 0u, 1u, 2u, 3u, 4u };
            std::array<uint32_t, 3> const faces{ 3u, 4u, 5u };

            mesh.setVertices(vertices);
            mesh.setIndices(indices);
            mesh.setFaceIndexCounts(faces);
            mesh.recalculateBounds();
        }

        // -------------------------------------------------------------------------------------
        // Blob manipulation
        // -------------------------------------------------------------------------------------

        [[nodiscard]] std::vector<std::byte> serializeOrFail(GeoMesh const& mesh) noexcept
        {
            std::vector<std::byte> blob{};
            ErrorCode error = ErrorCode::None;

            REQUIRE(LitlMesh::serialize(mesh, blob, error) == true);
            REQUIRE(error == ErrorCode::None);

            return blob;
        }

        [[nodiscard]] Header readHeader(std::vector<std::byte> const& blob) noexcept
        {
            Header header{};
            std::memcpy(&header, blob.data(), sizeof(Header));
            return header;
        }

        template<typename T>
        void patch(std::vector<std::byte>& blob, size_t offset, T value) noexcept
        {
            REQUIRE((offset + sizeof(T)) <= blob.size());
            std::memcpy(blob.data() + offset, &value, sizeof(T));
        }

        void clearHeaderFlags(std::vector<std::byte>& blob) noexcept
        {
            constexpr size_t flagOffset = 44u;
            constexpr size_t flagEnd = flagOffset + sizeof(uint32_t);

            REQUIRE(blob.size() >= flagEnd);
            std::memset(blob.data() + flagOffset, 0u, sizeof(uint32_t));
        }

        /// <summary>
        /// Re-stamps contentHash. Only required for mutations at or past the descriptor table,
        /// since Header::validate runs before parse checks the hash.
        /// </summary>
        void rehash(std::vector<std::byte>& blob) noexcept
        {
            Header const header = readHeader(blob);
            uint64_t const hash = BinaryBlockFile::calculateContentHash(blob, header);
            std::memcpy(blob.data() + offsetof(Header, contentHash), &hash, sizeof(hash));
        }

        /// <summary>
        /// Byte offset of the descriptor carrying the given id. Avoids hardcoding block ordering.
        /// </summary>
        [[nodiscard]] size_t descriptorOffset(std::vector<std::byte> const& blob, BinaryBlockIdType id) noexcept
        {
            Header const header = readHeader(blob);

            for (uint32_t i = 0u; i < header.blockCount; ++i)
            {
                size_t const offset = static_cast<size_t>(header.descriptorsOffset) + (sizeof(BlockDescriptor) * i);

                BinaryBlockIdType blockId{};
                std::memcpy(&blockId, blob.data() + offset, sizeof(blockId));

                if (blockId == id)
                {
                    return offset;
                }
            }

            return 0ull;
        }

        [[nodiscard]] BlockDescriptor readDescriptor(std::vector<std::byte> const& blob, BinaryBlockIdType id) noexcept
        {
            size_t const offset = descriptorOffset(blob, id);
            REQUIRE(offset != 0ull);

            BlockDescriptor descriptor{};
            std::memcpy(&descriptor, blob.data() + offset, sizeof(descriptor));
            return descriptor;
        }

        void requireParseFails(std::vector<std::byte> const& blob, ErrorCode expected) noexcept
        {
            LitlMesh parsed{};
            ErrorCode error = ErrorCode::None;

            REQUIRE(LitlMesh::parse(blob, parsed, error) == false);
            REQUIRE(error == expected);
        }

        void requireDeserializeFails(std::vector<std::byte> const& blob, ErrorCode expected) noexcept
        {
            LitlMesh parsed{};
            ErrorCode error = ErrorCode::None;

            // Structure must still be valid - only the mesh semantics are wrong.
            REQUIRE(LitlMesh::parse(blob, parsed, error) == true);
            REQUIRE(error == ErrorCode::None);

            GeoMesh out{};
            REQUIRE(parsed.deserialize(out, error) == false);
            REQUIRE(error == expected);
        }

        void requireMeshesMatch(GeoMesh const& expected, GeoMesh const& actual) noexcept
        {
            REQUIRE(actual.vertexCount() == expected.vertexCount());
            REQUIRE(actual.indexCount() == expected.indexCount());
            REQUIRE(actual.faceCount() == expected.faceCount());

            REQUIRE(std::memcmp(actual.getVertices().data(), expected.getVertices().data(), expected.getVertices().size_bytes()) == 0);
            REQUIRE(std::memcmp(actual.getIndices().data(), expected.getIndices().data(), expected.getIndices().size_bytes()) == 0);
            REQUIRE(std::memcmp(actual.getFaceIndexCounts().data(), expected.getFaceIndexCounts().data(), expected.getFaceIndexCounts().size_bytes()) == 0);

            REQUIRE(actual.getBounds().min == expected.getBounds().min);
            REQUIRE(actual.getBounds().max == expected.getBounds().max);
        }

        /// <summary>
        /// Serializes the provided mesh, then parses the blob and deserializes it.
        /// Then compares the original source mesh and the round-trip mesh.
        /// </summary>
        void requireRoundTrip(GeoMesh const& source) noexcept
        {
            std::vector<std::byte> const blob = serializeOrFail(source);

            LitlMesh parsed{};
            ErrorCode error = ErrorCode::None;

            REQUIRE(LitlMesh::parse(blob, parsed, error) == true);
            REQUIRE(error == ErrorCode::None);

            GeoMesh restored{};
            REQUIRE(parsed.deserialize(restored, error) == true);
            REQUIRE(error == ErrorCode::None);

            requireMeshesMatch(source, restored);
        }
    }

    // -------------------------------------------------------------------------------------
    // Round trip
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("litlmesh round trips a single triangle", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeTriangleMesh(mesh);
        requireRoundTrip(mesh);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh round trips two triangles", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeQuadMesh(mesh);
        requireRoundTrip(mesh);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh round trips mixed face sizes", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeMixedFaceMesh(mesh);
        requireRoundTrip(mesh);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh round trips asymmetric bounds", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeQuadMesh(mesh);
        mesh.setBoundsMinMax(vec3{ -13.25f, -0.5f, 400.125f }, vec3{ -1.0f, 99.75f, 1024.0f });
        requireRoundTrip(mesh);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh serialization is deterministic", "[core::formats::litlmesh]")
    {
        // Proves the inter-block padding is zeroed. Without it the content hash - and therefore
        // any use of the file as a cache key - varies between runs of identical input.
        GeoMesh mesh{};
        makeMixedFaceMesh(mesh);

        std::vector<std::byte> const first = serializeOrFail(mesh);
        std::vector<std::byte> const second = serializeOrFail(mesh);

        REQUIRE(first.size() == second.size());
        REQUIRE(std::memcmp(first.data(), second.data(), first.size()) == 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh serialize rejects an empty mesh", "[core::formats::litlmesh]")
    {
        GeoMesh empty{};
        std::vector<std::byte> blob{};
        ErrorCode error = ErrorCode::None;

        REQUIRE(LitlMesh::serialize(empty, blob, error) == false);
        REQUIRE(error == ErrorCode::SourceMeshEmpty);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Malformed header
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("litlmesh parse rejects malformed headers", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeQuadMesh(mesh);

        std::vector<std::byte> const good = serializeOrFail(mesh);
        Header const header = readHeader(good);

        // Truncated below the header entirely.
        {
            std::vector<std::byte> blob(sizeof(Header) - 1u, std::byte(0));
            requireParseFails(blob, ErrorCode::InvalidFileSize);
        }

        {
            std::vector<std::byte> blob = good;
            patch(blob, offsetof(Header, magic), BinaryBlockIdType{ 'X', 'X', 'X', 'X' });
            requireParseFails(blob, ErrorCode::InvalidFileType);
        }

        {
            std::vector<std::byte> blob = good;
            patch<uint16_t>(blob, offsetof(Header, versionMajor), LitlMesh::Identity.versionMajor + 1u);
            requireParseFails(blob, ErrorCode::MajorVersionMismatch);
        }

        {
            std::vector<std::byte> blob = good;
            patch<uint16_t>(blob, offsetof(Header, versionMinor), LitlMesh::Identity.versionMinor + 1u);
            requireParseFails(blob, ErrorCode::MinorVersionMismatch);
        }

        // Blob shorter than the declared size.
        {
            std::vector<std::byte> blob = good;
            blob.resize(blob.size() - 1u);
            requireParseFails(blob, ErrorCode::InvalidFileSize);
        }

        // Declared size longer than the blob.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, offsetof(Header, totalBytes), header.totalBytes + 1u);
            requireParseFails(blob, ErrorCode::InvalidFileSize);
        }

        {
            std::vector<std::byte> blob = good;
            patch<uint32_t>(blob, offsetof(Header, blockCount), 0u);
            requireParseFails(blob, ErrorCode::WhereTheBlocksAt);
        }

        {
            std::vector<std::byte> blob = good;
            patch<uint32_t>(blob, offsetof(Header, blockCount), BinaryBlockFile::MaxBlocks + 1u);
            requireParseFails(blob, ErrorCode::TooManyBlocks);
        }

        // Descriptors claimed to start inside the header.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, offsetof(Header, descriptorsOffset), sizeof(Header) - 32u);
            requireParseFails(blob, ErrorCode::InvalidFirstDescriptorOffset);
        }

        // Descriptors claimed to start past the end of the file.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, offsetof(Header, descriptorsOffset), header.totalBytes + 1u);
            requireParseFails(blob, ErrorCode::InvalidFirstDescriptorOffset);
        }

        // REGRESSION: descriptorsOffset near 2^64 once wrapped when added to the descriptor
        // table size, letting parse read before the start of the buffer. Keep this test.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, offsetof(Header, descriptorsOffset), 0xFFFFFFFFFFFFFFC0ull);
            requireParseFails(blob, ErrorCode::InvalidFirstDescriptorOffset);
        }

        // blocksOffset inconsistent with descriptorsOffset + descriptor table size.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, offsetof(Header, blocksOffset), header.blocksOffset + sizeof(BlockDescriptor));
            requireParseFails(blob, ErrorCode::InvalidFirstBlockOffset);
        }

        // Corrupt payload byte, hash left untouched.
        {
            std::vector<std::byte> blob = good;
            BlockDescriptor const vertices = readDescriptor(blob, LitlMesh::BlockIds::Vertices);
            size_t const target = static_cast<size_t>(vertices.blockOffset);

            blob[target] = static_cast<std::byte>(std::to_integer<uint8_t>(blob[target]) ^ 0xFFu);
            requireParseFails(blob, ErrorCode::ContentHashMismatch);
        }
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Malformed descriptors
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("litlmesh parse rejects malformed block descriptors", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeQuadMesh(mesh);

        std::vector<std::byte> const good = serializeOrFail(mesh);
        Header const header = readHeader(good);

        size_t const vertexDescriptor = descriptorOffset(good, LitlMesh::BlockIds::Vertices);
        size_t const indexDescriptor = descriptorOffset(good, LitlMesh::BlockIds::Indices);

        REQUIRE(vertexDescriptor != 0ull);
        REQUIRE(indexDescriptor != 0ull);

        BlockDescriptor const vertices = readDescriptor(good, LitlMesh::BlockIds::Vertices);

        // Block starting before the first legal block offset.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, vertexDescriptor + offsetof(BlockDescriptor, blockOffset), 8ull);
            rehash(blob);
            requireParseFails(blob, ErrorCode::DescriptorBlockOutOfBounds);
        }

        // Block starting past the end of the file.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, vertexDescriptor + offsetof(BlockDescriptor, blockOffset), header.totalBytes + 16u);
            rehash(blob);
            requireParseFails(blob, ErrorCode::DescriptorBlockOutOfBounds);
        }

        // Block extending past the end of the file.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, vertexDescriptor + offsetof(BlockDescriptor, blockBytes), header.totalBytes);
            rehash(blob);
            requireParseFails(blob, ErrorCode::BlockSizeOutOfBounds);
        }

        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, vertexDescriptor + offsetof(BlockDescriptor, elementBytes), 0ull);
            rehash(blob);
            requireParseFails(blob, ErrorCode::ElementSizeOfZero);
        }

        // elementBytes * elementCount no longer equals blockBytes.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, vertexDescriptor + offsetof(BlockDescriptor, elementCount), vertices.elementCount + 1u);
            rehash(blob);
            requireParseFails(blob, ErrorCode::BlockSizeMismatch);
        }

        // Block offset no longer a multiple of 16.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, vertexDescriptor + offsetof(BlockDescriptor, blockOffset), vertices.blockOffset + 4u);
            rehash(blob);
            requireParseFails(blob, ErrorCode::InvalidBlockOffset);
        }

        // Index block dropped on top of the vertex block.
        {
            std::vector<std::byte> blob = good;
            patch<uint64_t>(blob, indexDescriptor + offsetof(BlockDescriptor, blockOffset), vertices.blockOffset);
            rehash(blob);
            requireParseFails(blob, ErrorCode::BlockOverlap);
        }
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Structurally valid, semantically invalid
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("litlmesh deserialize rejects an out of range index", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeQuadMesh(mesh);

        std::vector<std::byte> const good = serializeOrFail(mesh);
        BlockDescriptor const indices = readDescriptor(good, LitlMesh::BlockIds::Indices);

        // Exactly one past the last vertex.
        {
            std::vector<std::byte> blob = good;
            patch<uint32_t>(blob, static_cast<size_t>(indices.blockOffset), static_cast<uint32_t>(mesh.vertexCount()));
            rehash(blob);
            requireDeserializeFails(blob, ErrorCode::InvalidIndexFound);
        }

        // Wildly out of range.
        {
            std::vector<std::byte> blob = good;
            patch<uint32_t>(blob, static_cast<size_t>(indices.blockOffset), 0xFFFFFFFFu);
            rehash(blob);
            requireDeserializeFails(blob, ErrorCode::InvalidIndexFound);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh deserialize rejects malformed face counts", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeQuadMesh(mesh);
        mesh.setAllFaceIndexCounts(1u);     // change from triangle faces or else the AllTriangles flag will be set and the face block omitted.

        std::vector<std::byte> const good = serializeOrFail(mesh);
        BlockDescriptor const faces = readDescriptor(good, LitlMesh::BlockIds::Faces);
        size_t const firstFace = static_cast<size_t>(faces.blockOffset);

        {
            std::vector<std::byte> blob = good;
            patch<uint32_t>(blob, firstFace, 0u);
            clearHeaderFlags(blob);
            rehash(blob);
            requireDeserializeFails(blob, ErrorCode::ZeroFaceFound);
        }

        // Running sum exceeds the index count - caught by the early exit.
        {
            std::vector<std::byte> blob = good;
            patch<uint32_t>(blob, firstFace, static_cast<uint32_t>(mesh.indexCount()) + 1u);
            clearHeaderFlags(blob);
            rehash(blob);
            requireDeserializeFails(blob, ErrorCode::InvalidFaceSum);
        }

        // Total falls short of the index count - caught by the final comparison.
        {
            std::vector<std::byte> blob = good;
            patch<uint32_t>(blob, firstFace, 2u);
            clearHeaderFlags(blob);
            rehash(blob);
            requireDeserializeFails(blob, ErrorCode::InvalidFaceSum);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh deserialize rejects a malformed bounds block", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeTriangleMesh(mesh);

        std::vector<std::byte> blob = serializeOrFail(mesh);
        size_t const bounds = descriptorOffset(blob, LitlMesh::BlockIds::Bounds);

        REQUIRE(bounds != 0ull);

        // Five floats instead of six. Kept internally consistent so parse still succeeds.
        patch<uint64_t>(blob, bounds + offsetof(BlockDescriptor, elementCount), 5ull);
        patch<uint64_t>(blob, bounds + offsetof(BlockDescriptor, blockBytes), 5ull * sizeof(float));
        rehash(blob);

        requireDeserializeFails(blob, ErrorCode::InvalidBoundsValues);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh deserialize reports each missing block", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeTriangleMesh(mesh);
        mesh.setAllFaceIndexCounts(1u);     // Need this, otherwise when we serialize it will opt out of a FACE block anyways to just supply the AllTriangles flag.

        std::vector<std::byte> const good = serializeOrFail(mesh);

        struct Expectation
        {
            BinaryBlockIdType id;
            ErrorCode expected;
        };

        std::array<Expectation, 4> const expectations{
            Expectation{ LitlMesh::BlockIds::Vertices, ErrorCode::MissingVertexBlock },
            Expectation{ LitlMesh::BlockIds::Indices,  ErrorCode::MissingIndexBlock },
            Expectation{ LitlMesh::BlockIds::Faces,    ErrorCode::MissingFaceBlock },
            Expectation{ LitlMesh::BlockIds::Bounds,   ErrorCode::MissingBoundsBlock }
        };

        for (auto const& expectation : expectations)
        {
            std::vector<std::byte> blob = good;
            size_t const offset = descriptorOffset(blob, expectation.id);

            REQUIRE(offset != 0ull);

            // Rename rather than remove, so the file stays structurally valid.
            patch(blob, offset + offsetof(BlockDescriptor, blockId), BinaryBlockIdType{ 'N', 'O', 'P', 'E' });
            rehash(blob);

            requireDeserializeFails(blob, expectation.expected);
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh deserialize leaves the target mesh untouched on failure", "[core::formats::litlmesh]")
    {
        GeoMesh source{};
        makeQuadMesh(source);

        std::vector<std::byte> blob = serializeOrFail(source);
        BlockDescriptor const indices = readDescriptor(blob, LitlMesh::BlockIds::Indices);

        patch<uint32_t>(blob, static_cast<size_t>(indices.blockOffset), 0xFFFFFFFFu);
        rehash(blob);

        LitlMesh parsed{};
        ErrorCode error = ErrorCode::None;

        REQUIRE(LitlMesh::parse(blob, parsed, error) == true);

        // Pre-populate the destination, then confirm a failed load does not partially overwrite it.
        GeoMesh destination{};
        makeMixedFaceMesh(destination);

        GeoMesh expected{};
        makeMixedFaceMesh(expected);

        REQUIRE(parsed.deserialize(destination, error) == false);
        REQUIRE(error == ErrorCode::InvalidIndexFound);

        requireMeshesMatch(expected, destination);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // find / as<T> / preconditions
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("litlmesh find locates blocks and ignores unknown ids", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeQuadMesh(mesh);

        std::vector<std::byte> const blob = serializeOrFail(mesh);

        LitlMesh parsed{};
        ErrorCode error = ErrorCode::None;

        REQUIRE(LitlMesh::parse(blob, parsed, error) == true);

        auto const vertices = parsed.find(LitlMesh::BlockIds::Vertices);

        REQUIRE(vertices.has_value() == true);
        REQUIRE(vertices.value().elementBytes == sizeof(Vertex));
        REQUIRE(vertices.value().elementCount == mesh.vertexCount());
        REQUIRE(vertices.value().bytes.size() == (sizeof(Vertex) * mesh.vertexCount()));

        REQUIRE(parsed.find(BinaryBlockIdType{ 'Q', 'Q', 'Q', 'Q' }).has_value() == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh block as<T> rejects a mismatched element type", "[core::formats::litlmesh]")
    {
        GeoMesh mesh{};
        makeQuadMesh(mesh);

        std::vector<std::byte> const blob = serializeOrFail(mesh);

        LitlMesh parsed{};
        ErrorCode error = ErrorCode::None;

        REQUIRE(LitlMesh::parse(blob, parsed, error) == true);

        auto const vertices = parsed.find(LitlMesh::BlockIds::Vertices);

        REQUIRE(vertices.has_value() == true);
        REQUIRE(vertices.value().as<uint32_t>(error).has_value() == false);
        REQUIRE(error == ErrorCode::ElementSizeMismatch);

        error = ErrorCode::None; // clear error as .as() uses it for short-circuiting

        REQUIRE(vertices.value().as<Vertex>(error).has_value() == true);
        REQUIRE(error == ErrorCode::None);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("litlmesh parse requires an aligned source blob", "[core::formats::litlmesh]")
    {
        // Block offsets are relative, so a misaligned blob still parses - but the absolute
        // addresses handed to as<T> are then unusable. This pins the (implicit) precondition
        // that the caller supplies a blob aligned to at least 16 bytes.
        GeoMesh mesh{};
        makeQuadMesh(mesh);

        std::vector<std::byte> const aligned = serializeOrFail(mesh);

        std::vector<std::byte> shifted(aligned.size() + 1u, std::byte(0));
        std::memcpy(shifted.data() + 1u, aligned.data(), aligned.size());

        std::span<std::byte const> const view{ shifted.data() + 1u, aligned.size() };

        LitlMesh parsed{};
        ErrorCode error = ErrorCode::None;

        REQUIRE(LitlMesh::parse(view, parsed, error) == true);

        auto const vertices = parsed.find(LitlMesh::BlockIds::Vertices);

        REQUIRE(vertices.has_value() == true);
        REQUIRE(vertices.value().as<Vertex>(error).has_value() == false);
        REQUIRE(error == ErrorCode::ElementOffsetAlignmentMismatch);
    } LITL_END_TEST_CASE
}