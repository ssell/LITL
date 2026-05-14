#include <fstream>
#include <vector>

#include "tests.hpp"
#include "litl-renderer/pipeline/shader/reflection.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Basic Reflection", "[renderer::shaderReflection]")
    {
        std::ifstream file("data/test.spv", std::ios::ate | std::ios::binary);

        REQUIRE(file.is_open());

        const auto fileSize = static_cast<uint32_t>(file.tellg());
        std::vector<uint8_t> fileBuffer(fileSize);

        file.seekg(0);
        file.read(reinterpret_cast<char*>(fileBuffer.data()), fileSize);
        file.close();

        std::span<uint8_t> bytes(fileBuffer);

        auto reflectedVertex = litl::reflectSPIRV("vertexMain", bytes);
        auto reflectedFragment = litl::reflectSPIRV("fragmentMain", bytes);
        auto reflectedGeometry = litl::reflectSPIRV("geometryMain", bytes);

        REQUIRE(reflectedVertex != std::nullopt);
        REQUIRE(reflectedFragment != std::nullopt);
        REQUIRE(reflectedGeometry == std::nullopt);
    } LITL_END_TEST_CASE
}