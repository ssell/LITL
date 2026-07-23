#include <fstream>

#include "litl-engine/startup.hpp"
#include "litl-core/containers/alignedByteBuffer.hpp"
#include "spinSystem.hpp"

using namespace litl;

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
        .objectInfo = ObjectDescriptor { .name = "Flat" },
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

void createSpinningTriangle(EntityCommands& commands, MaterialHandle material, MeshHandle mesh, vec3 position, float spinRate)
{
    auto triangleEntity = commands.createEntity();

    Transform transform{};
    transform.setPosition(position);

    commands.addComponent(triangleEntity, transform);
    commands.addComponent(triangleEntity, MaterialRef{ .handle = material });
    commands.addComponent(triangleEntity, MeshRef{ .handle = mesh });
    commands.addComponent(triangleEntity, Spin{ .rate = spinRate });
}

void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto objectPool = services.get<ObjectPool>();
    auto sceneManager = services.get<SceneManager>();
    auto sceneView = services.get<SceneView>();

    sceneManager->createScene({.partition = ScenePartitionType::Null });
    sceneView->setMainCamera(objectPool->createCamera({}));

    auto triangleMaterial = createTriangleMaterial(*objectPool);
    auto triangleMesh = createTriangleMesh(*objectPool);

    for (float x = -5.0f; x <= 5.0f; x += 0.5f)
    {
        for (float y = -5.0f; y <= 5.0f; y += 0.5f)
        {
            createSpinningTriangle(commands, triangleMaterial, triangleMesh, { x,  y, 5.0f }, 1.0f);
        }
    }
}

void configureSystems(SystemCollection& systems)
{
    systems.addSystem<SpinSystem>(SystemGroup::Update);
}

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Triangle Sample" } },
        nullptr, 
        configureSystems,
        bootstrap,
        nullptr);

    engine.start();

    return 0;
}
