#include "litl-engine/startup.hpp"
#include "spinSystem.hpp"

using namespace litl;

void configureSystems(SystemCollection& systems);
void bootstrap(ServiceProvider& services, EntityCommands& commands);
void createSpinningTriangle(EntityCommands& commands, MaterialHandle material, MeshHandle mesh, vec3 position, float spinRate);
MaterialHandle createTriangleMaterial(ObjectPool& objectPool);
MeshHandle createTriangleMesh(ObjectPool& objectPool);

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Triangle Sample" } },
        nullptr,    // this sample uses no custom services
        configureSystems,
        bootstrap,
        nullptr);   // this sample uses no custom callbacks

    engine.start();

    return 0;
}

/// <summary>
/// This is called once by the engine and provides the user a chance to add their custom systems.
/// </summary>
void configureSystems(SystemCollection& systems)
{
    systems.addSystem<SpinSystem>(SystemGroup::Update);
}

/// <summary>
/// This is called once by the engine after all services and systems have been configured.
/// It serves as a place for the user to create the initial entities in the scene and perform other setup prior to the engine starting its frame loop.
/// </summary>
void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto objectPool = services.get<ObjectPool>();       // Source of common objects such as GPU buffers, meshes, materials, cameras, etc.
    auto sceneView = services.get<SceneView>();         // A view into the current active scene.

    auto cameraHandle = objectPool->createCamera({ .projection = CameraProjection::Perspective, .clearColor = { 0.035f, 0.035f, 0.05f } });
    auto* camera = objectPool->getCamera(cameraHandle);

    sceneView->setMainCamera(cameraHandle);
    camera->setWorldPosition(vec3{ 0.0f, 0.0f, 0.0f });
    camera->lookAt(vec3::forward(), vec3::up());

    auto triangleMaterial = createTriangleMaterial(*objectPool);
    auto triangleMesh = createTriangleMesh(*objectPool);

    createSpinningTriangle(commands, triangleMaterial, triangleMesh, { 0.0f, -0.35f, 2.0f }, 1.0f);
}

/// <summary>
/// Creates a single spinning triangle at the specified position with the given spin rate.
/// </summary>
void createSpinningTriangle(EntityCommands& commands, MaterialHandle material, MeshHandle mesh, vec3 position, float spinRate)
{
    auto triangleEntity = commands.createEntity();      // Note that this is a DeferredEntity. It will be materialized into a true Entity when the commands are processed.

    Transform transform{};
    transform.setPosition(position);

    commands.addComponent<Transform>(triangleEntity, transform);
    commands.addComponent<LocalBounds>(triangleEntity, LocalBounds{});
    commands.addComponent<WorldBounds>(triangleEntity, WorldBounds{});
    commands.addComponent<MaterialRef>(triangleEntity, MaterialRef{ .handle = material });
    commands.addComponent<MeshRef>(triangleEntity, MeshRef{ .handle = mesh });
    commands.addComponent<Spin>(triangleEntity, Spin{ .rate = spinRate });
}

/// <summary>
/// Loads the common flat shader and creates a material using it.
/// </summary>
MaterialHandle createTriangleMaterial(ObjectPool& objectPool)
{
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

/// <summary>
/// Creates a basic triangle mesh with colored vertices.
/// </summary>
MeshHandle createTriangleMesh(ObjectPool& objectPool)
{
    std::array<Vertex, 3> vertices = {
        Vertex {                                        // left
            .position = { -0.5f, 0.0f, 0.0f },
            .color = { 0.0f, 1.0f, 0.0f },
            .uv = { 0.0f, 0.0f }
        },
        Vertex {                                        // top
            .position = { 0.0f, 1.0f, 0.0f },
            .color = { 1.0f, 0.0f, 0.0f },
            .uv = { 0.5f, 1.0f }
        },
        Vertex {                                        // right
            .position = { 0.5f, 0.0f, 0.0f },
            .color = { 0.0f, 0.0f, 1.0f },
            .uv = { 1.0f, 0.0f }
        }
    };

    std::array<uint32_t, 3> indices = { 0, 1, 2 };

    auto meshHandle = objectPool.createMesh(MeshDescriptor{
        .objectInfo = ObjectDescriptor { .name = "Triangle" },
        .vertexInfo = MeshVertexDescriptor{
            .vertexCount = 3u,
            .vertexByteSize = sizeof(Vertex),
            .vertexData = as_byte_span(vertices)
        },
        .indexInfo = MeshIndexDescriptor{
            .indexCount = 3u,
            .indexByteSize = sizeof(uint32_t),
            .indexData = as_byte_span(indices)
        }
    });

    return meshHandle;
}