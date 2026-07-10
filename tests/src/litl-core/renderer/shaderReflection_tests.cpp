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

        const std::optional<ShaderReflection> reflectedShader = litl::reflectSPIRV(byteBuffer.as<std::byte>());

        REQUIRE(reflectedShader != std::nullopt);
        REQUIRE(reflectedShader->entryPoints.size() == 2);

        const bool hasVertex = (reflectedShader->entryPoints[0].stage == ShaderStage::Vertex) || (reflectedShader->entryPoints[1].stage == ShaderStage::Vertex);
        const bool hasFragment = (reflectedShader->entryPoints[0].stage == ShaderStage::Fragment) || (reflectedShader->entryPoints[1].stage == ShaderStage::Fragment);

        REQUIRE(hasVertex == true);
        REQUIRE(hasFragment == true);
    } LITL_END_TEST_CASE
}