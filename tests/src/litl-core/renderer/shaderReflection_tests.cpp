#include <fstream>
#include <vector>

#include "tests.hpp"
#include "litl-core/containers/alignedByteBuffer.hpp"
#include "litl-renderer/reflection.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Basic Reflection", "[renderer::shaderReflection]")
    {
        std::ifstream file("data/test.spv", std::ios::ate | std::ios::binary);

        REQUIRE(file.is_open());

        const auto fileSizeBytes = static_cast<size_t>(file.tellg());
        AlignedByteBuffer<4> byteBuffer{ fileSizeBytes };               // alignment of 4 as vkCreateShaderModule requires 4-byte aligment

        file.seekg(0);
        file.read(byteBuffer.as<char>().data(), byteBuffer.size());
        file.close();

        auto reflectedVertex = litl::reflectSPIRV("vertexMain", byteBuffer.as<std::byte>());
        auto reflectedFragment = litl::reflectSPIRV("fragmentMain", byteBuffer.as<std::byte>());
        auto reflectedGeometry = litl::reflectSPIRV("geometryMain", byteBuffer.as<std::byte>());

        REQUIRE(reflectedVertex != std::nullopt);
        REQUIRE(reflectedFragment != std::nullopt);
        REQUIRE(reflectedGeometry == std::nullopt);

        REQUIRE(reflectedVertex->stage == ShaderStage::Vertex);
        REQUIRE(reflectedFragment->stage == ShaderStage::Fragment);
    } LITL_END_TEST_CASE
}