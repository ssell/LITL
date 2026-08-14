#include "litl-engine/startup.hpp"

using namespace litl;

void bootstrap(ServiceProvider& services, EntityCommands& commands);

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
}