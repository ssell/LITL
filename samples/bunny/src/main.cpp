#include "litl-engine/startup.hpp"

using namespace litl;

void bootstrap(ServiceProvider& services, EntityCommands& commands);
MaterialRef createPlaceholderMaterial(ObjectPool& objectPool);

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

    const vec3 cameraPos = vec3{ 0.0f, 1.0f, 0.0f };
    const vec3 bunnyPos = vec3{ 0.0f, 0.0f, 3.0f };

    createMainCamera(
        CameraDescriptor{ .projection = CameraProjection::Perspective, .clearColor = color{ 0.035f, 0.035f, 0.05f } }, 
        cameraPos, 
        bunnyPos,
        vec3::up(), 
        *objectPool, 
        *sceneView);

    // ... in progress ...

    auto bunnyMesh = assets->getMesh("mesh/bunny");                     // maps to "assets/mesh/bunny.litlmesh"
    auto bunnyMaterialRef = createPlaceholderMaterial(*objectPool);
    auto bunnyEntity = commands.createEntity();

    commands.addComponent<Transform>(bunnyEntity, Transform::create(bunnyPos - vec3::right() * 0.75f));
    commands.addComponent<LocalBounds>(bunnyEntity, LocalBounds{});     // todo these need to come from the mesh ...
    commands.addComponent<WorldBounds>(bunnyEntity, WorldBounds{});
    commands.addComponent<MaterialRef>(bunnyEntity, bunnyMaterialRef);
    commands.addComponent<MeshRef>(bunnyEntity, MeshRef{ .handle = bunnyMesh->handle });

    auto* material = objectPool->getMaterial(bunnyMaterialRef.handle);

    material->setColor("tint"_sid, colors::Orange, bunnyMaterialRef.slot);
    material->setFloat("fade"_sid, 1.0f, bunnyMaterialRef.slot);

    // --- test second bunny

    auto bunnyEntity2 = commands.createEntity();
    auto bunnyEntity2MaterialRef = bunnyMaterialRef;
    bunnyEntity2MaterialRef.slot = material->allocateSlot();

    commands.addComponent<Transform>(bunnyEntity2, Transform::create(bunnyPos + vec3::right() * 0.75f));
    commands.addComponent<LocalBounds>(bunnyEntity2, LocalBounds{});     // todo these need to come from the mesh ...
    commands.addComponent<WorldBounds>(bunnyEntity2, WorldBounds{});
    commands.addComponent<MaterialRef>(bunnyEntity2, bunnyEntity2MaterialRef);
    commands.addComponent<MeshRef>(bunnyEntity2, MeshRef{ .handle = bunnyMesh->handle });

    material->setColor("tint"_sid, colors::Green, bunnyEntity2MaterialRef.slot);
    material->setFloat("fade"_sid, 1.0f, bunnyEntity2MaterialRef.slot);
}

MaterialRef createPlaceholderMaterial(ObjectPool& objectPool)
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

    return MaterialRef{
        .handle = materialHandle,
        .slot = objectPool.getMaterial(materialHandle)->allocateSlot()
    };
}