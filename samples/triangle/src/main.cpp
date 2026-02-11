#include "litl-engine/engine.hpp"

class TriangleObj : public LITL::Engine::SceneObject
{
public:

    void onSetup() override {}
    void onUpdate() override {}
    void onRender(LITL::Renderer::CommandBuffer* pCommandBuffer) override {}

protected:

private:
};

int main()
{
    LITL::Engine::Engine engine({});

    if (!engine.openWindow("LITL - Triangle Sample", 1024, 1024))
    {
        return 1;
    }

    engine.addSceneObject<TriangleObj>();

    while (engine.shouldRun())
    {
        engine.run();
    }

    return 0;
}
