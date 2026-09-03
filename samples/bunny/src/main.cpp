#include "litl-engine/startup.hpp"
#include "flashSystem.hpp"

using namespace litl;

void configureSystems(SystemCollection& systems);
void bootstrap(ServiceProvider& services, EntityCommands& commands);
MaterialHandle createPlaceholderMaterial(ObjectPool& objectPool);
void createBunny(EntityCommands& commands, MeshHandle meshHandle, MaterialHandle materialHandle, Material* material, vec3 position, color color, bool flashing);

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Bunny Sample" } },
        nullptr,
        configureSystems,
        nullptr,
        bootstrap);

    engine.start();

    return 0;
}

void configureSystems(SystemCollection& systems)
{
    systems.addSystem<samples::FlashSystem>(SystemGroup::Update);
}

void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto objectPool = services.get<ObjectPool>();       // Source of common objects such as GPU buffers, meshes, materials, cameras, etc.
    auto sceneView = services.get<SceneView>();         // A view into the current active scene.
    auto assets = services.get<AssetManager>();

    createMainCamera(
        CameraDescriptor{ .projection = CameraProjection::Perspective, .clearColor = color{ 0.035f, 0.035f, 0.05f } }, 
        vec3::zero(),
        vec3::forward(),
        vec3::up(), 
        *objectPool, 
        *sceneView);

    // ... in progress ...

    //auto* bunnyShader = assets->getShader("shaders/test");
    auto* bunnyMaterial = assets->getMaterial("materials/flat");

    /*
    auto* bunnyMesh = assets->getMesh("mesh/bunny");                             // maps to "assets/mesh/bunny.litlmesh"
    auto* bunnyMaterialTest = assets->getMaterial("materials/flat");
    auto bunnyMaterialHandle = createPlaceholderMaterial(*objectPool);
    auto* bunnyMaterial = objectPool->getMaterial(bunnyMaterialHandle);

    if (bunnyMesh->handle.isValid() && bunnyMaterialHandle.isValid() && (bunnyMaterial != nullptr))
    {
        bunnyMaterial->setDefaultColor("tint"_sid, colors::White);
        bunnyMaterial->setDefaultFloat("fade"_sid, 1.0f);

        for (int32_t x = -10; x <= 10; ++x)
        {
            for (int32_t y = -10; y <= 10; ++y)
            {
                createBunny(
                    commands, 
                    bunnyMesh->handle, 
                    bunnyMaterialHandle, 
                    bunnyMaterial, 
                    vec3{static_cast<float>(x) * 1.5f, static_cast<float>(y) * 1.5f, 28.0f },
                    color{ static_cast<float>(x + 10) * 0.05f, static_cast<float>(y + 10) * 0.05f, 0.0f },
                    ((x == 0u) && (y == 0u)));
            }
        }
    }
    */
}

MaterialHandle createPlaceholderMaterial(ObjectPool& objectPool)
{
    // ... todo this will eventually be defined by a material asset ...
    auto spirvBytes = File("assets/shaders/spirv/test.spv").readAllBytes();

    auto materialHandle = objectPool.createMaterial(MaterialDescriptor{
        .objectInfo = ObjectDescriptor {.name = "Test Material" },
        .vertexShader = ShaderResourceDescriptor {
            .resource = "test.spv",
            .entryPoint = "vertexMain",
            .bytes = spirvBytes.value()
        },
        .fragmentShader = ShaderResourceDescriptor {
            .resource = "test.spv",
            .entryPoint = "fragmentMain",
            .bytes = spirvBytes.value()
        }
    });

    return materialHandle;
}

void createBunny(EntityCommands& commands, MeshHandle meshHandle, MaterialHandle materialHandle, Material* material, vec3 position, color color, bool flashing)
{
    const DeferredEntity entity = commands.createEntity();

    const MaterialRef materialRef{
        .handle = materialHandle,
        .slot = material->allocateSlot(),
    };

    commands.addComponent<Transform>(entity, Transform::create(position));
    commands.addComponent<LocalBounds>(entity, LocalBounds{});     // todo these need to come from the mesh ...
    commands.addComponent<WorldBounds>(entity, WorldBounds{});
    commands.addComponent<MaterialRef>(entity, materialRef);
    commands.addComponent<MeshRef>(entity, MeshRef{ .handle = meshHandle });

    if (flashing)
    {
        commands.addComponent<samples::Flash>(entity, {});
    }


    material->setColor("tint"_sid, (flashing ? colors::White : color), materialRef.slot);
}