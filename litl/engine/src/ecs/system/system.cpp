#include "litl-engine/ecs/system/system.hpp"

namespace LITL::Engine::ECS
{
    void System::run(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
    {
        assert(m_runFunc != nullptr);
        m_runFunc(world, dt, chunk, layout);
    }
}