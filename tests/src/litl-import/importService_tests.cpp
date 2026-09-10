#include <array>
#include <string_view>

#include "tests.hpp"
#include "litl-import/importService.hpp"

namespace litl::tests
{
    namespace
    {
        struct ItemDedupe
        {
            import::ImportedDataType type;
            std::string originalName;
            std::string expectedName;
        };
    }

    LITL_TEST_CASE("sanitizeAndDeduplicateItemNames", "[import::importService]")
    {
        const std::array<ItemDedupe, 10> items{
            ItemDedupe{ import::ImportedDataType::Mesh, "wall", "wall" },
            ItemDedupe{ import::ImportedDataType::Mesh, "WALL", "wall_1" },
            ItemDedupe{ import::ImportedDataType::Mesh, "wall", "wall_2" },
            ItemDedupe{ import::ImportedDataType::Material, "", "material_0" },
            ItemDedupe{ import::ImportedDataType::Material, "", "material_1" },
            ItemDedupe{ import::ImportedDataType::Material, "", "material_2" },
            ItemDedupe{ import::ImportedDataType::Shader, "  wall   SHADER!? lets go!!!!", "wall-shader!_-lets-go!!!!" },
            ItemDedupe{ import::ImportedDataType::Model, "aux_model", "aux_model" },
            ItemDedupe{ import::ImportedDataType::Model, "model_0", "model_0" },
            ItemDedupe{ import::ImportedDataType::Model, "aux", "model_0_1" }         // "aux" is reserved so should get cleared and then a type-based name generated.
        };                                                                            // this would typically make it "model_0" but that will already exist so it becomes "model_0_1"

        import::ImportedData data;

        for (auto& item : items)
        {
            data.items.push_back({});
            REQUIRE(data.items.back().setType(item.type) == true);
            data.items.back().setName(item.originalName);
        }

        REQUIRE(data.items.size() == items.size());
        import::sanitizeAndDeduplicateImportedItemNames(data);
        REQUIRE(data.items.size() == items.size());

        for (uint32_t i = 0u; i < static_cast<uint32_t>(data.items.size()); ++i)
        {
            auto& dataItem = data.items[i];

            REQUIRE(dataItem.getType() == items[i].type);
            REQUIRE(dataItem.getName() == items[i].expectedName);
        }

    } LITL_END_TEST_CASE
}