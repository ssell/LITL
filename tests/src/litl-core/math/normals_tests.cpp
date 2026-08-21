#include "tests.hpp"
#include "litl-core/math/uncommon.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("triangle normal up", "[math::normals]")
    {
        const vec3 v0{ 0.0f, 0.0f, 0.0f };
        const vec3 v1{ 2000.0f, 0.0f, -50.0f };
        const vec3 v2{ 1337.0f, 0.0f, 1337.0f };

        const vec3 normalCW = faceNormal(v0, v1, v2);
        const vec3 normalCCW = faceNormalCCW(v0, v1, v2);

        REQUIRE(normalCW == vec3::up());
        REQUIRE(normalCCW == -normalCW);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangle normal right", "[math::normals]")
    {
        const vec3 v0{ 0.0f, 0.0f, 0.0f };
        const vec3 v1{ 0.0f, 1337.0f, 1337.0f };
        const vec3 v2{ 0.0f, 2000.0f, -50.0f };

        const vec3 normalCW = faceNormal(v0, v1, v2);
        const vec3 normalCCW = faceNormalCCW(v0, v1, v2);

        REQUIRE(normalCW == vec3::right());
        REQUIRE(normalCCW == -normalCW);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("triangle normal forward", "[math::normals]")
    {
        const vec3 v0{ 0.0f, 0.0f, 0.0f };
        const vec3 v1{ 1337.0f, 1337.0f, 0.0f };
        const vec3 v2{ 2000.0f, -50.0f, 0.0f };

        const vec3 normalCW = faceNormal(v0, v1, v2);
        const vec3 normalCCW = faceNormalCCW(v0, v1, v2);

        REQUIRE(normalCW == vec3::forward());
        REQUIRE(normalCCW == -normalCW);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("ngon normal up", "[math::normals]")
    {
        std::array<vec3, 5> positions{
            vec3{ 0.0f, 0.0f, 0.0f },
            vec3{ 10.0f, 0.0f, -10.0f },
            vec3{ 20.0f, 0.0f, 0.0f },
            vec3{ 15.0f, 0.0f, 5.0f },
            vec3{ 5.0f, 0.0f, 10.0f }
        };

        const vec3 pointNormalCW = ngonFaceNormal({ positions.begin(), 1 });
        const vec3 pointNormalCCW = ngonFaceNormalCCW({ positions.begin(), 1 });

        REQUIRE(pointNormalCW == vec3{});
        REQUIRE(pointNormalCCW == vec3{});

        const vec3 lineNormalCW = ngonFaceNormal({ positions.begin(), 2 });
        const vec3 lineNormalCCW = ngonFaceNormalCCW({ positions.begin(), 2 });

        REQUIRE(lineNormalCW == vec3{});
        REQUIRE(lineNormalCCW == vec3{});

        const vec3 triNormalCW = ngonFaceNormal({ positions.begin(), 3 });
        const vec3 triNormalCCW = ngonFaceNormalCCW({ positions.begin(), 3 });

        REQUIRE(triNormalCW == vec3::up());
        REQUIRE(triNormalCCW == -triNormalCW);

        const vec3 quadNormalCW = ngonFaceNormal({ positions.begin(), 4 });
        const vec3 quadNormalCCW = ngonFaceNormalCCW({ positions.begin(), 4 });

        REQUIRE(quadNormalCW == vec3::up());
        REQUIRE(quadNormalCCW == -quadNormalCW);

        const vec3 ngonNormalCW = ngonFaceNormal(positions);
        const vec3 ngonNormalCCW = ngonFaceNormalCCW(positions);

        REQUIRE(ngonNormalCW == vec3::up());
        REQUIRE(ngonNormalCCW == -ngonNormalCW);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("ngon normal up vertices", "[math::normals]")
    {
        std::array<Vertex, 5> positions{
            Vertex{.position = vec3{ 0.0f, 0.0f, 0.0f }},
            Vertex{.position = vec3{ 10.0f, 0.0f, -10.0f }},
            Vertex{.position = vec3{ 20.0f, 0.0f, 0.0f }},
            Vertex{.position = vec3{ 15.0f, 0.0f, 5.0f }},
            Vertex{.position = vec3{ 5.0f, 0.0f, 10.0f }}
        };

        const vec3 pointNormalCW = ngonFaceNormal({ positions.begin(), 1 });
        const vec3 pointNormalCCW = ngonFaceNormalCCW({ positions.begin(), 1 });

        REQUIRE(pointNormalCW == vec3{});
        REQUIRE(pointNormalCCW == vec3{});

        const vec3 lineNormalCW = ngonFaceNormal({ positions.begin(), 2 });
        const vec3 lineNormalCCW = ngonFaceNormalCCW({ positions.begin(), 2 });

        REQUIRE(lineNormalCW == vec3{});
        REQUIRE(lineNormalCCW == vec3{});

        const vec3 triNormalCW = ngonFaceNormal({ positions.begin(), 3 });
        const vec3 triNormalCCW = ngonFaceNormalCCW({ positions.begin(), 3 });

        REQUIRE(triNormalCW == vec3::up());
        REQUIRE(triNormalCCW == -triNormalCW);

        const vec3 quadNormalCW = ngonFaceNormal({ positions.begin(), 4 });
        const vec3 quadNormalCCW = ngonFaceNormalCCW({ positions.begin(), 4 });

        REQUIRE(quadNormalCW == vec3::up());
        REQUIRE(quadNormalCCW == -quadNormalCW);

        const vec3 ngonNormalCW = ngonFaceNormal(positions);
        const vec3 ngonNormalCCW = ngonFaceNormalCCW(positions);

        REQUIRE(ngonNormalCW == vec3::up());
        REQUIRE(ngonNormalCCW == -ngonNormalCW);
    } LITL_END_TEST_CASE
}