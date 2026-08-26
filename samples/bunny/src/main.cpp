#include "litl-engine/startup.hpp"

using namespace litl;

void bootstrap(ServiceProvider& services, EntityCommands& commands);
MaterialHandle createPlaceholderMaterial(ObjectPool& objectPool);
void createBunny(EntityCommands& commands, MeshHandle meshHandle, MaterialHandle materialHandle, Material* material, vec3 position, color color);

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Bunny Sample" } },
        nullptr,
        nullptr,
        nullptr,
        bootstrap);

    engine.start();

    return 0;
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

    auto* bunnyMesh = assets->getMesh("mesh/bunny");                             // maps to "assets/mesh/bunny.litlmesh"
    auto bunnyMaterialHandle = createPlaceholderMaterial(*objectPool);
    auto* bunnyMaterial = objectPool->getMaterial(bunnyMaterialHandle);

    if (bunnyMesh->handle.isValid() && bunnyMaterialHandle.isValid() && (bunnyMaterial != nullptr))
    {
        for (int32_t x = -10; x <= 10; ++x)
        {
            for (int32_t y = -10; y < 10; ++y)
            {
                createBunny(
                    commands, 
                    bunnyMesh->handle, 
                    bunnyMaterialHandle, 
                    bunnyMaterial, 
                    vec3{static_cast<float>(x) * 1.5f, static_cast<float>(y) * 1.5f, 28.0f },
                    color{ static_cast<float>(x + 10) * 0.05f, static_cast<float>(y + 10) * 0.05f, 0.0f });
            }
        }
    }
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

void createBunny(EntityCommands& commands, MeshHandle meshHandle, MaterialHandle materialHandle, Material* material, vec3 position, color color)
{
    auto entity = commands.createEntity();

    const MaterialRef materialRef{
        .handle = materialHandle,
        .slot = material->allocateSlot()
    };

    commands.addComponent<Transform>(entity, Transform::create(position));
    commands.addComponent<LocalBounds>(entity, LocalBounds{});     // todo these need to come from the mesh ...
    commands.addComponent<WorldBounds>(entity, WorldBounds{});
    commands.addComponent<MaterialRef>(entity, materialRef);
    commands.addComponent<MeshRef>(entity, MeshRef{ .handle = meshHandle });

    material->setColor("tint"_sid, color, materialRef.slot);
    material->setFloat("fade"_sid, 1.0f, materialRef.slot);
}