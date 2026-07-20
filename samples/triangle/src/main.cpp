#include "litl-engine/startup.hpp"

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

void bootstrap(ServiceProvider& services, EntityCommands& commands)
{
    auto objectPool = services.get<ObjectPool>();
    auto sceneManager = services.get<SceneManager>();
    auto sceneView = services.get<SceneView>();

    sceneManager->createScene({});
    sceneView->setMainCamera(objectPool->createCamera({}));

    auto meshHandle = createTriangleMesh(*objectPool);
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
