#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <vector>

#include "litl-renderer/pipeline/shaderReflection.hpp"

TEST_CASE("Basic Reflection", "[renderer::shaderReflection]")
{
    std::ifstream file("data/test.spv", std::ios::ate | std::ios::binary);

    REQUIRE(file.is_open());

    const auto fileSize = static_cast<uint32_t>(file.tellg());
    std::vector<uint8_t> fileBuffer(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(fileBuffer.data()), fileSize);
    file.close();

    std::span<uint8_t> bytes(fileBuffer);

    auto reflectedVertex = LITL::Renderer::reflectSPIRV("vertexMain", bytes);
    auto reflectedFragment = LITL::Renderer::reflectSPIRV("fragmentMain", bytes);
    auto reflectedGeometry = LITL::Renderer::reflectSPIRV("geometryMain", bytes);

    REQUIRE(reflectedVertex != std::nullopt);
    REQUIRE(reflectedFragment != std::nullopt);
    REQUIRE(reflectedGeometry == std::nullopt);
}