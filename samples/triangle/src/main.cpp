#include <fstream>

#include "litl-engine/startup.hpp"
#include "litl-core/containers/alignedByteBuffer.hpp"
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

    sceneView->setMainCamera(objectPool->createCamera({}));

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

    commands.addComponent(triangleEntity, transform);
    commands.addComponent(triangleEntity, MaterialRef{ .handle = material });
    commands.addComponent(triangleEntity, MeshRef{ .handle = mesh });
    commands.addComponent(triangleEntity, Spin{ .rate = spinRate });
}

/// <summary>
/// Loads the common flat shader and creates a material using it.
/// </summary>
MaterialHandle createTriangleMaterial(ObjectPool& objectPool)
{
    // Load the shader SPIR-V bytes
    const std::string resourcePath = "assets/shaders/spirv/flat.spv";
    std::ifstream file(resourcePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        return{};
    }

    const auto fileSizeBytes = static_cast<size_t>(file.tellg());
    AlignedByteBuffer<4> byteBuffer{ fileSizeBytes };

    file.seekg(0);
    file.read(byteBuffer.as<char>().data(), byteBuffer.size());
    file.close();

    auto spirvBytes = byteBuffer.as<std::byte const>();

    // Create the material
    auto materialDescriptor = MaterialDescriptor{
        .objectInfo = ObjectDescriptor {.name = "Flat" },
        .vertexShader = ShaderResourceDescriptor {
            .resource = resourcePath,
            .entryPoint = "vertexMain",
            .bytes = spirvBytes
        },
        .fragmentShader = ShaderResourceDescriptor {
            .resource = resourcePath,
            .entryPoint = "fragmentMain",
            .bytes = spirvBytes
        }
    };

    return objectPool.createMaterial(materialDescriptor);
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