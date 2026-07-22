#include <fstream>

#include "litl-engine/startup.hpp"
#include "litl-engine/ecs/common.hpp"
#include "litl-core/containers/alignedByteBuffer.hpp"

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

void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto objectPool = services.get<ObjectPool>();
    auto sceneManager = services.get<SceneManager>();
    auto sceneView = services.get<SceneView>();

    sceneManager->createScene({});
    sceneView->setMainCamera(objectPool->createCamera({}));

    auto triangleEntity = commands.createEntity();
    auto triangleMaterial = createTriangleMaterial(*objectPool);
    auto triangleMesh = createTriangleMesh(*objectPool);

    commands.addComponent(triangleEntity, Transform{});
    commands.addComponent(triangleEntity, MaterialRef{ .handle = triangleMaterial });
    commands.addComponent(triangleEntity, MeshRef{ .handle = triangleMesh });
}

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings { .applicationName = "LITL - Triangle Sample" } },
        nullptr, 
        nullptr,
        bootstrap,
        nullptr);

    engine.start();

    return 0;
}
