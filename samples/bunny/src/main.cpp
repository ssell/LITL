#include "litl-engine/startup.hpp"

using namespace litl;

void bootstrap(ServiceProvider& services, EntityCommands& commands);
MaterialHandle createPlaceholderMaterial(ObjectPool& objectPool);

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
        vec3{}, 
        vec3::forward(), 
        vec3::up(), 
        *objectPool, 
        *sceneView);

    // ... in progress ...

    auto bunnyMesh = assets->getMesh("mesh/bunny");     // maps to "assets/mesh/bunny.litlmesh"
    auto bunnyMaterial = createPlaceholderMaterial(*objectPool);

    // ... todo below is dependent on finishing the mesh asset pipeline (gpu buffer upload) ...
    /*
    auto bunnyEntity = commands.createEntity();

    Transform transform{};
    transform.setPosition(vec3{ 0.0f, 0.0f, 5.0f });

    commands.addComponent<Transform>(bunnyEntity, transform);
    commands.addComponent<LocalBounds>(bunnyEntity, LocalBounds{});     // todo these need to come from the mesh ...
    commands.addComponent<WorldBounds>(bunnyEntity, WorldBounds{});
    commands.addComponent<MaterialRef>(bunnyEntity, MaterialRef{ .handle = bunnyMaterial });
    commands.addComponent<MeshRef>(bunnyEntity, MeshRef{ .handle = bunnyMesh->handle });
    */
}

MaterialHandle createPlaceholderMaterial(ObjectPool& objectPool)
{
    // ... todo this will eventually be defined by a material asset ...
    auto spirvBytes = File("assets/shaders/spirv/flat.spv").readAllBytes();

    return objectPool.createMaterial(MaterialDescriptor{
        .objectInfo = ObjectDescriptor {.name = "Flat" },
        .vertexShader = ShaderResourceDescriptor {
            .resource = "flat.spv",
            .entryPoint = "vertexMain",
            .bytes = spirvBytes.value()
        },
        .fragmentShader = ShaderResourceDescriptor {
            .resource = "flat.spv",
            .entryPoint = "fragmentMain",
            .bytes = spirvBytes.value()
        }
        });
}