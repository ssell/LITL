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

        auto reflectedShader = litl::reflectSPIRV(byteBuffer.as<std::byte>());

        REQUIRE(reflectedShader != std::nullopt);
        REQUIRE(reflectedShader->entryPoints.size() == 2);
        REQUIRE((reflectedShader->entryPoints[0].stage == ShaderStage::Vertex) || (reflectedShader->entryPoints[0].stage == ShaderStage::Fragment));
        REQUIRE((reflectedShader->entryPoints[1].stage == ShaderStage::Vertex) || (reflectedShader->entryPoints[1].stage == ShaderStage::Fragment));
    } LITL_END_TEST_CASE
}