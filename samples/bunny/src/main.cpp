#include "litl-engine/startup.hpp"
#include "flashSystem.hpp"

using namespace litl;

void configureSystems(SystemCollection& systems);
void bootstrap(ServiceProvider& services, EntityCommands& commands);
MaterialHandle createPlaceholderMaterial(ObjectPool& objectPool);
void createBunny(EntityCommands& commands, MeshHandle meshHandle, Material* material, MaterialRef const& materialRef, vec3 position, color color, bool flashing);

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

    auto* bunnyMesh = assets->getMesh("mesh/bunny");
    auto* bunnyMaterial = assets->getMaterial("materials/flat");

    if (bunnyMesh->handle.isValid() && bunnyMaterial->handle.isValid())
    {
        for (int32_t x = -10; x <= 10; ++x)
        {
            for (int32_t y = -10; y <= 10; ++y)
            {
                createBunny(
                    commands, 
                    bunnyMesh->handle, 
                    bunnyMaterial->material,
                    bunnyMaterial->allocate(),
                    vec3{static_cast<float>(x) * 1.5f, static_cast<float>(y) * 1.5f, 28.0f },
                    color{ static_cast<float>(x + 10) * 0.05f, static_cast<float>(y + 10) * 0.05f, 0.0f },
                    ((x == 0u) && (y == 0u)));
            }
        }
    }
}

void createBunny(EntityCommands& commands, MeshHandle meshHandle, Material* material, MaterialRef const& materialRef, vec3 position, color color, bool flashing)
{
    const DeferredEntity entity = commands.createEntity();

    commands.addComponent<Transform>(entity, Transform::create(position));
    commands.addComponent<LocalBounds>(entity, LocalBounds{});      // todo these need to come from the mesh ...
    commands.addComponent<WorldBounds>(entity, WorldBounds{});
    commands.addComponent<MaterialRef>(entity, materialRef);
    commands.addComponent<MeshRef>(entity, MeshRef{ .handle = meshHandle });

    if (flashing)
    {
        commands.addComponent<samples::Flash>(entity, {});
    }

    material->setColor("tint"_sid, (flashing ? colors::White : color), materialRef.slot);
}